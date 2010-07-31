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
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef KTNEFWIDGET_H
#define	KTNEFWIDGET_H

#include <klistview.h>
#include <tqptrlist.h>
#include <kdepimmacros.h>

class KTNEFAttach;

class KDE_EXPORT KTNEFView : public KListView
{
	Q_OBJECT

public:
	KTNEFView(TQWidget *parent = 0, const char *name = 0);
	~KTNEFView();

	void setAttachments(TQPtrList<KTNEFAttach> *list);
	TQPtrList<KTNEFAttach>* getSelection();

signals:
	void dragRequested( const TQValueList<KTNEFAttach*>& list );

protected:
	void resizeEvent(TQResizeEvent *e);
	void startDrag();

private slots:
	void adjustColumnWidth();

private:
	TQPtrList<KTNEFAttach>	attachments_;
};

#endif
