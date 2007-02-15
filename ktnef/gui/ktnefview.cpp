/*
    ktnefview.cpp

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

#include "ktnefview.h"
#include <ktnef/ktnefattach.h>
#include "attachpropertydialog.h"

#include <qheader.h>
#include <qpixmap.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include <kmimetype.h>

class Attachment : public QListViewItem
{
public:
	Attachment(QListView *parent, KTNEFAttach *attach);
	~Attachment();

	KTNEFAttach* getAttachment() const { return attach_; }

private:
	KTNEFAttach	*attach_;
};

Attachment::Attachment(QListView *parent, KTNEFAttach *attach)
	: QListViewItem(parent, attach->name()), attach_(attach)
{
	setText(2, QString::number( attach_->size() ));
	if (!attach_->fileName().isEmpty()) setText(0, attach_->fileName());
	KMimeType::Ptr	mimeType = KMimeType::mimeType( attach_->mimeTag() );
	setText(1, mimeType->comment());
	QPixmap pix = loadRenderingPixmap( attach, parent->colorGroup().base() );
	if ( !pix.isNull() )
		setPixmap( 0, pix );
	else
		setPixmap(0, mimeType->pixmap(KIcon::Small));
	setDragEnabled( true );
}

Attachment::~Attachment()
{
}

//------------------------------------------------------------------------------------------------------

KTNEFView::KTNEFView(QWidget *parent, const char *name)
	: KListView(parent,name)
{
	attachments_.setAutoDelete(false);
	addColumn(i18n("File Name"));
	addColumn(i18n("File Type"));
	addColumn(i18n("Size"));
	setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	setLineWidth(1);
	setSelectionMode(QListView::Extended);
	setHScrollBarMode(QScrollView::AlwaysOff);
	setVScrollBarMode(QScrollView::AlwaysOn);
	QTimer::singleShot( 0, this, SLOT(adjustColumnWidth()) );
}

KTNEFView::~KTNEFView()
{
}

void KTNEFView::setAttachments(QPtrList<KTNEFAttach> *list)
{
	clear();
	if (list)
	{
		QPtrListIterator<KTNEFAttach>	it(*list);
		for (;it.current();++it)
			new Attachment(this, it.current());
	}
}

void KTNEFView::resizeEvent(QResizeEvent *e)
{
	adjustColumnWidth();
	resizeContents(visibleWidth(),visibleHeight());
	if (e) QListView::resizeEvent(e);
}

QPtrList<KTNEFAttach>* KTNEFView::getSelection()
{
	attachments_.clear();
	QListViewItem	*item = firstChild();
	while (item)
	{
		if (item->isSelected()) attachments_.append(((Attachment*)item)->getAttachment());
		item = item->nextSibling();
	}
	return &attachments_;
}

void KTNEFView::startDrag()
{
	QListViewItemIterator it( this, QListViewItemIterator::Selected );
	QValueList<KTNEFAttach*> list;
	while ( it.current() )
	{
		list << static_cast<Attachment*>( it.current() )->getAttachment();
		++it;
	}
	if ( !list.isEmpty() )
		emit dragRequested( list );
}

void KTNEFView::adjustColumnWidth()
{
	int w = visibleWidth()/2;
	setColumnWidth(0,w);
	setColumnWidth(1,w/2);
	setColumnWidth(2,w/2);
}

#include "ktnefview.moc"
