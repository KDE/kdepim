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
# pragma interface "EmpathTaskWidget.h"
#endif

#ifndef EMPATH_TASK_WIDGET_H
#define EMPATH_TASK_WIDGET_H

// Qt includes
#include <qwidget.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qprogressbar.h>

// Local includes
#include "EmpathTask.h" 

class EmpathTaskItem : public QWidget
{
    Q_OBJECT
        
    public:
        
        EmpathTaskItem(
            const QString & title,
            QWidget * parent, const char * name);
        
        virtual ~EmpathTaskItem();
        
        QSize minimumSizeHint() const;
        
    public slots:
        
        void s_done();
        void s_inc();
        void s_setMax(int);
        void s_setPos(int);
        
    signals:
        
        void done(EmpathTaskItem *);
        
    private:
        
        QString title_;
        int pos_;
        int max_;
        
        QProgressBar* progressMeter_;
        QLabel        * label_;
        QGridLayout * layout_;
};


class EmpathTaskWidget : public QWidget
{
    Q_OBJECT
        
    public:
        
        EmpathTaskWidget(QWidget * parent = 0, const char * name = 0);
        virtual    ~EmpathTaskWidget();
        
        void    resizeEvent(QResizeEvent *);
        
    protected slots:
            
        void s_done(EmpathTaskItem *);
        void s_addTask(EmpathTask *);

    private:
        
        QList<EmpathTaskItem> itemList_;
        int    itemHeight_;
        QLabel * l;
};

#endif

// vim:ts=4:sw=4:tw=78
