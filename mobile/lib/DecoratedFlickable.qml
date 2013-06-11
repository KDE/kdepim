import QtQuick 1.1

Item {
  property alias content: flickable.contentItem
  property alias contentHeight: flickable.contentHeight
  property alias contentWidth: flickable.contentWidth
  Flickable {
    anchors.fill: parent
    id: flickable
    flickableDirection: Flickable.VerticalFlick
    boundsBehavior: Flickable.StopAtBounds
    clip: true
  }
  Image {
    source: KDE.locate( "lib", "kde4/imports/org/kde/scrollable-top.png" );
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.left: parent.left
    fillMode: Image.TileHorizontally
    visible: !flickable.atYBeginning
  }
  Image {
    source: KDE.locate( "lib", "kde4/imports/org/kde/scrollable-bottom.png" );
    anchors.bottom: parent.bottom
    anchors.right: parent.right
    anchors.left: parent.left
    fillMode: Image.TileHorizontally
    visible: !flickable.atYEnd
  }
}
