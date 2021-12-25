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

#include "OpenautoLog.hpp"
#include "openauto/Projection/IInputDeviceEventHandler.hpp"
#include "openauto/Projection/InputDevice.hpp"
#include <QDebug>

namespace openauto
{
namespace projection
{

InputDevice::InputDevice(QObject& parent, configuration::IConfiguration::Pointer configuration, const QRect& touchscreenGeometry, const QRect& displayGeometry)
    : parent_(parent)
    , configuration_(std::move(configuration))
    , touchscreenGeometry_(touchscreenGeometry)
    , displayGeometry_(displayGeometry)
    , eventHandler_(nullptr)
{
    this->moveToThread(parent.thread());
    pointer_id_queue.push(INT_MAX);
}

void InputDevice::start(IInputDeviceEventHandler& eventHandler)
{
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    OPENAUTO_LOG(info) << "[InputDevice] start.";
    eventHandler_ = &eventHandler;
    parent_.installEventFilter(this);
}

void InputDevice::stop()
{
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    OPENAUTO_LOG(info) << "[InputDevice] stop.";
    parent_.removeEventFilter(this);
    eventHandler_ = nullptr;
}

bool InputDevice::eventFilter(QObject* obj, QEvent* event)
{
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    if(eventHandler_ != nullptr)
    {
        if(event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
        {
            QKeyEvent* key = static_cast<QKeyEvent*>(event);
            if(!key->isAutoRepeat())
            {
                return this->handleKeyEvent(event, key);
            }
        }
        else if(event->type() == QEvent::TouchBegin || event->type() == QEvent::TouchUpdate || event->type() == QEvent::TouchEnd || event->type() == QEvent::TouchCancel)
        {
            return this->handleTouchEvent(event);
        }
        else if(event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove)
        {
            return this->handleMouseEvent(event);
        }
    }

    return QObject::eventFilter(obj, event);
}

bool InputDevice::handleKeyEvent(QEvent* event, QKeyEvent* key)
{
    auto eventType = event->type() == QEvent::KeyPress ? ButtonEventType::PRESS : ButtonEventType::RELEASE;
    aasdk::proto::enums::ButtonCode::Enum buttonCode;
    WheelDirection wheelDirection = WheelDirection::NONE;

    switch(key->key())
    {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        buttonCode = aasdk::proto::enums::ButtonCode::ENTER;
        break;

    case Qt::Key_Left:
        buttonCode = aasdk::proto::enums::ButtonCode::LEFT;
        break;

    case Qt::Key_Right:
        buttonCode = aasdk::proto::enums::ButtonCode::RIGHT;
        break;

    case Qt::Key_Up:
        buttonCode = aasdk::proto::enums::ButtonCode::UP;
        break;

    case Qt::Key_Down:
        buttonCode = aasdk::proto::enums::ButtonCode::DOWN;
        break;

    case Qt::Key_Escape:
        buttonCode = aasdk::proto::enums::ButtonCode::BACK;
        break;

    case Qt::Key_H:
        buttonCode = aasdk::proto::enums::ButtonCode::HOME;
        break;

    case Qt::Key_P:
        buttonCode = aasdk::proto::enums::ButtonCode::PHONE;
        break;

    case Qt::Key_O:
        buttonCode = aasdk::proto::enums::ButtonCode::CALL_END;
        break;

    case Qt::Key_X:
        buttonCode = aasdk::proto::enums::ButtonCode::PLAY;
        break;

    case Qt::Key_C:
        buttonCode = aasdk::proto::enums::ButtonCode::PAUSE;
        break;

    case Qt::Key_MediaPrevious:
    case Qt::Key_V:
        buttonCode = aasdk::proto::enums::ButtonCode::PREV;
        break;

    case Qt::Key_MediaPlay:
    case Qt::Key_B:
        buttonCode = aasdk::proto::enums::ButtonCode::TOGGLE_PLAY;
        break;

    case Qt::Key_MediaNext:
    case Qt::Key_N:
        buttonCode = aasdk::proto::enums::ButtonCode::NEXT;
        break;

    case Qt::Key_M:
        buttonCode = aasdk::proto::enums::ButtonCode::MICROPHONE_1;
        break;

    case Qt::Key_1:
        wheelDirection = WheelDirection::LEFT;
        eventType = ButtonEventType::NONE;
        buttonCode = aasdk::proto::enums::ButtonCode::SCROLL_WHEEL;
        break;

    case Qt::Key_2:
        wheelDirection = WheelDirection::RIGHT;
        eventType = ButtonEventType::NONE;
        buttonCode = aasdk::proto::enums::ButtonCode::SCROLL_WHEEL;
        break;

    default:
        return true;
    }

    const auto& buttonCodes = this->getSupportedButtonCodes();
    if(std::find(buttonCodes.begin(), buttonCodes.end(), buttonCode) != buttonCodes.end())
    {
        if(buttonCode != aasdk::proto::enums::ButtonCode::SCROLL_WHEEL || event->type() == QEvent::KeyRelease)
        {
            eventHandler_->onButtonEvent({eventType, wheelDirection, buttonCode});
        }
    }

    return true;
}
bool InputDevice::handleTouchEvent(QEvent* event)
{
    if(!configuration_->getTouchscreenEnabled())
    {
        return true;
    }

    
    QTouchEvent* qtTouchEvent = static_cast<QTouchEvent*>(event);
    aasdk::proto::enums::TouchAction::Enum type;
    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
    aasdk::proto::messages::InputEventIndication inputEventIndication;
    inputEventIndication.set_timestamp(timestamp.count());
    auto touchEvent = inputEventIndication.mutable_touch_event();
    switch(event->type()){
        case QEvent::TouchBegin:
            type = aasdk::proto::enums::TouchAction::PRESS;
            break;
        case QEvent::TouchUpdate:
            if(qtTouchEvent->touchPointStates().testFlag(Qt::TouchPointPressed)) type = aasdk::proto::enums::TouchAction::POINTER_DOWN;
            else if(qtTouchEvent->touchPointStates().testFlag(Qt::TouchPointReleased)) type = aasdk::proto::enums::TouchAction::POINTER_UP;
            else{
                type = aasdk::proto::enums::TouchAction::DRAG;
                touchEvent->set_action_index(0);
            }
            break;
        case QEvent::TouchEnd:
            type = aasdk::proto::enums::TouchAction::RELEASE;
            break;
        case QEvent::TouchCancel:
            type = aasdk::proto::enums::TouchAction::RELEASE;
        default:
            return true;
    }

    touchEvent->set_touch_action(type);

    auto pointers = qtTouchEvent->touchPoints();

    // What's with this pointer map and pointer id queue?
    // Pointers get a unique id during their lifespan, and android auto gets pissy when you try and just use the unique id qt assigns
    // but it's happier with smaller valued ids
    // and since there are no garuntees about id range, or similar (because of drivers and whatnot)
    // pointer id queue is a min heap, with the first item being the next availible "smaller id"
    // and pointer maps stores the qt id: small id relation
    // the pointer id queue will expand as needed

    // I kinda hate this, but it works

    for(int i=0; i<pointers.count(); i++)
    {
        if(pointers[i].state() == Qt::TouchPointPressed){
            touchEvent->set_action_index(i);
            if(pointer_id_queue.top() == INT_MAX){
                pointer_id_queue.push(max_pointers++);
            }
            pointer_map.insert(pointers[i].id(), pointer_id_queue.top());
            pointer_id_queue.pop();
        }
        else if(pointers[i].state() == Qt::TouchPointReleased){
            touchEvent->set_action_index(i);
            pointer_id_queue.push(pointer_map.take(pointers[i].id()));
        }
        
        auto touchLocation = touchEvent->add_touch_location();
        touchLocation->set_x((static_cast<float>(pointers[i].pos().x()) / touchscreenGeometry_.width()) * displayGeometry_.width());
        touchLocation->set_y((static_cast<float>(pointers[i].pos().y()) / touchscreenGeometry_.height()) * displayGeometry_.height());
        touchLocation->set_pointer_id(pointer_map[pointers[i].id()]);
    }

    eventHandler_->onTouchEvent(inputEventIndication);

    return true;

}
bool InputDevice::handleMouseEvent(QEvent* event)
{
    if(!configuration_->getTouchscreenEnabled())
    {
        return true;
    }

    aasdk::proto::enums::TouchAction::Enum type;

    switch(event->type())
    {
    case QEvent::MouseButtonPress:
        type = aasdk::proto::enums::TouchAction::PRESS;
        break;
    case QEvent::MouseButtonRelease:
        type = aasdk::proto::enums::TouchAction::RELEASE;
        break;
    case QEvent::MouseMove:
        type = aasdk::proto::enums::TouchAction::DRAG;
        break;
    default:
        return true;
    };

    QMouseEvent* mouse = static_cast<QMouseEvent*>(event);
    if(event->type() == QEvent::MouseButtonRelease || mouse->buttons().testFlag(Qt::LeftButton))
    {
        const uint32_t x = (static_cast<float>(mouse->pos().x()) / touchscreenGeometry_.width()) * displayGeometry_.width();
        const uint32_t y = (static_cast<float>(mouse->pos().y()) / touchscreenGeometry_.height()) * displayGeometry_.height();
        eventHandler_->onMouseEvent({type, x, y, 0});
    }

    return true;
}

bool InputDevice::hasTouchscreen() const
{
    return configuration_->getTouchscreenEnabled();
}

QRect InputDevice::getTouchscreenGeometry() const
{
    return touchscreenGeometry_;
}

void InputDevice::setTouchscreenGeometry(QRect& touchscreenGeometry)
{
    touchscreenGeometry_ = touchscreenGeometry;
}

IInputDevice::ButtonCodes InputDevice::getSupportedButtonCodes() const
{
    return configuration_->getButtonCodes();
}

}
}
