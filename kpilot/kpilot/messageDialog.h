/* messageDialog.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is a rather lame replacement for KMessageBox, and it
** has been around since KDE beta 4 (long before 1.0!). It should
** be removed sometime after 2.1, since I don't think we're using
** it anymore.
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/


#ifndef __MESSAGE_DIALOG
#define __MESSAGE_DIALOG

#include <qdialog.h>
#include <qlabel.h>

class MessageDialog : public QDialog
    {
    Q_OBJECT

    public:
    MessageDialog( QString title, QWidget* parent=0, const char* name=0, bool modal=false);
    void setMessage(QString  message);
    
    protected:
    QLabel* fMessage;
    };
#else
#warning "File doubly included"
#endif

// $Log$
// Revision 1.5  2001/02/06 08:05:19  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
