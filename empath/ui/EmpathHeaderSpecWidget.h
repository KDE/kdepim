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
# pragma interface "EmpathHeaderSpecWidget.h"
#endif

#ifndef EMPATHHEADERSPECWIDGET_H
#define EMPATHHEADERSPECWIDGET_H

// Qt includes
#include <qhbox.h>
#include <qstring.h>
#include <qlabel.h>

// Local includes
#include "EmpathDefines.h"
#include <RMM_Header.h>

class QLabel;

class EmpathHeaderBodyWidget;

class EmpathHeaderSpecWidget : public QHBox
{
    Q_OBJECT

    public:
        
        EmpathHeaderSpecWidget(RMM::RHeader header, QWidget * parent = 0);

        ~EmpathHeaderSpecWidget();

        int         sizeOfColumnOne();
        void        setColumnOneSize(int);

        RMM::RHeader header();

        QString     headerName();
        QString     headerBody();

    signals:

        void goUp();
        void goDown();
    
    protected:
       
        void focusInEvent   (QFocusEvent *);
        void keyPressEvent  (QKeyEvent *);

    protected slots:

        void s_setFocus();   
        
    private:

        QLabel                  * headerNameWidget_;
        EmpathHeaderBodyWidget  * headerBodyWidget_;
       
        RMM::RHeader header_;
};

#endif
// vim:ts=4:sw=4:tw=78
