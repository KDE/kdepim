import Qt 4.6
import org.kde 4.5
import org.kde.akonadi 4.5
import "HeaderView.qml"

Rectangle {
  id: topLevel
  color: "white"
  height: 480
  width: 800

  Rectangle {
    id: homeScreen
    anchors.fill: parent
    color: "blue"
    Row {
      anchors.fill: parent
      Image {
        id: icon
        height: Math.min( parent.height, 256 )
        width: height
        anchors.verticalCenter: parent.verticalCenter
        source: KDE.iconPath( "kmail", height )
      }
      Text {
        text: "Welcome to KMail Mobile!!!"
        anchors.verticalCenter: icon.verticalCenter
      }
    }
    MouseArea  {
      anchors.fill: parent
      onClicked: topLevel.state = 'collectionListState'
    }
  }

  CollectionView {
    id: collectionList
    anchors.fill: parent
    model: collectionModel
    onCollectionSelected: topLevel.state = 'headerListState'
  }

  HeaderView {
    id: headerList
    anchors.fill: parent
    model: itemModel
  }

  states: [
    State {
      name: "homeScreenState"
      PropertyChanges {
        target: homeScreen;
        visible: true
      }
      PropertyChanges {
        target: collectionList
        visible: false
      }
      PropertyChanges {
        target: headerList
        visible: false
      }
    },
    State {
      name: "collectionListState"
      PropertyChanges {
        target: homeScreen
        visible: false
      }
      PropertyChanges {
        target: collectionList
        visible: true
      }
      PropertyChanges {
        target: headerList
        visible: false
      }
    },
    State {
      name: "headerListState"
      PropertyChanges {
        target: homeScreen
        visible: false
      }
      PropertyChanges {
        target: collectionList
        visible: false
      }
      PropertyChanges {
        target: headerList
        visible: true
      }
    }
  ]

  state: "homeScreenState";

  Binding {
    target: application
    property: "collectionRow"
    value: collectionList.currentIndex
  }

}
