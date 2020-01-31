import QtQuick 2.7
import QtQuick.Controls 2.2

RoundButton {
    id: emojiButton
    font.bold: true
    font.italic: false
    font.underline: false

    contentItem: Label {
        text: emojiButton.text
        font.family: "Twitter Color Emoji"
        font.bold: emojiButton.font.bold
        font.pixelSize: emojiButton.font.pixelSize
        renderType: Text.QtRendering
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }
}
