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
# pragma interface "EmpathConfigPOP3Server.h"
#endif

#ifndef EMPATHCONFIGPOP3SERVER_H
#define EMPATHCONFIGPOP3SERVER_H

// Qt includes
#include <qwidget.h>
#include <qlineedit.h>
#include <qspinbox.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathURL.h"

class RikGroupBox;
class EmpathMailboxPOP3;
class EmpathPasswordEditWidget;

/**
 * Configure name, port, password etc for pop3 server
 */
class EmpathConfigPOP3Server : public QWidget
{
    Q_OBJECT

    public:
        
        EmpathConfigPOP3Server(const EmpathURL &, QWidget * = 0);

        ~EmpathConfigPOP3Server();
        
        void loadData();
        void saveData();
        
    private:

        EmpathURL url_;

        QLineEdit   * le_uname_;
        QLineEdit   * le_inServer_;
        QSpinBox    * sb_inServerPort_;
        
        EmpathPasswordEditWidget * epew_pass_;
};

#endif
// vim:ts=4:sw=4:tw=78
