import QtQuick 2.7
import QtQuick.Controls 2.2

Item {
    width: textEdit.width
    height: textEdit.height

    function hovered(mouseArea) {}
    function clicked() {
        textViewer.showText(fileshare.url_private_download, fileshare.name, fileshare.username, teamRoot.teamId)
    }

    Label {
        id: textEdit
        width: paintedWidth
        height: paintedHeight
        text: fileshare.preview_highlight !== "" ? fileshare.preview_highlight : qsTr("Preview not available")
        wrapMode: Text.WordWrap
        textFormat: Text.RichText
        renderType: Text.QtRendering
    }
}

