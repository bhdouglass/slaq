import QtQuick 2.7
import QtQuick.Controls 2.2
import ".."

Button {
    property real bgSizeW: Theme.headerSize
    property real bgSizeH: Theme.headerSize
    property real sizeW: Theme.headerSize
    property real sizeH: Theme.headerSize

    property alias source: avatarImage.source

    id: button
    //display: AbstractButton.IconOnly
    hoverEnabled: true
    background: Item {
        implicitHeight: bgSizeH
        implicitWidth: bgSizeW
    }
    contentItem: Image {
        id: avatarImage
        x: (bgSizeW - sizeW)/2
        y: (bgSizeH - sizeH)/2
        sourceSize: Qt.size(sizeW, sizeH)
        scale: hovered ? 1.1 : 1.0
        width: sizeW
        height: sizeH
        smooth: true
    }
}
