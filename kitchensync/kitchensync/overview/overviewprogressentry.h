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

#ifndef OVERVIEWPROGRESSENTRY_H
#define OVERVIEWPROGRESSENTRY_H

#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qwidget.h>

namespace KSync {

namespace OverView {

class OverViewProgressEntry : public QWidget
{
  Q_OBJECT

  public:
    OverViewProgressEntry( QWidget* parent, const char* name );
    ~OverViewProgressEntry();

    void setText( const QString& );
    void setProgress( int );
    void setPixmap( const QPixmap& );
    QString name();

  private:
    QString m_name;
    QLabel* m_textLabel;
    QLabel* m_progressField;
    QLabel* m_pixmapLabel;
};

}

}

#endif
