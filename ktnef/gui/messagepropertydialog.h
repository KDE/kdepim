/*
    messagepropertydialog.h

    Copyright (C) 2003 Michael Goffioul <goffioul@imec.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#ifndef MESSAGEPROPERTYDIALOG_H
#define MESSAGEPROPERTYDIALOG_H

#include <kdialogbase.h>

class KListView;
class KTNEFMessage;

class MessagePropertyDialog : public KDialogBase
{
	Q_OBJECT
public:
	MessagePropertyDialog( QWidget *parent, KTNEFMessage *msg );

protected slots:
	void slotUser1();

private:
	KTNEFMessage *m_message;
	KListView    *m_listview;
};

#endif /* MESSAGEPROPERTYDIALOG_H */
