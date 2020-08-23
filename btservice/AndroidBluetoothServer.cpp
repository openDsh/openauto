#include "OpenautoLog.hpp"
#include "btservice/AndroidBluetoothServer.hpp"
#include <QNetworkInterface>
#include <QThread>

namespace openauto
{
namespace btservice
{

AndroidBluetoothServer::AndroidBluetoothServer(openauto::configuration::IConfiguration::Pointer config_)
    : rfcommServer_(std::make_unique<QBluetoothServer>(QBluetoothServiceInfo::RfcommProtocol, this))
    , config(std::move(config_))
{
    handshakeState = IDLE;
    connect(rfcommServer_.get(), &QBluetoothServer::newConnection, this, &AndroidBluetoothServer::onClientConnected);
    QThread *thread = QThread::create([&]{ this->stateMachine(); });
    thread->start();
}

void AndroidBluetoothServer::stateMachine()
{
    while(true){
        switch(handshakeState){
            case IDLE:
                break;
            
            case DEVICE_CONNECTED:
                handshakeState = SENDING_SOCKETINFO_MESSAGE;
                break;

            case SENDING_SOCKETINFO_MESSAGE:
                writeSocketInfoMessage();
                break;

            case SENT_SOCKETINFO_MESSAGE:
                break;

            case PHONE_RESP_SOCKETINFO:
                handshakeState = SENDING_NETWORKINFO_MESSAGE;
                break;
            
            case SENDING_NETWORKINFO_MESSAGE:
                writeNetworkInfoMessage();
                break;
            
            case SENT_NETWORKINFO_MESSAGE:
                break;

            case PHONE_RESP_NETWORKINFO:
                break;
            
        }
    }
}

bool AndroidBluetoothServer::start(const QBluetoothAddress& address, uint16_t portNumber)
{
    OPENAUTO_LOG(info)<<"listening";
    return rfcommServer_->listen(address, portNumber);
}

void AndroidBluetoothServer::onClientConnected()
{
    socket = rfcommServer_->nextPendingConnection();

    if(socket != nullptr)
    {
        OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Device Connected: " << socket->peerName().toStdString();
        connect(socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
        handshakeState = DEVICE_CONNECTED;
    }
    else
    {
        OPENAUTO_LOG(error) << "[AndroidBluetoothServer] received null socket during client connection.";
    }
}

bool AndroidBluetoothServer::writeProtoMessage(uint16_t messageType, google::protobuf::Message &message){
    QByteArray byteArray(message.SerializeAsString().c_str(), message.ByteSize());
    uint16_t message_length = message.ByteSize();
    char byte1 =  messageType & 0x000000ff;
    char byte2 = (messageType & 0x0000ff00) >> 8;
    byteArray.prepend(byte1);
    byteArray.prepend(byte2);
    byte1 =  message_length & 0x000000ff;
    byte2 = (message_length & 0x0000ff00) >> 8;
    byteArray.prepend(byte1);
    byteArray.prepend(byte2);
    int sentBytes = socket->write(byteArray);
    if(sentBytes != byteArray.length()) return false;
    return true;
}

void AndroidBluetoothServer::writeSocketInfoMessage(){
        OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Sending Socket Info";

        btservice::proto::SocketInfo socketInfo;
        
        QString ipAddr;
        QList<QHostAddress> list = QNetworkInterface::allAddresses();

        for(int nIter=0; nIter<list.count(); nIter++)
        {
            if(!list[nIter].isLoopback())
                if (list[nIter].protocol() == QAbstractSocket::IPv4Protocol )
                    ipAddr = list[nIter].toString();
        }
        socketInfo.set_address(ipAddr.toStdString());
        socketInfo.set_port(5000);
        socketInfo.set_unknown_1(0);

        if(writeProtoMessage(7, socketInfo)){
            OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Sent Socket Info";
            handshakeState = SENT_SOCKETINFO_MESSAGE;
        }
        else{
            OPENAUTO_LOG(error) << "[AndroidBluetoothServer] Error Sending Socket Info";
            handshakeState = ERROR;
        }
}

void AndroidBluetoothServer::writeNetworkInfoMessage(){
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Sending Network Packet";

    btservice::proto::NetworkInfo networkMessage;
    networkMessage.set_ssid(config->getWifiSSID());
    networkMessage.set_psk(config->getWifiPassword());

    foreach(QNetworkInterface netInterface, QNetworkInterface::allInterfaces())
    {
        // Return only the first non-loopback MAC Address
        if (!(netInterface.flags() & QNetworkInterface::IsLoopBack))
            networkMessage.set_mac_addr(netInterface.hardwareAddress().toStdString());
    }
    networkMessage.set_security_mode(8);
    networkMessage.set_unknown_2(0);


    if(writeProtoMessage(3, networkMessage)){
        OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Sent Network Packet";
        handshakeState = SENT_NETWORKINFO_MESSAGE;
    }
    else{
        OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Error sending Network Packet";
        handshakeState = ERROR;
    }
}

void AndroidBluetoothServer::readSocket(){
    OPENAUTO_LOG(info) << "[AndroidBluetoothServer] DATA: ";
    if (!socket)
          return;
    QByteArray data = socket->read(1024);
    if(data.length()==0) return;
    uint16_t messageType = data[2]<<8 | data[3];
    btservice::proto::PhoneResponse resp;
    switch(messageType){
        case 2:
            break;
        case 6:
            resp.ParseFromString(data.toStdString().c_str());
            break;
        


    }
    switch(handshakeState){
        case IDLE:
            break;
        
        case DEVICE_CONNECTED:
            break;

        case SENDING_SOCKETINFO_MESSAGE:
            break;

        case SENT_SOCKETINFO_MESSAGE:
            if(messageType == 2){
                OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Phone acknowledged Socket Info";
                handshakeState = PHONE_RESP_SOCKETINFO;
            }else{
                OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Got unexpected message";
                handshakeState = ERROR;
            }
            break;

        case PHONE_RESP_SOCKETINFO:
            break;
        
        case SENDING_NETWORKINFO_MESSAGE:
            break;
        
        case SENT_NETWORKINFO_MESSAGE:

            if(messageType == 6){
                OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Phone acknowledged Network Info with status code: "<<resp.status_code();
                handshakeState = PHONE_RESP_NETWORKINFO;
            }else{
                OPENAUTO_LOG(info) << "[AndroidBluetoothServer] Got unexpected message";
                handshakeState = ERROR;
            }
            break;

        case PHONE_RESP_NETWORKINFO:
            break;
        
    }
}
    
}

}

