
import Qt 4.7

Column {
  property alias typename : buttonText.text

  property alias listDelegate : list.delegate

  Rectangle {
    color : "blue"
    Text {
      id : buttonText
      anchors.centerIn : parent
    }
    MouseArea {
      anchors.fill : parent
    }
    height : 100
    width : 100
  }
  ListView {
    // TODO: Need anchors to make this implicit?
    height : 200
    width : 100
    model : 5
    id : list

    delegate : Text {
      height : 30
      text : index
    }
  }
}
