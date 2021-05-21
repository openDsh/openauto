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

#include <QApplication>
#include <QScreen>
#include "aasdk/Channel/AV/MediaAudioServiceChannel.hpp"
#include "aasdk/Channel/AV/SystemAudioServiceChannel.hpp"
#include "aasdk/Channel/AV/SpeechAudioServiceChannel.hpp"
#include "openauto/Service/ServiceFactory.hpp"
#include "openauto/Service/VideoService.hpp"
#include "openauto/Service/MediaAudioService.hpp"
#include "openauto/Service/SpeechAudioService.hpp"
#include "openauto/Service/SystemAudioService.hpp"
#include "openauto/Service/AudioInputService.hpp"
#include "openauto/Service/SensorService.hpp"
#include "openauto/Service/BluetoothService.hpp"
#include "openauto/Service/InputService.hpp"
#include "openauto/Projection/QtVideoOutput.hpp"
#include "openauto/Projection/GSTVideoOutput.hpp"
#include "openauto/Projection/OMXVideoOutput.hpp"
#include "openauto/Projection/RtAudioOutput.hpp"
#include "openauto/Projection/QtAudioOutput.hpp"
#include "openauto/Projection/QtAudioInput.hpp"
#include "openauto/Projection/InputDevice.hpp"
#include "openauto/Projection/LocalBluetoothDevice.hpp"
#include "openauto/Projection/RemoteBluetoothDevice.hpp"
#include "openauto/Projection/DummyBluetoothDevice.hpp"

namespace openauto
{
namespace service
{

ServiceFactory::ServiceFactory(boost::asio::io_service& ioService, configuration::IConfiguration::Pointer configuration, QWidget *activeArea, std::function<void(bool)> activeCallback, bool nightMode)
    : ioService_(ioService)
    , configuration_(std::move(configuration))
    , activeArea_(activeArea)
    , screenGeometry_(this->mapActiveAreaToGlobal(activeArea_))
    , activeCallback_(activeCallback)
#if defined USE_OMX
    , omxVideoOutput_(std::make_shared<projection::OMXVideoOutput>(configuration_, this->QRectToDestRect(screenGeometry_), activeCallback_))
#elif defined USE_GST
    , gstVideoOutput_((QGst::init(nullptr, nullptr), std::make_shared<projection::GSTVideoOutput>(configuration_, activeArea_, activeCallback_)))
#else
    , qtVideoOutput_(nullptr)
#endif
    , btservice_(configuration_)
    , nightMode_(nightMode)
{

}

ServiceList ServiceFactory::create(aasdk::messenger::IMessenger::Pointer messenger)
{
    ServiceList serviceList;

    projection::IAudioInput::Pointer audioInput(new projection::QtAudioInput(1, 16, 16000), std::bind(&QObject::deleteLater, std::placeholders::_1));
    serviceList.emplace_back(std::make_shared<AudioInputService>(ioService_, messenger, std::move(audioInput)));
    this->createAudioServices(serviceList, messenger);

    std::shared_ptr<SensorService> sensorService = std::make_shared<SensorService>(ioService_, messenger, nightMode_);
    sensorService_ = sensorService;
    serviceList.emplace_back(sensorService);

    serviceList.emplace_back(this->createVideoService(messenger));
    serviceList.emplace_back(this->createBluetoothService(messenger));
    serviceList.emplace_back(this->createInputService(messenger));

    return serviceList;
}

IService::Pointer ServiceFactory::createVideoService(aasdk::messenger::IMessenger::Pointer messenger)
{
#if defined USE_OMX
    auto videoOutput(omxVideoOutput_);
#elif defined USE_GST
    auto videoOutput(gstVideoOutput_);
#else
    qtVideoOutput_ = new projection::QtVideoOutput(configuration_, activeArea_);
    if(activeCallback_ != nullptr)
    {
        QObject::connect(qtVideoOutput_, &projection::QtVideoOutput::startPlayback, [callback = activeCallback_]() { callback(true); });
        QObject::connect(qtVideoOutput_, &projection::QtVideoOutput::stopPlayback, [this]() {
            activeCallback_(false);
            qtVideoOutput_->disconnect();
            qtVideoOutput_ = nullptr;
        });
    }
    projection::IVideoOutput::Pointer videoOutput(qtVideoOutput_, std::bind(&QObject::deleteLater, std::placeholders::_1));
#endif
    return std::make_shared<VideoService>(ioService_, messenger, std::move(videoOutput));
}

IService::Pointer ServiceFactory::createBluetoothService(aasdk::messenger::IMessenger::Pointer messenger)
{
    projection::IBluetoothDevice::Pointer bluetoothDevice;
    switch(configuration_->getBluetoothAdapterType())
    {
    case configuration::BluetoothAdapterType::LOCAL:
        bluetoothDevice = projection::IBluetoothDevice::Pointer(new projection::LocalBluetoothDevice(), std::bind(&QObject::deleteLater, std::placeholders::_1));
        break;

    case configuration::BluetoothAdapterType::REMOTE:
        bluetoothDevice = std::make_shared<projection::RemoteBluetoothDevice>(configuration_->getBluetoothRemoteAdapterAddress());
        break;

    default:
        bluetoothDevice = std::make_shared<projection::DummyBluetoothDevice>();
        break;
    }

    return std::make_shared<BluetoothService>(ioService_, messenger, std::move(bluetoothDevice));
}

IService::Pointer ServiceFactory::createInputService(aasdk::messenger::IMessenger::Pointer messenger)
{
    QRect videoGeometry;
    switch(configuration_->getVideoResolution())
    {
    case aasdk::proto::enums::VideoResolution::_720p:
        videoGeometry = QRect(0, 0, 1280, 720);
        break;

    case aasdk::proto::enums::VideoResolution::_1080p:
        videoGeometry = QRect(0, 0, 1920, 1080);
        break;

    default:
        videoGeometry = QRect(0, 0, 800, 480);
        break;
    }

    //account for margins being applied to android auto
    videoGeometry.setWidth(videoGeometry.width()-configuration_->getVideoMargins().width());
    videoGeometry.setHeight(videoGeometry.height()-configuration_->getVideoMargins().height());


    QObject* inputObject = activeArea_ == nullptr ? qobject_cast<QObject*>(QApplication::instance()) : qobject_cast<QObject*>(activeArea_);
    inputDevice_ = std::make_shared<projection::InputDevice>(*inputObject, configuration_, std::move(screenGeometry_), std::move(videoGeometry));

    return std::make_shared<InputService>(ioService_, messenger, std::move(projection::IInputDevice::Pointer(inputDevice_)));
}

void ServiceFactory::createAudioServices(ServiceList& serviceList, aasdk::messenger::IMessenger::Pointer messenger)
{
    if(configuration_->musicAudioChannelEnabled())
    {
        auto mediaAudioOutput = configuration_->getAudioOutputBackendType() == configuration::AudioOutputBackendType::RTAUDIO ?
                    std::make_shared<projection::RtAudioOutput>(2, 16, 48000) :
                    projection::IAudioOutput::Pointer(new projection::QtAudioOutput(2, 16, 48000), std::bind(&QObject::deleteLater, std::placeholders::_1));

        serviceList.emplace_back(std::make_shared<MediaAudioService>(ioService_, messenger, std::move(mediaAudioOutput)));
    }

    if(configuration_->speechAudioChannelEnabled())
    {
        auto speechAudioOutput = configuration_->getAudioOutputBackendType() == configuration::AudioOutputBackendType::RTAUDIO ?
                    std::make_shared<projection::RtAudioOutput>(1, 16, 16000) :
                    projection::IAudioOutput::Pointer(new projection::QtAudioOutput(1, 16, 16000), std::bind(&QObject::deleteLater, std::placeholders::_1));

        serviceList.emplace_back(std::make_shared<SpeechAudioService>(ioService_, messenger, std::move(speechAudioOutput)));
    }

    auto systemAudioOutput = configuration_->getAudioOutputBackendType() == configuration::AudioOutputBackendType::RTAUDIO ?
                std::make_shared<projection::RtAudioOutput>(1, 16, 16000) :
                projection::IAudioOutput::Pointer(new projection::QtAudioOutput(1, 16, 16000), std::bind(&QObject::deleteLater, std::placeholders::_1));

    serviceList.emplace_back(std::make_shared<SystemAudioService>(ioService_, messenger, std::move(systemAudioOutput)));
}

void ServiceFactory::setOpacity(unsigned int alpha)
{
#ifdef USE_OMX
    if(omxVideoOutput_ != nullptr)
    {
        omxVideoOutput_->setOpacity(alpha);
    }
#endif
}

void ServiceFactory::resize()
{
    screenGeometry_ = this->mapActiveAreaToGlobal(activeArea_);
    if(inputDevice_ != nullptr)
    {
        inputDevice_->setTouchscreenGeometry(screenGeometry_);
    }
#if defined USE_OMX
    if(omxVideoOutput_ != nullptr)
    {
        omxVideoOutput_->setDestRect(this->QRectToDestRect(screenGeometry_));
    }
#elif defined USE_GST
    if(gstVideoOutput_ != nullptr)
    {
        gstVideoOutput_->resize();
    }
#else
    if(qtVideoOutput_ != nullptr)
    {
        qtVideoOutput_->resize();
    }
#endif
}

void ServiceFactory::setNightMode(bool nightMode)
{
    nightMode_ = nightMode;
    if(std::shared_ptr<SensorService> sensorService = sensorService_.lock())
    {
        sensorService->setNightMode(nightMode_);
    }
}

void ServiceFactory::sendKeyEvent(QKeyEvent* event)
{
    if(inputDevice_ != nullptr)
    {
        inputDevice_->eventFilter(activeArea_, event);
    }
}

QRect ServiceFactory::mapActiveAreaToGlobal(QWidget* activeArea)
{
    if(activeArea == nullptr)
    {
        QScreen* screen = QGuiApplication::primaryScreen();
        return screen == nullptr ? QRect(0, 0, 1, 1) : screen->geometry();
    }

    QRect g = activeArea->geometry();
    QPoint p = activeArea->mapToGlobal(g.topLeft());

    return QRect(p.x(), p.y(), g.width(), g.height());
}

#ifdef USE_OMX
projection::DestRect ServiceFactory::QRectToDestRect(QRect rect)
{
    return projection::DestRect(rect.x(), rect.y(), rect.width(), rect.height());
}
#endif

}
}
