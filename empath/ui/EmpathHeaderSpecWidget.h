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

class QLineEdit;
class QLabel;

class EmpathHeaderSpecWidget : public QHBox
{
    Q_OBJECT

    public:
        
        EmpathHeaderSpecWidget(
            const QString & headerName = QString::null,
            const QString & headerBody = QString::null,
            QWidget * parent = 0, const char * name = 0);

        ~EmpathHeaderSpecWidget();

        int         sizeOfColumnOne();
        void        setColumnOneSize(int);

        QString     header();
        QString     headerName();
        QString     headerBody();
        void        setHeaderName(const QString & headerName);
        void        setHeaderBody(const QString & headerBody);

    signals:

        void lineUp();
        void lineDown();
    
    protected:
       
        void focusInEvent   (QFocusEvent *);
        void keyPressEvent  (QKeyEvent *);
        
    private:

        QLabel      * headerNameWidget_;
        QLineEdit   * headerBodyWidget_;
        
        QString headerName_;
        QString headerBody_;
        
        bool address_;
};

#endif
// vim:ts=4:sw=4:tw=78
