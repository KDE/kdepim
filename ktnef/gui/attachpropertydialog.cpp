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
#include <ktnef/ktnefdefs.h>
#include "qwmf.h"

#include <qlabel.h>
#include <klistview.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <qbuffer.h>
#include <qdatastream.h>
#include <qpicture.h>

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
	QPixmap pix = loadRenderingPixmap( attach, colorGroup().background() );
	if ( !pix.isNull() )
		icon_->setPixmap( pix );
	else
		icon_->setPixmap(mimetype->pixmap(KIcon::Small));
	description_->setText(mimetype->comment());
	s.setNum(attach->index());
	index_->setText(s);

	formatPropertySet( attach, properties_ );
	m_attach = attach;
}

void AttachPropertyDialog::saveClicked()
{
	saveProperty( properties_, m_attach, this );
}

void formatProperties( const QMap<int,KTNEFProperty*>& props, QListView *lv, QListViewItem *item, const QString& prefix )
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
		{
			newItem->setText( 1, ( *it )->valueString() );
			newItem->setText( 2, prefix + "_" + QString::number( it.key() ) );
		}
	}
}

void formatPropertySet( KTNEFPropertySet *pSet, QListView *lv )
{
	formatProperties( pSet->properties(), lv, 0, "prop" );
	QListViewItem *item = new QListViewItem( lv, i18n( "TNEF Attributes" ) );
	item->setOpen( true );
	formatProperties( pSet->attributes(), 0, item, "attr" );
}

void saveProperty( QListView *lv, KTNEFPropertySet *pSet, QWidget *parent )
{
	QListViewItem *item = lv->selectedItem();
	if ( !item )
		KMessageBox::error( parent, i18n( "Select an item." ) );
	else if ( item->text( 2 ).isEmpty() )
		KMessageBox::error( parent, i18n( "The selected item cannot be saved." ) );
	else
	{
		QString tag = item->text( 2 );
		int key = tag.mid( 5 ).toInt();
		QVariant prop = ( tag.startsWith( "attr_" ) ? pSet->attribute( key ) : pSet->property( key ) );
		QString filename = KFileDialog::getSaveFileName( tag, QString::null, parent );
		if ( !filename.isEmpty() )
		{
			QFile f( filename );
			if ( f.open( IO_WriteOnly ) )
			{
				switch ( prop.type() )
				{
					case QVariant::ByteArray:
						f.writeBlock( prop.asByteArray().data(), prop.asByteArray().size() );
						break;
					default:
						{
							QTextStream t( &f );
							t << prop.toString();
							break;
						}
				}
				f.close();
			}
			else
				KMessageBox::error( parent, i18n( "Unable to open file for writing, check file permissions." ) );
		}
	}
}

QPixmap loadRenderingPixmap( KTNEFPropertySet *pSet, const QColor& bgColor )
{
	QPixmap pix;
	QVariant rendData = pSet->attribute( attATTACHRENDDATA ), wmf = pSet->attribute( attATTACHMETAFILE );
	if ( !rendData.isNull() && !wmf.isNull() )
	{
		// Get rendering size
		QBuffer rendBuffer( rendData.asByteArray() );
		rendBuffer.open( IO_ReadOnly );
		QDataStream rendStream( &rendBuffer );
		rendStream.setByteOrder( QDataStream::LittleEndian );
		Q_UINT16 type, w, h;
		rendStream >> type >> w >> w; // read type and skip 4 bytes
		rendStream >> w >> h;
		rendBuffer.close();

		if ( type == 1 && w > 0 && h > 0 )
		{
			// Load WMF data
			QWinMetaFile wmfLoader;
			QBuffer wmfBuffer( wmf.asByteArray() );
			wmfBuffer.open( IO_ReadOnly );
			wmfLoader.setBbox( QRect( 0, 0, w, h ) );
			if ( wmfLoader.load( wmfBuffer ) )
			{
				pix.resize( w, h );
				pix.fill( bgColor );
				wmfLoader.paint( &pix );
			}
			wmfBuffer.close();
		}
	}
	return pix;
}
