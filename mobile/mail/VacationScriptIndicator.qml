import Qt 4.7

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
