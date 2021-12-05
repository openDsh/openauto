import QtQuick 2.4

import org.freedesktop.gstreamer.GLVideoItem 1.0

GstGLVideoItem {
    id: video
    objectName: "videoItem"
    anchors.centerIn: parent
    width: 500
    height: 500
}
