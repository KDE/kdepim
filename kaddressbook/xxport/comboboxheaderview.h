/*
   This file is part of KAddressBook.
   Copyright (C) 2007 Mathias Soeken <msoeken@tzi.de>

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
#ifndef COMBOBOXHEADERVIEW_H
#define COMBOBOXHEADERVIEW_H

#include <QHeaderView>

#include <QRect>
#include <QStringList>

class QAbstractItemModel;
class QComboBox;
class QEvent;
class QMouseEvent;
class QResizeEvent;
class QTableWidget;

class ComboBoxHeaderView : public QHeaderView {
  Q_OBJECT

  Q_PROPERTY( QStringList items READ items );
  Q_PROPERTY( int margin READ margin WRITE setMargin );

  public:
    ComboBoxHeaderView( QStringList items,
                        QTableWidget *parent,
                        bool hoverStyle = true );
    virtual ~ComboBoxHeaderView();

    QString headerLabel( int logicalIndex ) const;
    QStringList items() const;
    int margin() const;
    void setMargin( int margin);
    int indexOfHeaderLabel( int logicalIndex ) const;
    QString valueOfHeaderLabel( int logicalIndex ) const;

  private:
    inline QRect sectionRect( int logicalIndex ) const;
    inline void adjustComboBoxIndex( QComboBox *comboBox, int logicalIndex );
    void adjustComboBoxIndex( int logicalIndex );
    bool isViewVisible() const;

  private Q_SLOTS:
    void initialize();
    void slotActivated( const QString &text );
    void setCurrentIndex( int index );
    void slotResetTexts();

  protected:
    virtual void mouseMoveEvent( QMouseEvent *event );
    virtual void leaveEvent( QEvent *event );
    virtual void resizeEvent( QResizeEvent *event );
    virtual void setModel( QAbstractItemModel *model );

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond

    class ComboBox;
};

#endif
