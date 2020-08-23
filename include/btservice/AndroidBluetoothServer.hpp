#pragma once

#include <stdint.h>
#include <memory>
#include <sstream>
#include <QBluetoothServer>
#include <QBluetoothLocalDevice>
#include <QDataStream>
#include <btservice_proto/NetworkInfo.pb.h>
#include <btservice_proto/PhoneResponse.pb.h>
#include <btservice_proto/SocketInfo.pb.h>
#include "openauto/Configuration/Configuration.hpp"
#include "IAndroidBluetoothServer.hpp"

namespace openauto
{
namespace btservice
{

class AndroidBluetoothServer: public QObject, public IAndroidBluetoothServer
{
    Q_OBJECT

public:
    AndroidBluetoothServer(openauto::configuration::IConfiguration::Pointer config_);

    bool start(const QBluetoothAddress& address, uint16_t portNumber) override;

private slots:
    void onClientConnected();
    void readSocket();

private:
    std::unique_ptr<QBluetoothServer> rfcommServer_;
    QBluetoothSocket* socket;

    void writeSocketInfoMessage();
    void writeNetworkInfoMessage();
    void stateMachine();
    bool writeProtoMessage(uint16_t messageType, google::protobuf::Message &message);

    enum CONNECTION_STATUS {
        IDLE,
        DEVICE_CONNECTED,
        SENDING_SOCKETINFO_MESSAGE,
        SENT_SOCKETINFO_MESSAGE,
        PHONE_RESP_SOCKETINFO,
        SENDING_NETWORKINFO_MESSAGE,
        SENT_NETWORKINFO_MESSAGE,
        PHONE_RESP_NETWORKINFO,
        ERROR
    };
 
    CONNECTION_STATUS handshakeState = IDLE;
protected:
    openauto::configuration::IConfiguration::Pointer config;
};

}
}
