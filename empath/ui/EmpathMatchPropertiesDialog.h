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
# pragma interface "EmpathMatchPropertiesDialog.h"
#endif

#ifndef EMPATHMATCHPROPERTIESDIALOG_H
#define EMPATHMATCHPROPERTIESDIALOG_H

// Qt includes
#include <qdialog.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qspinbox.h>

class EmpathMatcher;

class EmpathMatchPropertiesDialog : public QDialog
{
    Q_OBJECT

    public:

        EmpathMatchPropertiesDialog(QWidget * parent, EmpathMatcher * matcher);

        ~EmpathMatchPropertiesDialog();
        EmpathMatcher * matcher();

    protected slots:

        void s_OK();
        void s_cancel();
        void s_help();

    private:
        
        EmpathMatcher * matcher_;

        QButtonGroup * bg_choices_;

        QLineEdit * le_exprBody_;
        QLineEdit * le_exprHeader_;

        QComboBox * cb_header_;

        QSpinBox * sb_size_;
};

#endif

// vim:ts=4:sw=4:tw=78
