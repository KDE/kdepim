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
# pragma interface "EmpathMessageStructureWidget.h"
#endif

#ifndef EMPATHMESSAGESTRUCTUREWIDGET_H
#define EMPATHMESSAGESTRUCTUREWIDGET_H

// Qt includes
#include <qpixmap.h>
#include <qlistview.h>
#include <qlist.h>
#include <qpopupmenu.h>

// Local includes
#include "EmpathDefines.h"
#include <RMM_Message.h>
#include <RMM_BodyPart.h>

class EmpathMessageStructureWidget : public QListView
{
    Q_OBJECT

    public:
        
        EmpathMessageStructureWidget(
            QWidget * parent = 0, const char * name = 0);

        ~EmpathMessageStructureWidget();
        
        void setMessage(RMM::RBodyPart & m);
        
    protected slots:
        
        void s_currentChanged(QListViewItem *);
        void s_rightButtonPressed(QListViewItem *, const QPoint &, int);
        void s_saveAs();
        void s_openWith();

    signals:
        
        void partChanged(RMM::RBodyPart *);
        
    private:
        
        void _addChildren(RMM::RBodyPart *, QListViewItem *);
        
        QPopupMenu popup_;

};

#endif

// vim:ts=4:sw=4:tw=78
