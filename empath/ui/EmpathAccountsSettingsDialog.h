/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
#pragma interface "EmpathAccountsSettingsDialog.h"
#endif

#ifndef EMPATHACCOUNTSSETTINGSDIALOG_H
#define EMPATHACCOUNTSSETTINGSDIALOG_H

// Qt includes
#include <qwidget.h>
#include <qlistview.h>

// KDE includes
#include <kdialog.h>

// Local includes
#include "EmpathURL.h"
#include "EmpathDefines.h"

class EmpathMailbox;

/**
 * Dialog used to configure settings for each account.
 * User can add, remove and edit accounts and see some vital info.
 */

class EmpathAccountsSettingsDialog : public KDialog
{
    Q_OBJECT

    public:
        
        EmpathAccountsSettingsDialog(QWidget * = 0);
        ~EmpathAccountsSettingsDialog();

        void loadData();

    protected slots:

        void s_newPOP();
        void s_newIMAP();
        void s_edit();
        void s_remove();
        
        void s_OK();
        void s_cancel();
        void s_help();
        void s_apply();
        
        void s_updateMailboxList();
        void s_currentChanged(QListViewItem *);

    private:
  
        QListView * lv_accts_;

        bool applied_;

        QPushButton * pb_remove_;
};

class EmpathAccountListItem : public QListViewItem
{
    public:
        EmpathAccountListItem(QListView *, EmpathMailbox *);
};

#endif
// vim:ts=4:sw=4:tw=78
