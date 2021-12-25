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
#include "openauto/Projection/InputEvent.hpp"
#include "aasdk_proto/MediaInfoChannelMetadataData.pb.h"
#include "aasdk_proto/MediaInfoChannelPlaybackData.pb.h"
#include "aasdk_proto/NavigationStatusMessage.pb.h"
#include "aasdk_proto/NavigationDistanceEventMessage.pb.h"
#include "aasdk_proto/NavigationTurnEventMessage.pb.h"
#include "openauto/Service/ServiceFactory.hpp"


namespace openauto
{
namespace service
{

class IAndroidAutoInterface: public std::enable_shared_from_this<IAndroidAutoInterface>
{
public:
    std::shared_ptr<IAndroidAutoInterface> getPtr()
     {
         return shared_from_this();
     }
    typedef std::shared_ptr<IAndroidAutoInterface> Pointer;

    virtual ~IAndroidAutoInterface() = default;

    virtual void mediaPlaybackUpdate(const aasdk::proto::messages::MediaInfoChannelPlaybackData& playback) = 0;
    virtual void mediaMetadataUpdate(const aasdk::proto::messages::MediaInfoChannelMetadataData& metadata) = 0;
    virtual void navigationStatusUpdate(const aasdk::proto::messages::NavigationStatus& navStatus) = 0;
    virtual void navigationTurnEvent(const aasdk::proto::messages::NavigationTurnEvent& turnEvent) = 0;
    virtual void navigationDistanceEvent(const aasdk::proto::messages::NavigationDistanceEvent& distanceEvent) = 0;
    void setServiceFactory(ServiceFactory* serviceFactory)
    {
        this->m_serviceFactory = serviceFactory;
        
    }
    void injectButtonPress(aasdk::proto::enums::ButtonCode::Enum buttonCode, projection::WheelDirection wheelDirection=openauto::projection::WheelDirection::NONE, projection::ButtonEventType buttonEventType = projection::ButtonEventType::NONE)
    {
        if(m_serviceFactory != NULL)
        {
            
            m_serviceFactory->sendButtonPress(buttonCode, wheelDirection, buttonEventType);
        }
    }
    void injectButtonPress(aasdk::proto::enums::ButtonCode::Enum buttonCode, projection::ButtonEventType buttonEventType)
    {
        this->injectButtonPress(buttonCode, projection::WheelDirection::NONE, buttonEventType);
    }
    void setNightMode(bool mode)
    {
        if(m_serviceFactory != NULL)
        {
            
            m_serviceFactory->setNightMode(mode);
        }
    }
    



private:
    using std::enable_shared_from_this<IAndroidAutoInterface>::shared_from_this;
    ServiceFactory* m_serviceFactory;
};



}
}
