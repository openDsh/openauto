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
    connectToBluetooth(QBluetoothAddress(QString::fromStdString(config->getLastBluetoothPair())));
}

void btservice::connectToBluetooth(QBluetoothAddress addr)
{
    QString program = QString::fromStdString("sudo stdbuf -oL rfcomm connect hci0 ")+addr.toString()+QString::fromStdString(" 2");
    OPENAUTO_LOG(info)<<"[btservice] Attempting bt connection with `"<<program.toStdString();
    rfcomm = new QProcess();

//    connect(rfcomm, SIGNAL(started()), this, SLOT(rfcomm_started()));
 //   connect(rfcomm, SIGNAL(finished(int)), this, SLOT(rfcomm_finished(int)));
   // connect(this, SIGNAL(kill_hcitool()), hcitool, SLOT(kill()));
   // connect(this, SIGNAL(terminate_hcitool()), hcitool, SLOT(terminate()));

    rfcomm->start(program, QProcess::Unbuffered | QProcess::ReadWrite);
}


}
}
