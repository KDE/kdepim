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
# pragma interface "EmpathFolderChooserWidget.h"
#endif

#ifndef EMPATHFOLDERCHOOSERWIDGET_H
#define EMPATHFOLDERCHOOSERWIDGET_H

// Qt includes
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qframe.h>

// Local includes
#include "EmpathURL.h"

class EmpathFolderChooserWidget : public QFrame
{
    Q_OBJECT
    
    public:
    
        EmpathFolderChooserWidget(QWidget * parent, const char * name);
        virtual ~EmpathFolderChooserWidget();

        EmpathURL selectedURL() const;
        
        void setURL(const EmpathURL & url);


    protected slots:
    
        void        s_browse();
    
    private:

        QLabel        * l_folderName_;
        QPushButton    * pb_selectFolder_;
        QGridLayout    * layout_;

        EmpathURL    url_;
};

#endif

// vim:ts=4:sw=4:tw=78
