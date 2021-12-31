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

#ifdef USE_GST
#pragma once

#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <boost/circular_buffer.hpp>
#include <boost/noncopyable.hpp>
#include "openauto/Projection/VideoOutput.hpp"
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <gst/video/video.h>
#include <QGlib/Error>
#include <QGlib/Connect>
#include <QGst/Init>
#include <QGst/Bus>
#include <QGst/Pipeline>
#include <QGst/Parse>
#include <QGst/Message>
#include <QGst/Utils/ApplicationSink>
#include <QGst/Utils/ApplicationSource>
#include <QGst/Ui/VideoWidget>
#include <QGst/ElementFactory>
#include <QGst/Quick/VideoSurface>
#include <QtQml/QQmlContext>
#include <QtQuickWidgets/QQuickWidget>
#include <QApplication>

namespace openauto
{
namespace projection
{

class GSTVideoOutput: public QObject, public VideoOutput, boost::noncopyable
{
    Q_OBJECT

public:
    GSTVideoOutput(configuration::IConfiguration::Pointer configuration, QWidget* videoContainer=nullptr, std::function<void(bool)> activeCallback=nullptr);
    ~GSTVideoOutput();
    bool open() override;
    bool init() override;
    void write(uint64_t timestamp, const aasdk::common::DataConstBuffer& buffer) override;
    void stop() override;
    void resize();

signals:
    void startPlayback();
    void stopPlayback();

protected slots:
    void onStartPlayback();
    void onStopPlayback();

public slots:
    void dumpDot();
private:
    static GstPadProbeReturn convertProbe(GstPad* pad, GstPadProbeInfo* info, void*);
    static gboolean busCallback(GstBus*, GstMessage* message, gpointer*);

    QGst::ElementPtr videoSink_;
    QQuickWidget* videoWidget_;
    GstElement* vidPipeline_;
    GstVideoFilter* vidCrop_;
    GstAppSrc* vidSrc_;
    QWidget* videoContainer_;
    QGst::Quick::VideoSurface* surface_;
    std::function<void(bool)> activeCallback_;
};

}
}

#endif
