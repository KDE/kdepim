/*
    knode.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
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

class KNMainWidget;
class KNListView;


class KNMainWindow : public KMainWindow
{
  Q_OBJECT

public:
  KNMainWindow( QWidget* parentWidget=0 );
  ~KNMainWindow();
  void openURL( const KURL& );

public slots:
  void slotConfToolbar();
  void slotNewToolbarConfig();
protected:
  bool queryClose();
private:
  KNMainWidget *m_mainWidget;
};

#endif // KNODE_H
