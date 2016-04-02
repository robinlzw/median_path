import QtQuick 2.5
import "."

Rectangle {
  width: Style.topology_button.width
  height: Style.topology_button.height
  property string label: "LABEL"
  property bool active: true
  property bool available: true
  color: available ? (active ? Style.topology_button.background_active : Style.topology_button.background_inactive) : Style.topology_button.background_unavailable
  Text {
    text: label
    anchors.centerIn: parent
    font.pointSize: Style.topology_button.font_size
    color: available ? (active ? Style.topology_button.font_color_active : Style.topology_button.font_color_inactive) : Style.topology_button.font_color_unavailable
  }
}