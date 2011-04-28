
import Qt 4.7
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.kcal 4.5 as KCal

Item {
  Image {
    id: backgroundImage
    x: 0
    y: 0
    source: "notes-mobile-background.png"
    visible: collectionView.visible
  }

  Item {
    anchors.fill : parent
    anchors.leftMargin : 30
    CreateTrip {
      anchors.fill : parent
      visible : triplist.currentIndex == triplist.count - 1
    }

    Trip {
      anchors.fill : parent

      trip : triplist.currentItem.trip
      visible : triplist.currentIndex != triplist.count - 1
    }
  }

  SlideoutPanelContainer {
    id: panelContainer
    anchors.fill: parent

    SlideoutPanel {
      id: folderPanel
      titleText: KDE.i18n( "Trips" )
      handleHeight: 150
      contentWidth : 200
      content: [
        ListView {
          id : triplist
          anchors.top : parent.top
          anchors.bottom : parent.bottom
          width : 100

          currentIndex : count - 1

          model : _tripModel

          delegate : Text {
            property variant trip : model.trip
            height : 30
            text : model.display

            MouseArea {
              anchors.fill : parent
              onClicked : {
                triplist.currentIndex = model.index
              }
            }
          }
        }
      ]
    }
    SlideoutPanel {
      id: folderPanel2
      titleText: KDE.i18n( "Current trip" )
      handleHeight: 150
      handlePosition : 200
      content: [
        KCal.IncidenceView {
          id: eventView
          width: parent.width
          height: parent.height - deleteButton.height

          itemId: triplist.currentItem.trip ? triplist.currentItem.trip.id : -1
        },
        KPIM.Button2 {
          id : deleteButton
          height : 70
          width : parent.width
          anchors.bottom : parent.bottom
          buttonText : "Delete"
          onClicked : {

          }
        }
      ]
    }
  }

}
