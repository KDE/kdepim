/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNODE_H
#define KNODE_H

#include <kxmlguiwindow.h>
#include <kdialog.h>
#include "resource.h"

#include <qglobal.h>

class KUrl;

namespace KPIM {
  class StatusbarProgressWidget;
  class ProgressDialog;
}
using KPIM::StatusbarProgressWidget;
using KPIM::ProgressDialog;
class KSqueezedTextLabel;

class KNMainWidget;


/** KNode main window. */
class KNMainWindow : public KXmlGuiWindow
{
  Q_OBJECT

public:
  explicit KNMainWindow( QWidget* parent = 0 );
  ~KNMainWindow();
  void openURL( const KUrl& );
  KNMainWidget *mainWidget() { return m_mainWidget; }

public slots:
  void slotConfToolbar();
  void slotNewToolbarConfig();
  void slotConfKeys();
protected:
  bool queryClose();
private:
  void setupStatusBar();
  KNMainWidget *m_mainWidget;
  StatusbarProgressWidget *mLittleProgress;
  ProgressDialog *mProgressDialog;
  KSqueezedTextLabel *mStatusMsgLabel;
private slots:
  void slotShowStatusMsg( const QString& );
};

#endif // KNODE_H
