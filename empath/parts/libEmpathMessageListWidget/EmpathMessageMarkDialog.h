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
# pragma interface "EmpathMessageMarkDialog.h"
#endif

#ifndef EMPATHMESSAGEMARKDIALOG_H
#define EMPATHMESSAGEMARKDIALOG_H

// Qt includes
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>

// KDE includes
#include <kdialog.h>
#include <kbuttonbox.h>

// Local includes
#include "EmpathIndexRecord.h"

class EmpathMessageMarkDialog : public KDialog
{
    Q_OBJECT

    public:
        
        enum MarkType { On, Off, Toggle };
        
        EmpathMessageMarkDialog(QWidget * parent = 0, const char * name = 0);

        ~EmpathMessageMarkDialog();

        MarkType markType();
        EmpathIndexRecord::Status status();

    protected slots:

        void s_OK();
        void s_Cancel();
        void s_Help();

    private:

        KButtonBox      * buttonBox_;
        
        QPushButton     * pb_OK_;
        QPushButton     * pb_Cancel_;
        QPushButton     * pb_Help_;

        QLabel          * l_tagged_;
        QLabel          * l_replied_;
        QLabel          * l_read_;
        
        QRadioButton    * rb_tagged_;
        QRadioButton    * rb_replied_;
        QRadioButton    * rb_read_;
        
        QRadioButton    * rb_on_;
        QRadioButton    * rb_off_;
        QRadioButton    * rb_toggle_;
        
        QButtonGroup    * typeButtonGroup_;
        QButtonGroup    * stateButtonGroup_;
};

#endif
// vim:ts=4:sw=4:tw=78
