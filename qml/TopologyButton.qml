import QtQuick 2.5
import "."

Rectangle {
  width: Style.topology_button.width
  height: Style.topology_button.height
  property string label: "LABEL"
  property bool active: true
  color: active ? Style.topology_button.background_active : Style.topology_button.background_inactive
  Text {
    text: label
    anchors.centerIn: parent
    font.pointSize: Style.topology_button.font_size
    color: active ? Style.topology_button.font_color_active : Style.topology_button.font_color_inactive
  }
}