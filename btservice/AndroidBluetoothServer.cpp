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
        writeSocketInfoRequest();
    }
    else
    {
        OPENAUTO_LOG(error) << "[AndroidBluetoothServer] received null socket during client connection.";
    }
}

bool AndroidBluetoothServer::writeProtoMessage(uint16_t messageType, google::protobuf::Message& message)
{
    QByteArray byteArray(message.SerializeAsString().c_str(), message.ByteSize());
    uint16_t messageLength = message.ByteSize();
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

void AndroidBluetoothServer::writeSocketInfoRequest()
{
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Sending SocketInfoRequest.";

    QString ipAddr;
    foreach(QHostAddress addr, QNetworkInterface::allAddresses())
    {
        if(!addr.isLoopback() && (addr.protocol() == QAbstractSocket::IPv4Protocol))
        {
            ipAddr = addr.toString();
        }
    }

    btservice::proto::SocketInfoRequest socketInfoRequest;
    socketInfoRequest.set_ip_address(ipAddr.toStdString());
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] ipAddress: "<< ipAddr.toStdString();

    socketInfoRequest.set_port(5000);
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] port: "<< 5000;

    if(this->writeProtoMessage(1, socketInfoRequest))
    {
        OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Sent SocketInfoRequest.";
    }
    else
    {
        OPENAUTO_LOG(error) << "[AndroidBluetoothServer] Error sending SocketInfoRequest.";
    }
}
void AndroidBluetoothServer::writeSocketInfoResponse()
{
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Sending SocketInfoResponse.";
    QString ipAddr;
    foreach(QHostAddress addr, QNetworkInterface::allAddresses())
    {
        if(!addr.isLoopback() && (addr.protocol() == QAbstractSocket::IPv4Protocol))
        {
            ipAddr = addr.toString();
        }
    }

    btservice::proto::SocketInfoResponse socketInfoResponse;
    socketInfoResponse.set_ip_address(ipAddr.toStdString());
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] ipAddress: "<< ipAddr.toStdString();

    socketInfoResponse.set_port(5000);
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] port: "<< 5000;

    socketInfoResponse.set_status(btservice::proto::Status::STATUS_SUCCESS);
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] status: "<< btservice::proto::Status::STATUS_SUCCESS;


    if(this->writeProtoMessage(7, socketInfoResponse))
    {
        OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Sent SocketInfoResponse.";
    }
    else
    {
        OPENAUTO_LOG(error) << "[AndroidBluetoothServer] Error sending SocketInfoResponse.";
    }
}

void AndroidBluetoothServer::handleSocketInfoRequestResponse(QByteArray data)
{
    btservice::proto::SocketInfoResponse socketInfoResponse;
    socketInfoResponse.ParseFromArray(data, data.size());
    OPENAUTO_LOG(info) <<"[AndroidBluetoothServer] Received SocketInfoRequestResponse, status: "<<socketInfoResponse.status();
}


void AndroidBluetoothServer::handleSocketInfoRequest(QByteArray data)
{
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Reading SocketInfoRequest.";

    btservice::proto::SocketInfoRequest socketInfoRequest;
    
    writeSocketInfoResponse();
}

void AndroidBluetoothServer::writeNetworkInfoMessage()
{
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Sending NetworkInfoMessage.";

    btservice::proto::NetworkInfo networkMessage;
    networkMessage.set_ssid(config_->getWifiSSID());
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] SSID: "<<config_->getWifiSSID();

    networkMessage.set_psk(config_->getWifiPassword());
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] PSKEY: "<<config_->getWifiPassword();

    if(config_->getWifiMAC().empty())
    {
        networkMessage.set_mac_addr(QNetworkInterface::interfaceFromName("wlan0").hardwareAddress().toStdString());
        OPENAUTO_LOG(info) << "[AndroidBluetoothServer] MAC: "<<QNetworkInterface::interfaceFromName("wlan0").hardwareAddress().toStdString();
    }
    else
    {
        networkMessage.set_mac_addr(config_->getWifiMAC());
        OPENAUTO_LOG(info) << "[AndroidBluetoothServer] MAC: "<< config_->getWifiMAC();
    }

    networkMessage.set_security_mode(btservice::proto::SecurityMode::WPA2_PERSONAL);
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Security: "<< btservice::proto::SecurityMode::WPA2_PERSONAL;

    networkMessage.set_ap_type(btservice::proto::AccessPointType::STATIC);
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] AP Type: "<< btservice::proto::AccessPointType::STATIC;


    if(this->writeProtoMessage(3, networkMessage))
    {
        OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Sent NetworkInfoMessage";
    }
    else
    {
        OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Error sending NetworkInfoMessage.";
    }
}

void AndroidBluetoothServer::handleUnknownMessage(int messageType, QByteArray data)
{
        OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Received unknown MessageType of "<<messageType;
        OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Unknown Message Data: "<<data.toHex(' ').toStdString()  ;
}

void AndroidBluetoothServer::readSocket()
{
    if(!socket_)
    {
        return;
    }

    auto data = socket_->readAll();
    if(data.length() == 0)
    {
        return;
    }

    uint16_t messageType = (data[2] << 8) | data[3];
    switch(messageType)
    {
        case 1:
            handleSocketInfoRequest(data);
            break;
        case 2:
            writeNetworkInfoMessage();
            break;
        case 7:
            data.remove(0, 4);
            handleSocketInfoRequestResponse(data);
            break;
        default:
            data.remove(0, 4);
            handleUnknownMessage(messageType, data);
            break;
    }
}

}
}
