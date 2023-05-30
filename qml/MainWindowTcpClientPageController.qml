import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SAK.Custom
import "common"

EDPane {
    id: root
    padding: 0

    property alias deviceController: controller

    DevicePageCommonClient {
        id: controller
        anchors.fill: parent
        title: qsTr("TCP client settings")
    }
}
