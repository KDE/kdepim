/*
    ktnefview.h

    Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#ifndef KTNEFWIDGET_H
#define	KTNEFWIDGET_H

#include <klistview.h>
#include <qptrlist.h>
#include <kdepimmacros.h>

class KTNEFAttach;

class KDE_EXPORT KTNEFView : public KListView
{
	Q_OBJECT

public:
	KTNEFView(QWidget *parent = 0, const char *name = 0);
	~KTNEFView();

	void setAttachments(QPtrList<KTNEFAttach> *list);
	QPtrList<KTNEFAttach>* getSelection();

signals:
	void dragRequested( const QValueList<KTNEFAttach*>& list );

protected:
	void resizeEvent(QResizeEvent *e);
	void startDrag();

private:
	QPtrList<KTNEFAttach>	attachments_;
};

#endif
