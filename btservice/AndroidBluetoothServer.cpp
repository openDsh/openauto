#include <QNetworkInterface>
#include <QThread>
#include "OpenautoLog.hpp"
#include "btservice/AndroidBluetoothServer.hpp"

namespace openauto
{
namespace btservice
{

AndroidBluetoothServer::AndroidBluetoothServer(openauto::configuration::IConfiguration::Pointer config)
    : rfcommServer_(std::make_unique<QBluetoothServer>(QBluetoothServiceInfo::RfcommProtocol, this))
    , socket_(nullptr)
    , config_(std::move(config))
    , handshakeState_(ConnectionStatus::IDLE)
{
    connect(rfcommServer_.get(), &QBluetoothServer::newConnection, this, &AndroidBluetoothServer::onClientConnected);

    QThread *thread = QThread::create([&]{ this->eventLoop(); });
    thread->start();
}

void AndroidBluetoothServer::eventLoop()
{
    while(true)
    {
        switch(handshakeState_)
        {
        case ConnectionStatus::IDLE:
        case ConnectionStatus::SENT_SOCKETINFO_MESSAGE:
        case ConnectionStatus::SENT_NETWORKINFO_MESSAGE:
        case ConnectionStatus::PHONE_RESP_NETWORKINFO:
        case ConnectionStatus::ERROR:
            break;

        case ConnectionStatus::DEVICE_CONNECTED:
            handshakeState_ = ConnectionStatus::SENDING_SOCKETINFO_MESSAGE;
            break;

        case ConnectionStatus::SENDING_SOCKETINFO_MESSAGE:
            writeSocketInfoMessage();
            break;

        case ConnectionStatus::PHONE_RESP_SOCKETINFO:
            handshakeState_ = ConnectionStatus::SENDING_NETWORKINFO_MESSAGE;
            break;

        case ConnectionStatus::SENDING_NETWORKINFO_MESSAGE:
            writeNetworkInfoMessage();
            break;
        }
    }
}

bool AndroidBluetoothServer::start(const QBluetoothAddress& address, uint16_t portNumber)
{
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] listening.";
    return rfcommServer_->listen(address, portNumber);
}

void AndroidBluetoothServer::onClientConnected()
{
    socket_ = rfcommServer_->nextPendingConnection();
    if(socket_ != nullptr)
    {
        OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Device Connected: " << socket_->peerName().toStdString();
        connect(socket_, SIGNAL(readyRead()), this, SLOT(readSocket()));
        handshakeState_ = ConnectionStatus::DEVICE_CONNECTED;
    }
    else
    {
        OPENAUTO_LOG(error) << "[AndroidBluetoothServer] received null socket during client connection.";
    }
}

bool AndroidBluetoothServer::writeProtoMessage(uint16_t messageType, google::protobuf::Message& message)
{
    QByteArray byteArray(message.SerializeAsString().c_str(), message.ByteSizeLong());
    uint16_t messageLength = message.ByteSizeLong();
    byteArray.prepend(messageType & 0x000000ff);
    byteArray.prepend((messageType & 0x0000ff00) >> 8);
    byteArray.prepend(messageLength & 0x000000ff);
    byteArray.prepend((messageLength & 0x0000ff00) >> 8);

    if(socket_->write(byteArray) != byteArray.length())
    {
        return false;
    }

    return true;
}

void AndroidBluetoothServer::writeSocketInfoMessage()
{
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Sending socket info.";

    QString ipAddr;
    foreach(QHostAddress addr, QNetworkInterface::allAddresses())
    {
        if(addr.isLoopback() && addr.protocol() == QAbstractSocket::IPv4Protocol)
        {
            ipAddr = addr.toString();
        }
    }

    btservice::proto::SocketInfo socketInfo;
    socketInfo.set_address(ipAddr.toStdString());
    socketInfo.set_port(5000);
    socketInfo.set_unknown_1(0);

    if(writeProtoMessage(7, socketInfo))
    {
        OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Sent socket info.";
        handshakeState_ = ConnectionStatus::SENT_SOCKETINFO_MESSAGE;
    }
    else
    {
        OPENAUTO_LOG(error) << "[AndroidBluetoothServer] Error sending socket Info.";
        handshakeState_ = ConnectionStatus::ERROR;
    }
}

void AndroidBluetoothServer::writeNetworkInfoMessage()
{
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Sending network packet.";

    btservice::proto::NetworkInfo networkMessage;
    networkMessage.set_ssid(config_->getWifiSSID());
    networkMessage.set_psk(config_->getWifiPassword());
    foreach(QNetworkInterface netInterface, QNetworkInterface::allInterfaces())
    {
        // Return only the first non-loopback MAC Address
        if (!(netInterface.flags() & QNetworkInterface::IsLoopBack))
        {
            networkMessage.set_mac_addr(netInterface.hardwareAddress().toStdString());
        }
    }
    networkMessage.set_security_mode(8);
    networkMessage.set_unknown_2(0);

    if(writeProtoMessage(3, networkMessage))
    {
        OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Sent network packet.";
        handshakeState_ = ConnectionStatus::SENT_NETWORKINFO_MESSAGE;
    }
    else
    {
        OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Error sending network packet.";
        handshakeState_ = ConnectionStatus::ERROR;
    }
}

void AndroidBluetoothServer::readSocket()
{
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] DATA: ";
    if (!socket_)
    {
        return;
    }

    QByteArray data = socket_->read(1024);
    if(data.length() == 0)
    {
        return;
    }

    uint16_t messageType = (data[2] << 8) | data[3];
    btservice::proto::PhoneResponse resp;
    switch(messageType)
    {
    case 2:
        break;

    case 6:
        resp.ParseFromString(data.toStdString().c_str());
        break;
    }

    switch(handshakeState_)
    {
    case ConnectionStatus::IDLE:
    case ConnectionStatus::DEVICE_CONNECTED:
    case ConnectionStatus::SENDING_SOCKETINFO_MESSAGE:
    case ConnectionStatus::PHONE_RESP_SOCKETINFO:
    case ConnectionStatus::SENDING_NETWORKINFO_MESSAGE:
    case ConnectionStatus::PHONE_RESP_NETWORKINFO:
    case ConnectionStatus::ERROR:
        break;

    case ConnectionStatus::SENT_SOCKETINFO_MESSAGE:
        if(messageType == 2)
        {
            OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Phone acknowledged socket info.";
            handshakeState_ = ConnectionStatus::PHONE_RESP_SOCKETINFO;
        }
        else
        {
            OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Got unexpected message.";
            handshakeState_ = ConnectionStatus::ERROR;
        }
        break;

    case ConnectionStatus::SENT_NETWORKINFO_MESSAGE:
        if(messageType == 6)
        {
            OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Phone acknowledged network info with status code: " << resp.status_code();
            handshakeState_ = ConnectionStatus::PHONE_RESP_NETWORKINFO;
        }
        else
        {
            OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Got unexpected message";
            handshakeState_ = ConnectionStatus::ERROR;
        }
        break;
    }
}

}
}
