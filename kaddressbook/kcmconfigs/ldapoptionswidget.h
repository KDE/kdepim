/*
    This file is part of KAddressBook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#ifndef LDAPOPTIONSWIDGET_H
#define LDAPOPTIONSWIDGET_H

#include <QtGui/QWidget>

class QToolButton;
class K3ListView;
class Q3ListViewItem;
class QPushButton;

class LDAPOptionsWidget : public QWidget
{
  Q_OBJECT

  public:
    LDAPOptionsWidget( QWidget* parent = 0, const char* name = 0 );
    ~LDAPOptionsWidget();

    void restoreSettings();
    void saveSettings();
    void defaults();

  Q_SIGNALS:
    void changed( bool );

  private Q_SLOTS:
    void slotAddHost();
    void slotEditHost();
    void slotRemoveHost();
    void slotSelectionChanged( Q3ListViewItem* );
    void slotItemClicked( Q3ListViewItem* );
    void slotMoveUp();
    void slotMoveDown();

  private:
    void initGUI();

    K3ListView* mHostListView;

    QPushButton* mAddButton;
    QPushButton* mEditButton;
    QPushButton* mRemoveButton;

    QToolButton* mUpButton;
    QToolButton* mDownButton;
};

#endif
