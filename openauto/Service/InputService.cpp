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

#include "aasdk_proto/InputEventIndicationMessage.pb.h"
#include "OpenautoLog.hpp"
#include "openauto/Service/InputService.hpp"

namespace openauto
{
namespace service
{

InputService::InputService(boost::asio::io_service& ioService, aasdk::messenger::IMessenger::Pointer messenger, projection::IInputDevice::Pointer inputDevice)
    : strand_(ioService)
    , channel_(std::make_shared<aasdk::channel::input::InputServiceChannel>(strand_, std::move(messenger)))
    , inputDevice_(std::move(inputDevice))
{
    OPENAUTO_LOG(info) << "[InputService] Created";
}

void InputService::start()
{
    strand_.dispatch([this, self = this->shared_from_this()]() {
        OPENAUTO_LOG(info) << "[InputService] start.";
        channel_->receive(this->shared_from_this());
    });
    serviceActive = true;
}

void InputService::stop()
{
    strand_.dispatch([this, self = this->shared_from_this()]() {
        OPENAUTO_LOG(info) << "[InputService] stop.";
        inputDevice_->stop();
    });
    serviceActive = false;
}

void InputService::fillFeatures(aasdk::proto::messages::ServiceDiscoveryResponse& response)
{
    OPENAUTO_LOG(info) << "[InputService] fill features.";

    auto* channelDescriptor = response.add_channels();
    channelDescriptor->set_channel_id(static_cast<uint32_t>(channel_->getId()));

    auto* inputChannel = channelDescriptor->mutable_input_channel();

    const auto& supportedButtonCodes = inputDevice_->getSupportedButtonCodes();

    for(const auto& buttonCode : supportedButtonCodes)
    {
        inputChannel->add_supported_keycodes(buttonCode);
    }

    if(inputDevice_->hasTouchscreen())
    {
        const auto& touchscreenSurface = inputDevice_->getTouchscreenGeometry();
        auto touchscreenConfig = inputChannel->mutable_touch_screen_config();

        touchscreenConfig->set_width(touchscreenSurface.width());
        touchscreenConfig->set_height(touchscreenSurface.height());
    }
}

void InputService::onChannelOpenRequest(const aasdk::proto::messages::ChannelOpenRequest& request)
{
    OPENAUTO_LOG(info) << "[InputService] open request, priority: " << request.priority();
    const aasdk::proto::enums::Status::Enum status = aasdk::proto::enums::Status::OK;
    OPENAUTO_LOG(info) << "[InputService] open status: " << status;

    aasdk::proto::messages::ChannelOpenResponse response;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&InputService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
}

void InputService::onBindingRequest(const aasdk::proto::messages::BindingRequest& request)
{
    OPENAUTO_LOG(info) << "[InputService] binding request, scan codes count: " << request.scan_codes_size();

    aasdk::proto::enums::Status::Enum status = aasdk::proto::enums::Status::OK;
    const auto& supportedButtonCodes = inputDevice_->getSupportedButtonCodes();

    for(int i = 0; i < request.scan_codes_size(); ++i)
    {
        if(std::find(supportedButtonCodes.begin(), supportedButtonCodes.end(), request.scan_codes(i)) == supportedButtonCodes.end())
        {
            OPENAUTO_LOG(error) << "[InputService] binding request, scan code: " << request.scan_codes(i)
                                << " is not supported.";

            status = aasdk::proto::enums::Status::FAIL;
            break;
        }
    }

    aasdk::proto::messages::BindingResponse response;
    response.set_status(status);

    if(status == aasdk::proto::enums::Status::OK)
    {
        inputDevice_->start(*this);
    }

    OPENAUTO_LOG(info) << "[InputService] binding request, status: " << status;

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&InputService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendBindingResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
}

void InputService::onChannelError(const aasdk::error::Error& e)
{
    OPENAUTO_LOG(error) << "[InputService] channel error: " << e.what();
}

void InputService::onButtonEvent(const projection::ButtonEvent& event)
{
    if(!serviceActive) return;
    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());

    strand_.dispatch([this, self = this->shared_from_this(), event = std::move(event), timestamp = std::move(timestamp)]() {
        aasdk::proto::messages::InputEventIndication inputEventIndication;
        inputEventIndication.set_timestamp(timestamp.count());

        if(event.code == aasdk::proto::enums::ButtonCode::SCROLL_WHEEL)
        {
            auto relativeEvent = inputEventIndication.mutable_relative_input_event()->add_relative_input_events();
            relativeEvent->set_delta(event.wheelDirection == projection::WheelDirection::LEFT ? -1 : 1);
            relativeEvent->set_scan_code(event.code);
        }
        else
        {
            auto buttonEvent = inputEventIndication.mutable_button_event()->add_button_events();
            buttonEvent->set_meta(0);
            buttonEvent->set_is_pressed(event.type == projection::ButtonEventType::PRESS);
            buttonEvent->set_long_press(false);
            buttonEvent->set_scan_code(event.code);
        }

        auto promise = aasdk::channel::SendPromise::defer(strand_);
        promise->then([]() {}, std::bind(&InputService::onChannelError, this->shared_from_this(), std::placeholders::_1));
        channel_->sendInputEventIndication(inputEventIndication, std::move(promise));
    });
}

void InputService::sendButtonPress(aasdk::proto::enums::ButtonCode::Enum buttonCode)
{    
    OPENAUTO_LOG(error) << "[InputService] injecting button press";

    onButtonEvent({projection::ButtonEventType::PRESS, projection::WheelDirection::NONE, buttonCode});
    onButtonEvent({projection::ButtonEventType::RELEASE, projection::WheelDirection::NONE, buttonCode});
}


void InputService::onTouchEvent(const projection::TouchEvent& event)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());

    strand_.dispatch([this, self = this->shared_from_this(), event = std::move(event), timestamp = std::move(timestamp)]() {
        aasdk::proto::messages::InputEventIndication inputEventIndication;
        inputEventIndication.set_timestamp(timestamp.count());

        auto touchEvent = inputEventIndication.mutable_touch_event();
        touchEvent->set_touch_action(event.type);
        auto touchLocation = touchEvent->add_touch_location();
        touchLocation->set_x(event.x);
        touchLocation->set_y(event.y);
        touchLocation->set_pointer_id(0);

        auto promise = aasdk::channel::SendPromise::defer(strand_);
        promise->then([]() {}, std::bind(&InputService::onChannelError, this->shared_from_this(), std::placeholders::_1));
        channel_->sendInputEventIndication(inputEventIndication, std::move(promise));
    });
}

}
}
