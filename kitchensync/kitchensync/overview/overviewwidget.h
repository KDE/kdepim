/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
† † Copyright (c) 2002 Maximilian Reiﬂ <harlekin@handhelds.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KSYNC_OVERVIEW_WIDGET_H
#define KSYNC_OVERVIEW_WIDGET_H

#include <qlabel.h>
#include <qlayout.h>
#include <qptrlist.h>
#include <qwidget.h>

#include "overviewprogressentry.h"

class QSplitter;
class QTextEdit;

namespace KSync {

class KonnectorProfile;
class Profile;

namespace OverView {

/**
  This is the MainWidget of the OverView and the only interface to the part...
 */
class Widget : public QWidget
{
  Q_OBJECT

  public:
    Widget( QWidget* parent, const char* name );
    ~Widget();

    void setProfile( const Profile& );
    void setProfile( const QString&,const QPixmap& pix );
    void syncProgress( ActionPart*, int, int);
    void startSync();
    void cleanView();

  private:
    int m_layoutFillIndex;
    QLabel* m_device;
    QLabel* m_profile;
    QLabel* m_logo;
    QVBoxLayout* m_layout;
    QPtrList<OverViewProgressEntry> m_messageList;
    QSplitter *m_split;
    QWidget* m_ab;
    QTextEdit* m_edit;
};

}

}

#endif
