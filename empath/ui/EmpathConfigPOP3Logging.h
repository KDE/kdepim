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
# pragma interface "EmpathConfigPOP3Logging.h"
#endif

#ifndef EMPATHCONFIGPOP3LOGGING_H
#define EMPATHCONFIGPOP3LOGGING_H

// Qt includes
#include <qwidget.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathURL.h"

class EmpathMailboxPOP3;
class EmpathFileSelectWidget;

/**
 * Configure what happens during a pop3 transaction wrt logging.
 * Perhaps we'll be using kio to do this kind of stuff soon.
 */
class EmpathConfigPOP3Logging : public QWidget
{
    Q_OBJECT

    public:
        
        EmpathConfigPOP3Logging(const EmpathURL &, QWidget * = 0);
        
        ~EmpathConfigPOP3Logging();
        
        void loadData();
        void saveData();
        
    protected slots:

        void    s_viewLog();

    private:

        EmpathURL url_;

        QCheckBox   * cb_logConversation_;
        QCheckBox   * cb_appendToLog_;
        QPushButton * pb_viewCurrentLog_;
        QSpinBox    * sb_maxLogFileSize_;

        EmpathFileSelectWidget * efsw_logFile_;
};
#endif
// vim:ts=4:sw=4:tw=78
