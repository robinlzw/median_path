import QtQuick 2.5
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import MedianPath 1.0
import "."

Rectangle {
  id: main
  width: Style.main_window.initial_width
  height: Style.main_window.initial_height
  color: Style.main_window.background
  visible: true
  
  GLCamera {
    id: cam1
    ratio: glwindow.height > 0 ? glwindow.width / glwindow.height : 1
  }
  
  MouseArea {
    anchors.fill: parent
    acceptedButtons: Qt.LeftButton | Qt.RightButton 
    
    property int prevX: -1
    property int prevY: -1
    
    onPositionChanged: {
      if( pressedButtons & Qt.RightButton ) {
        var dx = 0;
        var dy = 0;
        if( prevX > -1 ) {
          dx = 0.01 * (mouse.x - prevX)
        }
        if( prevY > -1 ) {
          dy = 0.01 * (mouse.y - prevY)
        }
        cam1.mouse_rotation( dx, dy )
        prevX = mouse.x
        prevY = mouse.y
      }
    }
  }
  
  GroupBox {
    title: "Geometry method"
    z: Style.z.top
    RowLayout {
      ExclusiveGroup{ id: geometry_method_group }
      RadioButton {
				id: voronoi_balls
      	text: "Voronoi Balls"
      	exclusiveGroup: geometry_method_group
    	}
    	
    	RadioButton {
      	id: polar_balls
      	text: "Polar Balls"
      	exclusiveGroup: geometry_method_group
    	}
    
    	RadioButton {
      	id: shrinking_balls
      	text: "Shrinking Balls"
      	exclusiveGroup: geometry_method_group
    	}
    }
  }
  
  
  GLWindow {
    id: glwindow
    width: parent.width
    height:parent.height
    z: Style.z.scene_window
    anchors.centerIn: parent
    camera: cam1
    focus: true
    
    Keys.onPressed: {
      if( event.key == Qt.Key_Q ) {
        cam1.set_go_left( true );
      	event.accepted = true;
      } else if( event.key == Qt.Key_D ) {
        cam1.set_go_right( true );
		 event.accepted = true;
      } else if( event.key == Qt.Key_Z ) {
        cam1.set_go_forward( true );
        event.accepted = true;
      } else if( event.key == Qt.Key_S ) {
        cam1.set_go_backward( true );
        event.accepted = true
  	  }
    }
      
    Keys.onReleased: {
      if( event.key == Qt.Key_Q ) {
        cam1.set_go_left( false );
        event.accepted = true;
      } else if( event.key == Qt.Key_D ) {
        cam1.set_go_right( false );
		event.accepted = true;
      } else if( event.key == Qt.Key_Z ) {
        cam1.set_go_forward( false );
        event.accepted = true;
      } else if( event.key == Qt.Key_S ) {
        cam1.set_go_backward( false );
        event.accepted = true
  	  }
  	}//end of Key.onReleased
  }//end of GLWindow
}