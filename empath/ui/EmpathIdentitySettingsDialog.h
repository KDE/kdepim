/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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
#include <qlabel.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qfont.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qdialog.h>

// KDE includes
#include <kbuttonbox.h>

// Local includes
#include "EmpathDefines.h"

class RikGroupBox;

class EmpathIdentitySettingsDialog : public QDialog
{
    Q_OBJECT

    public:
        
        static void create();
        ~EmpathIdentitySettingsDialog();

        void saveData();
        void loadData();
    
        void closeEvent(QCloseEvent * e) { e->accept(); delete this; }

    protected slots:

        void s_chooseSig();
        void s_editSig();

        void s_OK();
        void s_cancel();
        void s_help();
        void s_default();
        void s_apply();
        void s_saveSig();

    private:
        
        EmpathIdentitySettingsDialog(
            QWidget * parent = 0, const char * name = 0);
    
        QGridLayout    * topLevelLayout_;
        QGridLayout    * mainGroupLayout_;
        QGridLayout    * sigPreviewGroupLayout_;
    
        RikGroupBox    * rgb_main_;
        QWidget        * w_main_;
        
        RikGroupBox    * rgb_sigPreview_;
        QWidget        * w_sigPreview_;

        QLabel        * l_name_;
        QLabel        * l_email_;
        QLabel        * l_replyTo_;
        QLabel        * l_org_;
        QLabel        * l_sig_;
        

        QLineEdit    * le_chooseName_;
        QLineEdit    * le_chooseEmail_;
        QLineEdit    * le_chooseReplyTo_;
        QLineEdit    * le_chooseOrg_;
        QLineEdit    * le_chooseSig_;
        QMultiLineEdit    * mle_sigPreview_;

        QPushButton    * pb_chooseSig_;
        QPushButton    * pb_editSig_;

        QString            name_;
        QString            email_;
        QString            replyTo_;
        QString            org_;
        QString         sig_;
        KButtonBox        * buttonBox_;
        QPushButton        * pb_help_;
        QPushButton        * pb_default_;
        QPushButton        * pb_apply_;
        QPushButton        * pb_OK_;
        QPushButton        * pb_cancel_;
        
        static bool        exists_;
        bool            applied_;
};

#endif
// vim:ts=4:sw=4:tw=78
