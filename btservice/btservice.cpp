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
    if(config->getAutoconnectBluetooth())
        connectToBluetooth(QBluetoothAddress(QString::fromStdString(config->getLastBluetoothPair())), address);
}

void btservice::connectToBluetooth(QBluetoothAddress addr, QBluetoothAddress controller)
{
    // Update 07-12-22, with bluez update and krnbt, raspberry pi is behaving as expected. For this reason
    // the RPI specific code has been commented out. The commented out code (and comments below this one) 
    // exist as legacy now - in case there's a scenario where someone cannot update to krnbt or newer bluez.


    // The raspberry pi has a really tough time using bluetoothctl (or really anything) to connect to an Android phone
    // even though phone connecting to the pi is fine.
    // I found a workaround where you can make the pi attempt an rfcomm connection to the phone, and it connects immediately
    // This might require setting u+s on rfcomm though
    // Other computers with more sane bluetooth shouldn't have an issue using bluetoothctl

    // Update 01-10-21, latest firmware/package updates seem to have made bluetoothctl more stable
    // and it won't drop a connection anymore. Only issue, is that the connection will fail
    // if the desired target hasn't been "seen" yet
    // we can use hcitool to force the pi to recognize that we're trying to connect to a valid device.
    // this causes the target device to be "seen"
    // bluetoothctl can then connect as normal.
    // Why don't we just use rfcomm (as we had previously on pis)? Because an rfcomm initiated connection doesn't connect HFP, which breaks the use of
    // pi mic/speakers for android auto phone calls. bluetoothctl will connect all profiles.
    
#ifdef RPI
//    QString program = QString::fromStdString("sudo hcitool cc ")+addr.toString();
//    btConnectProcess = new QProcess();
//    OPENAUTO_LOG(info)<<"[btservice] Attempting to connect to last bluetooth device, "<<addr.toString().toStdString()<<" using hcitool/bluetoothctl hybrid";
//    btConnectProcess->start(program, QProcess::Unbuffered | QProcess::ReadWrite);
//    btConnectProcess->waitForFinished();
#endif
    btConnectProcess = new QProcess();
    btConnectProcess->setProcessChannelMode(QProcess::SeparateChannels);
    OPENAUTO_LOG(info)<<"[btservice] Attempting to connect to last bluetooth device, "<<addr.toString().toStdString()<<" with bluetoothctl";
    btConnectProcess->start("bluetoothctl");
    btConnectProcess->waitForStarted();
    btConnectProcess->write(QString("select %1\n").arg(controller.toString()).toUtf8());
    btConnectProcess->write(QString("connect %1\n").arg(addr.toString()).toUtf8());
    btConnectProcess->closeWriteChannel();
    btConnectProcess->waitForFinished();
}
}
}
