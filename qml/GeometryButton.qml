import QtQuick 2.5
import "."

Rectangle {
  width: Style.geometry_button.width
  height: Style.geometry_button.height
  property string label: "LABEL"
  property bool active: true
  property bool available: true
  color: available ? ( active ? Style.geometry_button.background_active : Style.geometry_button.background_inactive) : Style.geometry_button.background_unavailable
  Text {
    text: label
    anchors.centerIn: parent
    font.pointSize: Style.geometry_button.font_size
    color: available ? (active ? Style.geometry_button.font_color_active : Style.geometry_button.font_color_inactive) : Style.geometry_button.font_color_unavailable
  }
}