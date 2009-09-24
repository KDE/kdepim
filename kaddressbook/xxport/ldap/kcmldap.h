/*
    This file is part of KAddressBook.

    Copyright (c) 2003 - 2009 Tobias Koenig <tokoe@kde.org>

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
*/

#ifndef KCMLDAP_H
#define KCMLDAP_H

#include <kcmodule.h>

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QToolButton;

class KCMLdap : public KCModule
{
  Q_OBJECT

  public:
    explicit KCMLdap( QWidget* parent, const QVariantList &args );
    ~KCMLdap();

    virtual void load();
    virtual void save();
    virtual void defaults();

  Q_SIGNALS:
    void changed( bool );

  private Q_SLOTS:
    void slotAddHost();
    void slotEditHost();
    void slotRemoveHost();
    void slotSelectionChanged( QListWidgetItem* );
    void slotItemClicked( QListWidgetItem* );
    void slotMoveUp();
    void slotMoveDown();

  private:
    void initGUI();

    QListWidget* mHostListView;

    QPushButton* mAddButton;
    QPushButton* mEditButton;
    QPushButton* mRemoveButton;

    QToolButton* mUpButton;
    QToolButton* mDownButton;
};

#endif
