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
# pragma interface "EmpathConfigPOP3Widget.h"
#endif

#ifndef EMPATHCONFIGPOP3WIDGET_H
#define EMPATHCONFIGPOP3WIDGET_H

// Qt includes
#include <qwidget.h>

// KDE includes
#include <kapp.h>
#include <ktabctl.h>

// Local includes
#include "EmpathDefines.h"

class RikGroupBox;
class EmpathMailboxPOP3;

class EmpathConfigPOP3Server;
class EmpathConfigPOP3Logging;
class EmpathConfigPOP3Password;
class EmpathConfigPOP3General;
class EmpathMailbox;

/**
 * Container for pop3 config widget
 */
class EmpathConfigPOP3Widget : public KTabCtl
{
    Q_OBJECT

    public:
        
        EmpathConfigPOP3Widget(EmpathMailboxPOP3 * mailbox, bool loadData,
            QWidget * parent = 0, const char * name = 0);

        ~EmpathConfigPOP3Widget() { empathDebug("dtor"); }
        
        void setMailbox(EmpathMailboxPOP3 * mailbox, bool loadData = true);

        void    saveData();
        void    loadData();
        
    private:

        EmpathMailbox                * mailbox_;
        EmpathConfigPOP3Server        * serverWidget_;
        EmpathConfigPOP3Logging        * loggingWidget_;
        EmpathConfigPOP3Password    * passwordWidget_;
        EmpathConfigPOP3General        * generalWidget_;
};

#endif
// vim:ts=4:sw=4:tw=78
