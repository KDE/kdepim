/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __POPMAIL_SETUP_H
#define __POPMAIL_SETUP_H

#include <qtabdlg.h>
#include <qlined.h>
#include <qchkbox.h>
#include <qcombo.h>
#include <kfm.h>

class PopMailOptions : public QTabDialog
{
  Q_OBJECT

public:
  PopMailOptions();
  ~PopMailOptions();
  
public slots:
void commitChanges();
  void cancelChanges();
  void togglePopPass();
  void toggleUseSMTP();

private:
  // Email Prefs:
  QLineEdit* fEmailFrom;
  QLineEdit* fSignature;
  QLineEdit* fSendmailCmd;
  QLineEdit* fSMTPServer;
  QLineEdit* fSMTPPort;
  QCheckBox* fUseSMTP;
  
  QLineEdit* fPopServer;
  QLineEdit* fPopPort;
  QLineEdit* fPopUser;
  QCheckBox* fLeaveMail;
  QCheckBox* fSyncIncoming;
  QCheckBox* fSendOutgoing;
  QCheckBox* fStorePass;
  QLineEdit* fPopPass;
  void setupWidget();
};

#endif
