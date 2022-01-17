#include "OpenautoLog.hpp"
#include "openauto/Service/NavigationStatusService.hpp"
#include "aasdk_proto/ManeuverTypeEnum.pb.h"
#include "aasdk_proto/ManeuverDirectionEnum.pb.h"
#include "aasdk_proto/DistanceUnitEnum.pb.h"
#include "openauto/Service/IAndroidAutoInterface.hpp"

namespace openauto
{
namespace service
{

NavigationStatusService::NavigationStatusService(boost::asio::io_service& ioService, aasdk::messenger::IMessenger::Pointer messenger, IAndroidAutoInterface* aa_interface)
    : strand_(ioService)
    , channel_(std::make_shared<aasdk::channel::navigation::NavigationStatusServiceChannel>(strand_, std::move(messenger)))
{
    this->aa_interface_ = aa_interface;
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
    navStatusChannel->set_minimum_interval_ms(1000);
    navStatusChannel->set_type(aasdk::proto::enums::NavigationTurnType::IMAGE);
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

void NavigationStatusService::onStatusUpdate(const aasdk::proto::messages::NavigationStatus& navStatus)
{
    OPENAUTO_LOG(info) << "[NavigationStatusService] Navigation Status Update"
                       << ", Status: " <<  aasdk::proto::messages::NavigationStatus_Enum_Name(navStatus.status());
    if(aa_interface_ != NULL)
    {
        aa_interface_->navigationStatusUpdate(navStatus);
    }
    channel_->receive(this->shared_from_this());
}

void NavigationStatusService::onTurnEvent(const aasdk::proto::messages::NavigationTurnEvent& turnEvent)
{
    OPENAUTO_LOG(info) << "[NavigationStatusService] Turn Event"
                       << ", Street: " << turnEvent.street_name()
                       << ", Maneuver: " <<  aasdk::proto::enums::ManeuverDirection_Enum_Name(turnEvent.maneuverdirection()) << " " << aasdk::proto::enums::ManeuverType_Enum_Name(turnEvent.maneuvertype());
    if(aa_interface_ != NULL)
    {
        aa_interface_->navigationTurnEvent(turnEvent);
    }
    channel_->receive(this->shared_from_this());
}

void NavigationStatusService::onDistanceEvent(const aasdk::proto::messages::NavigationDistanceEvent& distanceEvent)
{
    OPENAUTO_LOG(info) << "[NavigationStatusService] Distance Event"
                       << ", Distance (meters): " << distanceEvent.meters()
                       << ", Time To Turn (seconds): " << distanceEvent.timetostepseconds()
                       << ", Distance: " << distanceEvent.distancetostepmillis()/1000.0
                       << " ("<<aasdk::proto::enums::DistanceUnit_Enum_Name(distanceEvent.distanceunit())<<")";
    if(aa_interface_ != NULL)
    {
        aa_interface_->navigationDistanceEvent(distanceEvent);
    }
    channel_->receive(this->shared_from_this());
}


void NavigationStatusService::setAndroidAutoInterface(IAndroidAutoInterface* aa_interface)
{
    this->aa_interface_ = aa_interface;
}


}
}
