
import Qt 4.7
import org.kde.kcal 4.5 as KCal
import org.kde.messageviewer 4.5 as MessageViewer

Item {

  KCal.IncidenceView {
    id: eventView
    anchors.fill: parent

    visible : triplist.currentItem.trip ? triplist.currentItem.trip.todoSelection ? triplist.currentItem.trip.todoSelection.hasSelection : false : false

    itemId: triplist.currentItem.trip ? triplist.currentItem.trip.itemSelection.id : -1
  }

  NoteView {
    id: noteView
    objectName : "noteView"
    visible : triplist.currentItem.trip ? triplist.currentItem.trip.notesSelection ? triplist.currentItem.trip.notesSelection.hasSelection : false : false
    anchors.fill: parent

    itemSelection : triplist.currentItem.trip ? triplist.currentItem.trip.itemSelection : null

    Rectangle {
      anchors.top : noteView.top
      anchors.bottom : noteView.bottom
      anchors.right : noteView.left
      width : noteView.anchors.leftMargin
      color : "#FAFAFA"
    }
  }

  MessageViewer.MessageView {
    id: messageView
    visible : triplist.currentItem.trip ? triplist.currentItem.trip.mailSelection ? triplist.currentItem.trip.mailSelection.hasSelection : false : false
    anchors.fill: parent
    itemId: triplist.currentItem.trip ? triplist.currentItem.trip.itemSelection.id : -1
  }

  Rectangle {
    id : backToMessageListButton
    visible: triplist.currentItem.trip.itemSelection.id != -1
    anchors.right : parent.right
    anchors.rightMargin : 70
    anchors.bottom : parent.bottom
    anchors.bottomMargin : 100
    Image {
      source : KDE.locate( "data", "mobileui/back-to-list-button.png" );
      MouseArea {
        anchors.fill : parent;
        onClicked : {
          console.log(triplist.currentItem.trip, triplist.currentItem.trip.itemSelection)
          triplist.currentItem.trip.itemSelection.clear()
        }
      }
    }
  }


}
