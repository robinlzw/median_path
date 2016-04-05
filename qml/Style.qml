pragma Singleton
import QtQuick 2.3

QtObject {

  property QtObject z: QtObject {
    property int maximum: 2000
	  property int topology_button:1000
	  property int rendering_button:975
	  property int geometry_button:950
	  property int camera_button:925
	  property int color_button:900
    property int scene_window: 500
  }
  
  property QtObject geometry_button: QtObject {
    property color background_inactive: "#dbeafa"
    property color background_active: "#67a9f0"
    property color background_unavailable: "#4789d0"
    property color font_color_active: "#000000"
    property color font_color_inactive: "#858585"
    property color font_color_unavailable: "#a5a5a5"
    
    property int font_size: 10
    property int height: 20
    property int width: 100
  }
  
  property QtObject camera_button: QtObject {
    property color background_active: "#A29DA3"
    property color font_color_active: "#000000"
    
    property int font_size: 10
    property int height: 20
    property int width: 100
  }  
  
  property QtObject color_button: QtObject {
    property color background_active: "#004080"
    property color font_color_active: "#000000"
    
    property int font_size: 10
    property int height: 20
    property int width: 100
  }   
  
  property QtObject topology_button: QtObject {
    property color background_inactive: "#C4F5C5"
    property color background_active: "#89F58D"
    property color background_unavailable: "#69d56D"
    property color font_color_active: "#000000"
    property color font_color_inactive: "#858585"
    property color font_color_unavailable: "#a5a5a5"
    
    property int font_size: 10
    property int height: 20
    property int width: 200
  }
  
  property QtObject rendering_button: QtObject {
    property color background_inactive: "#ED91AF"
    property color background_active: "#EB427A"
    property color font_color_active: "#000000"
    property color font_color_inactive: "#858585"
    
    property int font_size: 10
    property int height: 20
    property int width: 250
  }
  
  property QtObject main_window: QtObject {
    property int initial_width: 800
    property int initial_height: 800
  }
}

