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
# pragma interface "EmpathIdentitySettingsDialog.h"
#endif

#ifndef EMPATHIDENTITYSETTINGSDIALOG_H
#define EMPATHIDENTITYSETTINGSDIALOG_H

// Qt includes
#include <qwidget.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qdialog.h>

// KDE includes
#include <kbuttonbox.h>

// Local includes
#include "EmpathDefines.h"

class EmpathFileSelectWidget;

class EmpathIdentitySettingsDialog : public QDialog
{
    Q_OBJECT

    public:
        
        EmpathIdentitySettingsDialog(QWidget * parent = 0);
        ~EmpathIdentitySettingsDialog();

        void saveData();
        void loadData();

    protected slots:

        void s_sigChanged(const QString &);
        void s_editSig();

        void s_OK();
        void s_cancel();
        void s_help();
        void s_default();
        void s_apply();
        void s_saveSig();

    private:
    
        QLineEdit    * le_name_;
        QLineEdit    * le_email_;
        QLineEdit    * le_replyTo_;
        QLineEdit    * le_org_;
        EmpathFileSelectWidget * efsw_sig_;
        QMultiLineEdit    * mle_sig_;

        QPushButton * pb_editSig_;

        KButtonBox  * buttonBox_;
        QPushButton * pb_help_;
        QPushButton * pb_default_;
        QPushButton * pb_apply_;
        QPushButton * pb_OK_;
        QPushButton * pb_cancel_;
        
        bool            applied_;
};

#endif
// vim:ts=4:sw=4:tw=78
