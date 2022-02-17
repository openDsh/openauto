#include "OpenautoLog.hpp"
#include "openauto/Service/MediaStatusService.hpp"
#include "openauto/Service/IAndroidAutoInterface.hpp"

namespace openauto
{
namespace service
{

MediaStatusService::MediaStatusService(boost::asio::io_service& ioService, aasdk::messenger::IMessenger::Pointer messenger, IAndroidAutoInterface* aa_interface)
    : strand_(ioService)
    , channel_(std::make_shared<aasdk::channel::av::MediaStatusServiceChannel>(strand_, std::move(messenger)))
{
    aa_interface_ = aa_interface;
}

void MediaStatusService::start()
{
    strand_.dispatch([this, self = this->shared_from_this()]() {
        OPENAUTO_LOG(info) << "[MediaStatusService] start.";
        channel_->receive(this->shared_from_this());
    });
}

void MediaStatusService::stop()
{
    strand_.dispatch([this, self = this->shared_from_this()]() {
        OPENAUTO_LOG(info) << "[MediaStatusService] stop.";
    });
}

void MediaStatusService::fillFeatures(aasdk::proto::messages::ServiceDiscoveryResponse& response)
{
    OPENAUTO_LOG(info) << "[MediaStatusService] fill features";

    auto* channelDescriptor = response.add_channels();
    auto mediaStatusChannel = channelDescriptor->mutable_media_infochannel();
    channelDescriptor->set_channel_id(static_cast<uint32_t>(channel_->getId()));
}

void MediaStatusService::onChannelOpenRequest(const aasdk::proto::messages::ChannelOpenRequest& request)
{
    OPENAUTO_LOG(info) << "[MediaStatusService] open request, priority: " << request.priority();
    const aasdk::proto::enums::Status::Enum status = aasdk::proto::enums::Status::OK;
    OPENAUTO_LOG(info) << "[MediaStatusService] open status: " << status;

    aasdk::proto::messages::ChannelOpenResponse response;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&MediaStatusService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
}


void MediaStatusService::onChannelError(const aasdk::error::Error& e)
{
    OPENAUTO_LOG(error) << "[MediaStatusService] channel error: " << e.what();
}

void MediaStatusService::onMetadataUpdate(const aasdk::proto::messages::MediaInfoChannelMetadataData& metadata)
{
    OPENAUTO_LOG(info) << "[MediaStatusService] Metadata update"
                       << ", track: " <<  metadata.track_name()
                       << (metadata.has_artist_name()?", artist: ":"") << (metadata.has_artist_name()?metadata.artist_name():"")
                       << (metadata.has_album_name()?", album: ":"") << (metadata.has_album_name()?metadata.album_name():"")
                       << ", length: " << metadata.track_length();
    if(aa_interface_ != NULL)
    {
        aa_interface_->mediaMetadataUpdate(metadata);
    }
    channel_->receive(this->shared_from_this());
}

void MediaStatusService::onPlaybackUpdate(const aasdk::proto::messages::MediaInfoChannelPlaybackData& playback)
{
    OPENAUTO_LOG(info) << "[MediaStatusService] Playback update"
                       << ", source: " <<  playback.media_source()
                       << ", state: " << playback.playback_state()
                       << ", progress: " << playback.track_progress();
    if(aa_interface_ != NULL)
    {
        aa_interface_->mediaPlaybackUpdate(playback);
    }
    channel_->receive(this->shared_from_this());
}

void MediaStatusService::setAndroidAutoInterface(IAndroidAutoInterface* aa_interface)
{
    this->aa_interface_ = aa_interface;
}


}
}
