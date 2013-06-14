import QtQuick 1.1

ListView {
  id: listView
  Image {
    source: KDE.locate( "lib", "kde4/imports/org/kde/scrollable-top.png" );
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.left: parent.left
    fillMode: Image.TileHorizontally
    visible: !listView.atYBeginning
  }
  Image {
    source: KDE.locate( "lib", "kde4/imports/org/kde/scrollable-bottom.png" );
    anchors.bottom: parent.bottom
    anchors.right: parent.right
    anchors.left: parent.left
    fillMode: Image.TileHorizontally
    visible: !listView.atYEnd
  }
}
