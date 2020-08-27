#include "btservice/btservice.hpp"

namespace openauto
{
namespace btservice
{

btservice::btservice(openauto::configuration::IConfiguration::Pointer config)
    : androidBluetoothService_(cServicePortNumber)
    , androidBluetoothServer_(config)
{
    QBluetoothAddress address;
    auto adapters = QBluetoothLocalDevice::allDevices();
    if(adapters.size() > 0)
    {
        address = adapters.at(0).address();
    }
    else
    {
        OPENAUTO_LOG(error) << "[btservice] No adapter found.";
    }

    if(!androidBluetoothServer_.start(address, cServicePortNumber))
    {
        OPENAUTO_LOG(error) << "[btservice] Server start failed.";
        return;
    }

    OPENAUTO_LOG(info) << "[btservice] Listening for connections, address: " << address.toString().toStdString()
                       << ", port: " << cServicePortNumber;

    if(!androidBluetoothService_.registerService(address))
    {
        OPENAUTO_LOG(error) << "[btservice] Service registration failed.";
    }
    else
    {
        OPENAUTO_LOG(info) << "[btservice] Service registered, port: " << cServicePortNumber;
    }
}

}
}
