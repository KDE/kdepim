import QtQuick 1.1

Item {
  id: toggleswitch
  width: background.width
  height: background.height

  property bool on: false

  function setOn( value )
  {
    if ( value )
      toggleswitch.state = "on";
    else
      toggleswitch.state = "off";
  }

  function toggle() {
    if ( toggleswitch.state == "on" )
      toggleswitch.state = "off";
    else
      toggleswitch.state = "on";
  }

  function releaseSwitch() {
    if ( handle.x == 1 ) {
      if ( toggleswitch.state == "off" )
        return;
    }
    if ( handle.x == 78 ) {
      if ( toggleswitch.state == "on" )
        return;
    }

    toggle();
  }

  Image {
    id: background
    source : "images/sliderbackground.png";
    MouseArea {
      anchors.fill: parent
      onClicked: toggle()
    }
  }

  Image {
    id: handle
    x: 1;
    y: 2
    source : "images/sliderhandle.png";

    MouseArea {
      anchors.fill: parent
      drag.target: handle
      drag.axis: Drag.XAxis
      drag.minimumX: 1
      drag.maximumX: 78

      onClicked: toggle()
      onReleased: releaseSwitch()
    }
  }

  states: [
    State {
      name: "on"
      PropertyChanges { target: handle; x: 78 }
      PropertyChanges { target: toggleswitch; on: true }
    },
    State {
      name: "off"
      PropertyChanges { target: handle; x: 1 }
      PropertyChanges { target: toggleswitch; on: false }
    }
  ]

  transitions: Transition {
    NumberAnimation { properties: "x"; easing.type: Easing.InOutQuad; duration: 200 }
  }
}
