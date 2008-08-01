/*
    This file is part of KAddressBook.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef CONTACTEDITORWIDGETMANAGER_H
#define CONTACTEDITORWIDGETMANAGER_H

#include <QtGui/QGridLayout>
#include <QtGui/QWidget>

#include "contacteditorwidget.h"

namespace KABC {
class Addressee;
}

class QGridLayout;

class ContactEditorWidgetManager : public QObject
{
  Q_OBJECT

  public:
    static ContactEditorWidgetManager *self();

    /**
      Returns the number of available Contact Editor Page factories.
     */
    int count() const;

    /**
      Returns a factory.
     */
    KAB::ContactEditorWidgetFactory *factory( int pos ) const;

  protected:
    ContactEditorWidgetManager();
    ~ContactEditorWidgetManager();

  private:
    void reload();

    QList<KAB::ContactEditorWidgetFactory*> mFactories;

    static ContactEditorWidgetManager *mSelf;
};

class ContactEditorTabPage : public QWidget
{
  Q_OBJECT

  public:
    ContactEditorTabPage( QWidget *parent );

    /**
      Adds a widget to the tab.
     */
    void addWidget( KAB::ContactEditorWidget *widget );

    /**
      Load the contacts data into the GUI.
     */
    void loadContact( KABC::Addressee *addr );

    /**
      Save the data from the GUI into the passed contact
      object.
     */
    void storeContact( KABC::Addressee *addr );

    /**
      Sets whether the contact should be presented as
      read-only. You should update your GUI in the reimplemented
      method.
     */
    void setReadOnly( bool readOnly );

    /**
      Calculates the layout of the widgets and moves them to the
      correct position.
     */
    void updateLayout();

  Q_SIGNALS:
    /**
      Emitted whenever the page has changed.
     */
    void changed();

  private:
    QGridLayout *mLayout;
    KAB::ContactEditorWidget::List mWidgets;
};

#endif
