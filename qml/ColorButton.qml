import QtQuick 2.5
import "."

Rectangle {
  width: Style.color_button.width
  height: Style.color_button.height
  property string label: "LABEL"
  property bool active: true
  color: Style.color_button.background_active
  Text {
    text: label
    anchors.centerIn: parent
    font.pointSize: Style.color_button.font_size
    color: Style.color_button.font_color_active
  }
}