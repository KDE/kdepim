/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#ifndef KWIDGETLIST_H
#define KWIDGETLIST_H

#include <QtGui/QScrollArea>

class KWidgetListItem;

class KWidgetList : public QScrollArea
{
  Q_OBJECT

  public:
    KWidgetList( QWidget *parent = 0 );
    virtual ~KWidgetList();

    uint count() const;

    void appendItem( KWidgetListItem *item );
    void removeItem( int index );
    void takeItem( KWidgetListItem *item );

    void setSelected( KWidgetListItem *item );
    void setSelected( int index );

    bool isSelected( KWidgetListItem *item ) const;
    bool isSelected( int index ) const;

    KWidgetListItem *selectedItem() const;
    KWidgetListItem *item( int index ) const;

    int index( KWidgetListItem *item ) const;

    virtual bool eventFilter( QObject *object, QEvent *event );

  public Q_SLOTS:
    void clear();
    virtual void setFocus();

  Q_SIGNALS:
    void selectionChanged( KWidgetListItem *item );
    void doubleClicked( KWidgetListItem *item );

  private:
    class Private;
    Private *d;
};

class KWidgetListItem : public QWidget
{
  public:
    KWidgetListItem( KWidgetList *parent );
    ~KWidgetListItem();

    void setSelected( bool selected );

  protected:
    void setForegroundColor( const QColor &color );
    void setBackgroundColor( const QColor &color );
    void setSelectionForegroundColor( const QColor &color );
    void setSelectionBackgroundColor( const QColor &color );

  private:
    QColor mForegroundColor;
    QColor mBackgroundColor;
    QColor mSelectionForegroundColor;
    QColor mSelectionBackgroundColor;
};

#endif
