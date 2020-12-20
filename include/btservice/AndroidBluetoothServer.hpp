#pragma once

#include <stdint.h>
#include <atomic>
#include <memory>
#include <sstream>
#include <QBluetoothServer>
#include <QBluetoothLocalDevice>
#include <QDataStream>
#include <btservice_proto/NetworkInfo.pb.h>
#include <btservice_proto/SocketInfoRequest.pb.h>
#include <btservice_proto/SocketInfoResponse.pb.h>
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
    AndroidBluetoothServer(openauto::configuration::IConfiguration::Pointer config);
    bool start(const QBluetoothAddress& address, uint16_t portNumber) override;

private slots:
    void onClientConnected();
    void readSocket();

private:
    std::unique_ptr<QBluetoothServer> rfcommServer_;
    QBluetoothSocket* socket_;
    openauto::configuration::IConfiguration::Pointer config_;

    void handleUnknownMessage(int messageType, QByteArray data);
    void handleSocketInfoRequest(QByteArray data);
    void handleSocketInfoRequestResponse(QByteArray data);
    void writeSocketInfoRequest();
    void writeSocketInfoResponse();
    void writeNetworkInfoMessage();
    bool writeProtoMessage(uint16_t messageType, google::protobuf::Message& message);

};

}
}
