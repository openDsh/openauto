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

#include "aasdk/Channel/Navigation/NavigationStatusServiceChannel.hpp"
#include "IService.hpp"

namespace openauto
{
namespace service
{
class IAndroidAutoInterface;
class NavigationStatusService: public aasdk::channel::navigation::INavigationStatusServiceChannelEventHandler, public IService, public std::enable_shared_from_this<NavigationStatusService>
{
public:
    NavigationStatusService(boost::asio::io_service& ioService, aasdk::messenger::IMessenger::Pointer messenger, IAndroidAutoInterface* aa_interface);
    void start() override;
    void stop() override;
    void fillFeatures(aasdk::proto::messages::ServiceDiscoveryResponse& response) override;
    void onChannelOpenRequest(const aasdk::proto::messages::ChannelOpenRequest& request) override;
    void onChannelError(const aasdk::error::Error& e) override;
    void onTurnEvent(const aasdk::proto::messages::NavigationTurnEvent& turnEvent) override;
    void onDistanceEvent(const aasdk::proto::messages::NavigationDistanceEvent& distanceEvent) override;
    void onStatusUpdate(const aasdk::proto::messages::NavigationStatus& navStatus) override;
    void setAndroidAutoInterface(IAndroidAutoInterface* aa_interface);


private:
    using std::enable_shared_from_this<NavigationStatusService>::shared_from_this;

    boost::asio::io_service::strand strand_;
    aasdk::channel::navigation::NavigationStatusServiceChannel::Pointer channel_;
    IAndroidAutoInterface* aa_interface_ = nullptr;

};

}
}
