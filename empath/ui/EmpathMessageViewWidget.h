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
#pragma interface "EmpathMessageViewWidget.h"
#endif

#ifndef EMPATHMESSAGEVIEWWIDGET_H
#define EMPATHMESSAGEVIEWWIDGET_H

// Qt includes
#include <qpopupmenu.h>
#include <qlayout.h>

// KDE includes
#include <ktmainwindow.h>
#include <kapp.h>
#include <kstdaccel.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathJobInfo.h"
#include "EmpathURL.h"

class EmpathMessageStructureWidget;
class EmpathMessageHTMLWidget;
class EmpathHeaderViewWidget;

namespace RMM { class RBodyPart; }

class EmpathMessageViewWidget : public QWidget
{
    Q_OBJECT

    public:
        
        EmpathMessageViewWidget(const EmpathURL & url, QWidget * parent = 0);
        
        ~EmpathMessageViewWidget();
        
    public slots:

        void s_print();
        void s_setMessage(const EmpathURL &);
        void s_partChanged(RMM::RBodyPart *);
        void s_switchView();

    protected slots:
        
        void s_URLSelected(QString, int);
        void s_clipClicked();
        void s_jobComplete(EmpathJobInfo);

    private:
        
        void showText(QCString &, bool m = true);

        EmpathMessageStructureWidget * structureWidget_;
        EmpathMessageHTMLWidget      * messageWidget_;
        EmpathHeaderViewWidget       * headerViewWidget_;

        EmpathURL   url_;
        bool        viewingSource_;

        static unsigned int id_;
};

#endif

// vim:ts=4:sw=4:tw=78
