import QtQuick 1.1

Rectangle {
  visible: vacationManager.activeVacationScriptAvailable

  width: 26
  height: 26

  color: "red"

  MouseArea {
    anchors.fill: parent

    onClicked: vacationManager.editVacation();
  }
}
