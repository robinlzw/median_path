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
  
  Row {
	id: geometry_selection
	z: Style.z.geometry_button
	y: 10
	x: 10
	spacing: 10
	
	GeometryButton {
	  id:voronoi_balls
	  label: "Voronoi Balls"
	    
	  MouseArea {
	    anchors.fill: parent
		onClicked: {
	      voronoi_balls.active = true
	      polar_balls.active = false
	      shrinking_balls.active = false
	      glwindow.request_voronoi_geometry()
        }
	  }
	}
	  
	GeometryButton {
	  id:polar_balls
	  label: "Polar Balls"
	  active: false
	  MouseArea {
	    anchors.fill: parent
		onClicked: {
		  if( voronoi_reconstruction.active )
		    {
		      voronoi_reconstruction.active = false
		      delaunay_reconstruction.active = true
		      glwindow.request_delaunay_reconstruction()
		    }
		
		  voronoi_balls.active = false
		  polar_balls.active = true
		  shrinking_balls.active = false
		  glwindow.request_polar_geometry()
		}
	  }	    
	}
	  
	GeometryButton {
	  id:shrinking_balls
	  label: "Shrinking Balls"
	  active: false
	  MouseArea {
	    anchors.fill: parent
		onClicked: {
		  if( voronoi_reconstruction.active || powershape_reconstruction.active )
		    {
		      voronoi_reconstruction.active = false
		      powershape_reconstruction.active = false
		      delaunay_reconstruction.active = true
		      glwindow.request_delaunay_reconstruction()
		    }
		
		  voronoi_balls.active = false
		  polar_balls.active = false
		  shrinking_balls.active = true
		  glwindow.request_shrinking_geometry()
		}
	  }	    
	}
  } // end of Geometry Methods row
  
  Column {
    spacing: 10
    id: topology_selection
	z: Style.z.topology_button
	y: geometry_selection.y + 10 + Style.geometry_button.height
	x: 10
  
    TopologyButton {
      id: delaunay_reconstruction
      label: "Delaunay Reconstruction"
      active: true
      MouseArea {
        anchors.fill: parent
        onClicked: {
          delaunay_reconstruction.active = true
          weighted_alpha_reconstruction.active = false
          powershape_reconstruction.active = false
          voronoi_reconstruction.active = false
          glwindow.request_delaunay_reconstruction()
        }
      }
    }
    TopologyButton {
      id: weighted_alpha_reconstruction
      label: "Weighted Alpha Reconstruction"
      active: false
      MouseArea {
        anchors.fill: parent
        onClicked: {
          delaunay_reconstruction.active = false
          weighted_alpha_reconstruction.active = true
          powershape_reconstruction.active = false
          voronoi_reconstruction.active = false
          glwindow.request_weighted_alpha_reconstruction()
        }
      }
    }
    TopologyButton {
      id: powershape_reconstruction
      label: "Powershape Reconstruction"
      active: false
      visible: voronoi_balls.active || polar_balls.active
      MouseArea {
        anchors.fill: parent
        onClicked: {
          if( powershape_reconstruction.visible )
          {
            delaunay_reconstruction.active = false
            weighted_alpha_reconstruction.active = false
            powershape_reconstruction.active = true
            voronoi_reconstruction.active = false
            glwindow.request_powershape_reconstruction()
          }
        }
      }
    }
    TopologyButton {
      id: voronoi_reconstruction
      label: "Voronoi Reconstruction"
      active: false
      visible: voronoi_balls.active
      MouseArea {
        anchors.fill: parent
        onClicked: {
          if( voronoi_reconstruction.visible )
          {
            delaunay_reconstruction.active = false
            weighted_alpha_reconstruction.active = false
            powershape_reconstruction.active = false
            voronoi_reconstruction.active = true
            glwindow.request_voronoi_reconstruction()
          }
        }
      }
    }        
  
  }
  
  
  GLCamera {
    id: cam1
    ratio: glwindow.height > 0 ? glwindow.width / glwindow.height : 1
  }
  
  GLWindow {
    id: glwindow
    objectName: "glwindow"
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
