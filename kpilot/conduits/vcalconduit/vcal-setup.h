/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __VCAL_SETUP_H
#define __VCAL_SETUP_H

#include <qdialog.h>
#include <qlined.h>
#include <qcheckbox.h>

class VCalSetup : public QDialog
{
  Q_OBJECT

public:
  VCalSetup();
  ~VCalSetup();
  
public slots:
void slotCommitChanges();
  void slotCancelChanges();
  void slotBrowse();

private:
  QLineEdit* fCalendarFile;
  QCheckBox* fPromptYesNo;

  void setupWidget();
};

#endif
