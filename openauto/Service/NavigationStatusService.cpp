#include "OpenautoLog.hpp"
#include "openauto/Service/NavigationStatusService.hpp"

namespace openauto
{
namespace service
{

NavigationStatusService::NavigationStatusService(boost::asio::io_service& ioService, aasdk::messenger::IMessenger::Pointer messenger)
    : strand_(ioService)
    , channel_(std::make_shared<aasdk::channel::navigation::NavigationStatusServiceChannel>(strand_, std::move(messenger)))
{

}

void NavigationStatusService::start()
{
    strand_.dispatch([this, self = this->shared_from_this()]() {
        OPENAUTO_LOG(info) << "[NavigationStatusService] start.";
        channel_->receive(this->shared_from_this());
    });
}

void NavigationStatusService::stop()
{
    strand_.dispatch([this, self = this->shared_from_this()]() {
        OPENAUTO_LOG(info) << "[NavigationStatusService] stop.";
    });
}

void NavigationStatusService::fillFeatures(aasdk::proto::messages::ServiceDiscoveryResponse& response)
{
    OPENAUTO_LOG(info) << "[NavigationStatusService] fill features";

    auto* channelDescriptor = response.add_channels();
    channelDescriptor->set_channel_id(static_cast<uint32_t>(channel_->getId()));
    auto navStatusChannel = channelDescriptor->mutable_navigation_channel();
    navStatusChannel->set_minimum_interval_ms(10);
    navStatusChannel->set_type(1);
    auto* imageOptions = new aasdk::proto::data::NavigationImageOptions();
    imageOptions->set_colour_depth_bits(16);
    imageOptions->set_height(256);
    imageOptions->set_width(256);
    imageOptions->set_dunno(255);
    navStatusChannel->set_allocated_image_options(imageOptions);    
}

void NavigationStatusService::onChannelOpenRequest(const aasdk::proto::messages::ChannelOpenRequest& request)
{
    OPENAUTO_LOG(info) << "[NavigationStatusService] open request, priority: " << request.priority();
    const aasdk::proto::enums::Status::Enum status = aasdk::proto::enums::Status::OK;
    OPENAUTO_LOG(info) << "[NavigationStatusService] open status: " << status;

    aasdk::proto::messages::ChannelOpenResponse response;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&NavigationStatusService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
}


void NavigationStatusService::onChannelError(const aasdk::error::Error& e)
{
    OPENAUTO_LOG(error) << "[NavigationStatusService] channel error: " << e.what();
}

}
}
