/* passworddialog.h			KPilot
**
** Copyright (c) 1997 by Michael Roth
** 	Taken from KPasswd
**
** This file is part of the popmail conduit, a conduit for KPilot that
** synchronises the Pilot's email application with the outside world,
** which currently means:
**	-- sendmail or SMTP for outgoing mail
**	-- POP or mbox for incoming mail
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#ifndef MRQPASSWD_H
#define MRQPASSWD_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qdialog.h>
#include <qlineedit.h>

class PasswordDialog : public QDialog
{
    Q_OBJECT
   
public:
    PasswordDialog( QString head, QWidget* parent=0, const char* name=0, bool modal=false, WFlags f=0 );
    
    const char *password();			// Gibt das Paswort zurueck
    
private:
    QString _head;
    QLineEdit *_w_password;      
};

#endif
