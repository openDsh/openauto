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

#include "aasdk_proto/ButtonCodeEnum.pb.h"
#include "aasdk/Channel/Input/InputServiceChannel.hpp"
#include "IService.hpp"
#include "openauto/Projection/IInputDevice.hpp"
#include "openauto/Projection/IInputDeviceEventHandler.hpp"

namespace openauto
{
namespace service
{

class InputService:
        public aasdk::channel::input::IInputServiceChannelEventHandler,
        public IService,
        public projection::IInputDeviceEventHandler,
        public std::enable_shared_from_this<InputService>
{
public:
    InputService(boost::asio::io_service& ioService, aasdk::messenger::IMessenger::Pointer messenger, projection::IInputDevice::Pointer inputDevice);

    void sendPlayButtonEvent();
    void sendPauseButtonEvent();

    void start() override;
    void stop() override;
    void fillFeatures(aasdk::proto::messages::ServiceDiscoveryResponse& response) override;
    void onChannelOpenRequest(const aasdk::proto::messages::ChannelOpenRequest& request) override;
    void onBindingRequest(const aasdk::proto::messages::BindingRequest& request) override;
    void onChannelError(const aasdk::error::Error& e) override;
    void onButtonEvent(const projection::ButtonEvent& event) override;
    void onTouchEvent(const projection::TouchEvent& event) override;


private:
    using std::enable_shared_from_this<InputService>::shared_from_this;

    boost::asio::io_service::strand strand_;
    aasdk::channel::input::InputServiceChannel::Pointer channel_;
    projection::IInputDevice::Pointer inputDevice_;
    bool serviceActive = false;
};

}
}
