/*
*  This file is part of openauto project.
*  Copyright (C) 2018 f1x.studio (Michal Szwaj)
*
*  openauto is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 3 of the License, or
*  (at your option) any later version.

*  openauto is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with openauto. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "aasdk/USB/IUSBHub.hpp"
#include "aasdk/USB/IConnectedAccessoriesEnumerator.hpp"
#include "aasdk/USB/USBWrapper.hpp"
#include "aasdk/TCP/ITCPWrapper.hpp"
#include "aasdk/TCP/ITCPEndpoint.hpp"
#include "openauto/Service/IAndroidAutoEntityEventHandler.hpp"
#include "openauto/Service/IAndroidAutoEntityFactory.hpp"
#include "openauto/Service/IService.hpp"

namespace openauto
{

class App: public openauto::service::IAndroidAutoEntityEventHandler, public std::enable_shared_from_this<App>
{
public:
    typedef std::shared_ptr<App> Pointer;

    App(boost::asio::io_service& ioService, aasdk::usb::USBWrapper& usbWrapper, aasdk::tcp::ITCPWrapper& tcpWrapper, openauto::service::IAndroidAutoEntityFactory& androidAutoEntityFactory,
        aasdk::usb::IUSBHub::Pointer usbHub, aasdk::usb::IConnectedAccessoriesEnumerator::Pointer connectedAccessoriesEnumerator);

    void waitForDevice(bool enumerate = false);
    void start(aasdk::tcp::ITCPEndpoint::SocketPointer socket);
    void stop();
    void onAndroidAutoQuit() override;

private:
    std::function<void(bool)> androidAutoStatusCallback_;
    using std::enable_shared_from_this<App>::shared_from_this;
    void enumerateDevices();
    void waitForUSBDevice();
    void waitForWirelessDevice();
    void aoapDeviceHandler(aasdk::usb::DeviceHandle deviceHandle);
    void onUSBHubError(const aasdk::error::Error& error);

    boost::asio::io_service& ioService_;
    aasdk::usb::USBWrapper& usbWrapper_;
    aasdk::tcp::ITCPWrapper& tcpWrapper_;
    boost::asio::io_service::strand strand_;
    openauto::service::IAndroidAutoEntityFactory& androidAutoEntityFactory_;
    aasdk::usb::IUSBHub::Pointer usbHub_;
    boost::asio::ip::tcp::acceptor acceptor_;
    aasdk::usb::IConnectedAccessoriesEnumerator::Pointer connectedAccessoriesEnumerator_;
    openauto::service::IAndroidAutoEntity::Pointer androidAutoEntity_;
    bool isStopped_;
};

}
