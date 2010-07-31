/*
    This file is part of KDE.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KWINDOWPOSITIONER_H
#define KWINDOWPOSITIONER_H

#include <tqobject.h>

class QWindow;

class KWindowPositioner : public QObject
{
    Q_OBJECT
  public:
    enum Mode { Right, Bottom };

    KWindowPositioner( TQWidget *master, TQWidget *slave, Mode mode = Bottom );

    bool eventFilter( TQObject *watched, TQEvent *e );

    void reposition();
    
  private:
    TQWidget *mMaster;
    TQWidget *mSlave;
    
    Mode mMode;
};

#endif
