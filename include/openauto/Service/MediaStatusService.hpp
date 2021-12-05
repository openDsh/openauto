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

#include "aasdk/Channel/AV/MediaStatusServiceChannel.hpp"
#include "IService.hpp"

namespace openauto
{
namespace service
{
class IAndroidAutoInterface;
class MediaStatusService: public aasdk::channel::av::IMediaStatusServiceChannelEventHandler, public IService, public std::enable_shared_from_this<MediaStatusService>
{
public:
    MediaStatusService(boost::asio::io_service& ioService, aasdk::messenger::IMessenger::Pointer messenger, IAndroidAutoInterface* aa_interface);
    void start() override;
    void stop() override;
    void fillFeatures(aasdk::proto::messages::ServiceDiscoveryResponse& response) override;
    void onChannelOpenRequest(const aasdk::proto::messages::ChannelOpenRequest& request) override;
    void onChannelError(const aasdk::error::Error& e) override;
    void onMetadataUpdate(const aasdk::proto::messages::MediaInfoChannelMetadataData& metadata) override;
    void onPlaybackUpdate(const aasdk::proto::messages::MediaInfoChannelPlaybackData& playback) override;
    void setAndroidAutoInterface(IAndroidAutoInterface* aa_interface);


private:
    using std::enable_shared_from_this<MediaStatusService>::shared_from_this;

    boost::asio::io_service::strand strand_;
    aasdk::channel::av::MediaStatusServiceChannel::Pointer channel_;
    IAndroidAutoInterface* aa_interface_ = nullptr;
};

}
}
