/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Tobias Koenig <tobias.koenig@kdab.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include "ui_searchwidget.h"

#include <QGraphicsProxyWidget>
#include <QWidget>

class SearchWidget : public QWidget
{
  Q_OBJECT

  public:
    explicit SearchWidget( QWidget *parent = 0 );

    QString query() const;

  private:
    Ui_SearchWidget mUi;
};

class DeclarativeSearchWidget : public QGraphicsProxyWidget
{
  Q_OBJECT

  Q_PROPERTY( QString query READ query )

  public:
    explicit DeclarativeSearchWidget( QGraphicsItem *parent = 0 );
    ~DeclarativeSearchWidget();

  public Q_SLOTS:
    QString query() const;

  private:
    SearchWidget *mSearchWidget;
};

#endif
