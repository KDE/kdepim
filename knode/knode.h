/*
    knode.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2004 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNODE_H
#define KNODE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kmainwindow.h>
#include <kdialogbase.h>
#include "resource.h"

#include <qglobal.h>

class KURL;

namespace KPIM { 
  class StatusbarProgressWidget; 
  class ProgressDialog; 
}
using KPIM::StatusbarProgressWidget;
using KPIM::ProgressDialog;

class KNMainWidget;
class KNHeaderView;


class KNMainWindow : public KMainWindow
{
  Q_OBJECT

public:
  KNMainWindow( QWidget* parentWidget=0 );
  ~KNMainWindow();
  void openURL( const KURL& );
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
private slots:
  void slotShowStatusMsg( const QString& );
};

#endif // KNODE_H
