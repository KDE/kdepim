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

#ifndef KSYNC_OVERVIEWPART_H
#define KSYNC_OVERVIEWPART_H

#include <klocale.h>
#include <qpixmap.h>

#include <actionpart.h>

namespace KSync {

class OverviewWidget;

class OverviewPart : public ActionPart
{
  Q_OBJECT

  public:
    OverviewPart( QWidget *parent, const char *name,
	                QObject *object = 0, const char *name2 = 0,
                  const QStringList & = QStringList() );
    virtual ~OverviewPart();

    static KAboutData *createAboutData();

    QString type() const;
    QString title() const;
    QString description() const;
    bool hasGui() const;
    QPixmap *pixmap();
    QString iconName() const;
    QWidget *widget();

    void executeAction();

  private slots:
    void slotPartChanged( ActionPart * );
    void slotSyncProgress( ActionPart *, int, int );
    void slotProfileChanged( const Profile & );
    void slotStartSync();
    void slotDoneSync();

  private:
    QPixmap m_pixmap;
    OverView::Widget *m_widget;
};

}

#endif
