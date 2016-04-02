import QtQuick 2.5
import "."

Rectangle {
  width: Style.rendering_button.width
  height: Style.rendering_button.height
  property string label: "LABEL"
  property bool active: true
  color: active ? Style.rendering_button.background_active : Style.rendering_button.background_inactive
  Text {
    text: label
    anchors.centerIn: parent
    font.pointSize: Style.rendering_button.font_size
    color: active ? Style.rendering_button.font_color_active : Style.rendering_button.font_color_inactive
  }
}