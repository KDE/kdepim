/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KSYNC_VIEWER_H
#define KSYNC_VIEWER_H

#include <actionpart.h>

#include <klistview.h>
#include <klocale.h>

#include <qpixmap.h>
#include <qptrlist.h>

class KAboutData;

namespace KSync {

class Viewer : public ActionPart
{
  Q_OBJECT

  public:
    Viewer( QWidget *parent, const char *name, QObject *object = 0,
            const char *name2 = 0, const QStringList & = QStringList() );
    virtual ~Viewer();

    static KAboutData *createAboutData();

    QString type() const;
    QString title() const;
    QString description() const;
    bool hasGui() const;
    QPixmap *pixmap();
    QString iconName() const;
    QWidget *widget();

    void executeAction();

    bool needsKonnectorRead() const { return true; }

  protected slots:
    void expandAll();
    void collapseAll();

  private:
    QPixmap mPixmap;
    QWidget *mTopWidget;

    KListView *mListView;
};

}

#endif
