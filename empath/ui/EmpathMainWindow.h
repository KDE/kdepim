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

#ifndef EMPATH_MAIN_WINDOW_H
#define EMPATH_MAIN_WINDOW_H

// KDE includes
#include <kparts/mainwindow.h>
#include <kparts/part.h>

class QPopupMenu;
class QWidgetStack;
class KProgress;
class EmpathTask;
class EmpathMainWidget;
enum BarPosition {};

class EmpathMainWindow : public KParts::MainWindow
{
    Q_OBJECT

    public:
        
        EmpathMainWindow();
        virtual ~EmpathMainWindow();
    
    protected slots:

        void s_toolbarMoved(BarPosition);
        void s_newTask(EmpathTask *);
        void s_settings();

    private:
    
        void _initActions();

        KParts::ReadWritePart * browser_;

        QWidgetStack * progressStack_;
};

class EmpathProgressIndicator : public QWidget
{
    Q_OBJECT

    public:

        EmpathProgressIndicator(EmpathTask *, QWidgetStack * parent);
        virtual ~EmpathProgressIndicator();

    protected slots:

        void s_setMaxValue(int);
        void s_incValue();
        void s_finished();

    private:
        
        KProgress * progress_;
};


#endif
// vim:ts=4:sw=4:tw=78
