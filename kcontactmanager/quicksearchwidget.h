/*
    This file is part of KContactManager.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef QUICKSEARCHWIDGET_H
#define QUICKSEARCHWIDGET_H

#include <QtGui/QWidget>

class KLineEdit;

class QuickSearchWidget : public QWidget
{
  Q_OBJECT

  public:
    QuickSearchWidget( QWidget *parent = 0 );
    virtual ~QuickSearchWidget();

    virtual QSize sizeHint() const;

  Q_SIGNALS:
    void filterStringChanged( const QString &filterString );

  private:
    KLineEdit *mEdit;
};

#endif
