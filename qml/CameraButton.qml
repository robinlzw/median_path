import QtQuick 2.5
import "."

Rectangle {
  width: Style.geometry_button.width
  height: Style.geometry_button.height
  property string label: "LABEL"
  Text {
    text: label
    anchors.centerIn: parent
    font.pointSize: Style.geometry_button.font_size
    color: Style.camera_button.font_color_active
  }
}