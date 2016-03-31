import QtQuick 2.5
import "."

Rectangle {
  width: Style.geometry_button.width
  height: Style.geometry_button.height
  property string label: "LABEL"
  property bool active: true
  color: active ? Style.geometry_button.background_active : Style.geometry_button.background_inactive
  Text {
    text: label
    anchors.centerIn: parent
    font.pointSize: Style.geometry_button.font_size
    color: active ? Style.geometry_button.font_color_active : Style.geometry_button.font_color_inactive
  }
}