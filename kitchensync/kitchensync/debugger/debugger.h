/*
    This file is part of KitchenSync.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KSYNC_DEBUGGER
#define KSYNC_DEBUGGER

#include <klocale.h>
#include <qpixmap.h>

#include <manipulatorpart.h>

class KAboutData;

namespace KSync {

class Debugger : public ManipulatorPart
{
   Q_OBJECT
  public:
    Debugger( QWidget *parent, const char *name,
              QObject *object=0, const char *name2 = 0, // make GenericFactory loading possible
              const QStringList & = QStringList() );
    virtual ~Debugger();

    static KAboutData *createAboutData();

    QString type() const;
    QString name() const;
    QString description() const;
    bool partIsVisible() const;
    QPixmap *pixmap();
    QString iconName() const;
    QWidget* widget();

  private:
    QPixmap m_pixmap;
    QWidget *m_widget;
};

}

#endif
