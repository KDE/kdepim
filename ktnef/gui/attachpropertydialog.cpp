/*
    attachpropertydialog.cpp

    Copyright (C) 2002 Michael Goffioul <goffioul@imec.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include "attachpropertydialog.h"
#include <ktnef/ktnefattach.h>
#include <ktnef/ktnefproperty.h>
#include <ktnef/ktnefpropertyset.h>

#include <qlabel.h>
#include <klistview.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <klocale.h>

AttachPropertyDialog::AttachPropertyDialog(QWidget *parent, const char *name)
	: AttachPropertyDialogBase(parent, name, true)
{
}

AttachPropertyDialog::~AttachPropertyDialog()
{
}

void AttachPropertyDialog::setAttachment(KTNEFAttach *attach)
{
	QString	s = (attach->fileName().isEmpty() ? attach->name() : attach->fileName());
	filename_->setText("<b>"+s+"</b>");
	setCaption(s);
	display_->setText(attach->displayName());
	mime_->setText(attach->mimeTag());
	s.setNum(attach->size());
	s.append(" bytes");
	size_->setText(s);
	KMimeType::Ptr	mimetype = KMimeType::mimeType(attach->mimeTag());
	icon_->setPixmap(mimetype->pixmap(KIcon::Small));
	description_->setText(mimetype->comment());
	s.setNum(attach->index());
	index_->setText(s);

	formatPropertySet( attach, properties_ );
}

void formatProperties( const QMap<int,KTNEFProperty*>& props, QListView *lv, QListViewItem *item )
{
	for ( QMap<int,KTNEFProperty*>::ConstIterator it=props.begin(); it!=props.end(); ++it )
	{
		QListViewItem *newItem = 0;
		if ( lv )
			newItem = new QListViewItem( lv, ( *it )->keyString() );
		else if ( item )
			newItem = new QListViewItem( item, ( *it )->keyString() );
		else
		{
			kdWarning() << "formatProperties() called with no listview and no item" << endl;
			return;
		}

		QVariant value = ( *it )->value();
		if ( value.type() == QVariant::List )
		{
			newItem->setOpen( true );
			newItem->setText( 0, newItem->text( 0 ) + " [" + QString::number( value.asList().count() ) + "]" );
			int i = 0;
			for ( QValueList<QVariant>::ConstIterator lit=value.listBegin(); lit!=value.listEnd(); ++lit, i++ )
				new QListViewItem( newItem, "[" + QString::number( i ) + "]", KTNEFProperty::formatValue( *lit ) );
		}
		else if ( value.type() == QVariant::DateTime )
			newItem->setText( 1, value.asDateTime().toString() );
		else
			newItem->setText( 1, ( *it )->valueString() );
	}
}

void formatPropertySet( KTNEFPropertySet *pSet, QListView *lv )
{
	formatProperties( pSet->properties(), lv, 0 );
	QListViewItem *item = new QListViewItem( lv, i18n( "TNEF Attributes" ) );
	item->setOpen( true );
	formatProperties( pSet->attributes(), 0, item );
}
