/* messageDialog.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is, um, a message dialog. It is essentially superceded
** by KMessage and other classes, but it still hangs around until
** we finally do some serious cruft-removal on KPilot.
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


static const char *messagedialog_id="$Id$";

#include <qdialog.h>
#include <kapp.h>
#include "messageDialog.moc"

MessageDialog::MessageDialog( QString title, QWidget* parent, const char* name, bool modal)
  : QDialog(parent, name, modal, 0)
    {
    setGeometry(x(), y(), 250, 40);
    setCaption(title);
    fMessage = new QLabel(title, this);
    fMessage->setFixedWidth(220);
    fMessage->move(10, 10);
    fMessage->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    fMessage->setAlignment(AlignBottom | AlignHCenter);
    kapp->processEvents();
#ifdef DEBUG
	/* NOTREACHED */
	(void) messagedialog_id;
#endif
    }
  
void 
MessageDialog::setMessage(QString message)
    {
    fMessage->setText(message);
    fMessage->adjustSize();
    kapp->processEvents();
    }

// $Log$
// Revision 1.8  2001/02/05 20:58:48  adridg
// Fixed copyright headers for source releases. No code changed
//
