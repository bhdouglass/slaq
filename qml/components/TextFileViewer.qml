import QtQuick 2.11

Item {
    width: textEdit.width
    height: textEdit.height

    function hovered(mouseArea) {}
    function clicked() {}

    Text {
        id: textEdit
        width: paintedWidth
        height: paintedHeight
        text: model.preview_highlight
        wrapMode: Text.WordWrap
        textFormat: Text.RichText
        renderType: Text.QtRendering
    }
}
