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
# pragma interface "EmpathHeaderViewWidget.h"
#endif

#ifndef EMPATH_HEADER_VIEW_WIDGET_H
#define EMPATH_HEADER_VIEW_WIDGET_H

// Qt includes
#include <qwidget.h>
#include <qstrlist.h>
#include <qpixmap.h>

// Local includes
#include <RMM_Envelope.h>

class EmpathHeaderViewWidget : public QWidget
{
    Q_OBJECT
        
    public:
        
        EmpathHeaderViewWidget(QWidget * parent, const char * name);
        virtual ~EmpathHeaderViewWidget();
    
        void useEnvelope(RMM::REnvelope &);
        
    signals:
    
        void clipClicked();
        
    protected:
        
        void paintEvent(QPaintEvent *);
        void resizeEvent(QResizeEvent *);
        void mouseMoveEvent(QMouseEvent *);
        void leaveEvent(QEvent *) { mouseMoveEvent(0); }
        void mousePressEvent(QMouseEvent * e);

    private:
        
        bool            resized_;
        QPixmap            underClip_;
        QPixmap            buf_;
        QStrList        headerList_;
        QPixmap            clipIcon_;
        QPixmap            clipGlow_;
        bool            glowing_;
};

#endif

// vim:ts=4:sw=4:tw=78
