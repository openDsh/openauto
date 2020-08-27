#pragma once

#include <stdint.h>
#include <atomic>
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

enum class ConnectionStatus
{
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

class AndroidBluetoothServer: public QObject, public IAndroidBluetoothServer
{
    Q_OBJECT

public:
    AndroidBluetoothServer(openauto::configuration::IConfiguration::Pointer config);
    bool start(const QBluetoothAddress& address, uint16_t portNumber) override;

private slots:
    void onClientConnected();
    void readSocket();

private:
    std::unique_ptr<QBluetoothServer> rfcommServer_;
    QBluetoothSocket* socket_;
    openauto::configuration::IConfiguration::Pointer config_;
    std::atomic<ConnectionStatus> handshakeState_;

    void writeSocketInfoMessage();
    void writeNetworkInfoMessage();
    void eventLoop();
    bool writeProtoMessage(uint16_t messageType, google::protobuf::Message& message);

};

}
}
