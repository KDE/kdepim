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
# pragma interface "EmpathLeftSideWidget.h"
#endif

#ifndef EMPATH_LEFT_SIDE_WIDGET_H
#define EMPATH_LEFT_SIDE_WIDGET_H

// Qt includes
#include <qwidget.h>
#include <qlayout.h>

class EmpathFolderWidget;
class EmpathTaskWidget;
class EmpathMessageListWidget;

class EmpathLeftSideWidget : public QWidget
{
    Q_OBJECT
        
    public:
        
        EmpathLeftSideWidget(
            EmpathMessageListWidget *, QWidget * parent, const char * name);
        virtual ~EmpathLeftSideWidget();
        
    private:
        
        EmpathFolderWidget    * folderWidget_;
        EmpathTaskWidget    * taskWidget_;
        QGridLayout            * layout_;
};

#endif
// vim:ts=4:sw=4:tw=78
