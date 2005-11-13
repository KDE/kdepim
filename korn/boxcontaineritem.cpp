/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "boxcontaineritem.h"

#include "mailsubject.h"

#include <kaboutapplication.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kbugreport.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpassivepopup.h>
#include <kpopupmenu.h>
#include <kprocess.h>
#include <kshortcut.h>

#include <qbitmap.h>
#include <qcolor.h>
#include <qfont.h>
#include <qgrid.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qtooltip.h>
#include <qvbox.h>

BoxContainerItem::BoxContainerItem( QObject * parent, const char * name )
	: AccountManager( parent, name ),
	DCOPObject(),
	_command( new QString )
{
	short i;
	
	for( i = 0; i < 2; ++i )
	{
		_icons[ i ] = 0;
		_anims[ i ] = 0;
		_fgColour[ i ] = 0;
		_bgColour[ i ] = 0;
		_fonts[ i ] = 0;
	}
	
	for( i = 0; i < 3; ++i )
	{
		_recheckSettings[ i ] = false;
		_resetSettings[ i ] = false;
		_viewSettings[ i ] = false;
		_runSettings[ i ] = false;
		_popupSettings[ i ] = false;
	}
}

BoxContainerItem::~BoxContainerItem()
{
	delete _command;
}
	
void BoxContainerItem::readConfig( KConfig* config, const int index )
{
	//Read information about how the thing have to look like
	config->setGroup( QString( "korn-%1" ).arg( index ) );
	if( config->readBoolEntry( "hasnormalicon", false ) )
		_icons[ 0 ] = new QString( config->readEntry( "normalicon", "" ) );
	else
		_icons[ 0 ] = 0;
	if( config->readBoolEntry( "hasnewicon", false ) )
		_icons[ 1 ] = new QString( config->readEntry( "newicon", "" ) );
	else
		_icons[ 1 ] = 0;
	
	if( config->readBoolEntry( "hasnormalanim", false ) )
		_anims[ 0 ] = new QString( config->readEntry( "normalanim", "" ) );
	else
		_anims[ 0 ] = 0;
	if( config->readBoolEntry( "hasnewanim", false ) )
		_anims[ 1 ] = new QString( config->readEntry( "newanim", "" ) );
	else
		_anims[ 1 ] = 0;
	
	if( config->readBoolEntry( "hasnormalfgcolour", false ) )
		_fgColour[ 0 ] = new QColor( config->readColorEntry( "normalfgcolour" ) );
	else
		_fgColour[ 0 ] = 0;
	if( config->readBoolEntry( "hasnewfgcolour", false ) )
		_fgColour[ 1 ] = new QColor( config->readColorEntry( "newfgcolour" ) );
	else
		_fgColour[ 1 ] = 0;
	
	if( config->readBoolEntry( "hasnormalbgcolour", false ) )
		_bgColour[ 0 ] = new QColor( config->readColorEntry( "normalbgcolour" ) );
	else
		_bgColour[ 0 ] = 0;
	if( config->readBoolEntry( "hasnewbgcolour", false ) )
		_bgColour[ 1 ] = new QColor( config->readColorEntry( "newbgcolour" ) );
	else
		_bgColour[ 1 ] = 0;
	if( config->readBoolEntry( "hasnormalfont", false ) )
		_fonts[ 0 ] = new QFont( config->readFontEntry( "normalfont" ) );
	else
		_fonts[ 0 ] = 0;
	if( config->readBoolEntry( "hasnewfont", false ) )
		_fonts[ 1 ] = new QFont( config->readFontEntry( "newfont" ) );
	else
		_fonts[ 1 ] = 0;
	
	//Read information about the mappings.
	_recheckSettings[ 0 ] = config->readBoolEntry( "leftrecheck", true );
	_recheckSettings[ 1 ] = config->readBoolEntry( "middlerecheck", false );
	_recheckSettings[ 2 ] = config->readBoolEntry( "rightrecheck", false );
	
	_resetSettings[ 0 ] = config->readBoolEntry( "leftreset", false );
	_resetSettings[ 1 ] = config->readBoolEntry( "middlereset", false );
	_resetSettings[ 2 ] = config->readBoolEntry( "rightreset", false );
	
	_viewSettings[ 0 ] = config->readBoolEntry( "leftview", false );
	_viewSettings[ 1 ] = config->readBoolEntry( "middleview", false );
	_viewSettings[ 2 ] = config->readBoolEntry( "rightview", false );
	
	_runSettings[ 0 ] = config->readBoolEntry( "leftrun", false );
	_runSettings[ 1 ] = config->readBoolEntry( "middlerun", false );
	_runSettings[ 2 ] = config->readBoolEntry( "rightrun", false );
	
	_popupSettings[ 0 ] = config->readBoolEntry( "leftpopup", false );
	_popupSettings[ 1 ] = config->readBoolEntry( "middlepopup", false );
	_popupSettings[ 2 ] = config->readBoolEntry( "rightpopup", true );
	
	//Read the command
	*_command = config->readEntry( "command", "" );
	
	//Sets the object ID for the DCOP-object
	this->setObjId( config->readEntry( "name", "" ).utf8() );
	
	//Read the settings of the reimplemented class.
	//It is important to read this after the box-settings, because the
	//setCount-function is called in AccountManager::readConfig
	AccountManager::readConfig( config, index );
}

void BoxContainerItem::runCommand( const QString& cmd )
{
	KProcess *process = new KProcess;
	process->setUseShell( true );
	*process << cmd;
	connect( process, SIGNAL( processExited (KProcess *) ), this, SLOT( processExited( KProcess * ) ) );
	process->start();
}

void BoxContainerItem::mouseButtonPressed( Qt::ButtonState state )
{
	int button;
	if( state & Qt::LeftButton )
		button = 0;
	else if( state & Qt::RightButton )
		button = 2;
	else if( state & Qt::MidButton )
		button = 1;
	else
		return; //Invalid mouse button

	if( _recheckSettings[ button ] )
		doRecheck();
	if( _resetSettings[ button ] )
		doReset();
	if( _viewSettings[ button ] )
		doView();
	if( _runSettings[ button ] )
		runCommand();
	if( _popupSettings[ button ] )
		doPopup();
}

void BoxContainerItem::fillKPopupMenu( KPopupMenu* popupMenu, KActionCollection* actions ) const
{
	/*popupMenu->insertItem( i18n( "&Configure" ), this, SLOT( slotConfigure() ) );
	popupMenu->insertItem( i18n( "&Recheck" ), this, SLOT( slotRecheck() ) );
	popupMenu->insertItem( i18n( "R&eset Counter" ), this, SLOT( slotReset() ) );
	popupMenu->insertItem( i18n( "&View Emails" ), this, SLOT( slotView() ) );
	popupMenu->insertItem( i18n( "R&un Command" ), this, SLOT( slotRunCommand() ) );*/
	
	(new KAction( i18n("&Configure"),     KShortcut(), this, SLOT( slotConfigure()  ), actions ))->plug( popupMenu );
	(new KAction( i18n("&Recheck"),       KShortcut(), this, SLOT( slotRecheck()    ), actions ))->plug( popupMenu );
	(new KAction( i18n("R&eset Counter"), KShortcut(), this, SLOT( slotReset()      ), actions ))->plug( popupMenu );
	(new KAction( i18n("&View Emails"),   KShortcut(), this, SLOT( slotView()       ), actions ))->plug( popupMenu );
	(new KAction( i18n("R&un Command"),   KShortcut(), this, SLOT( slotRunCommand() ), actions ))->plug( popupMenu );
	popupMenu->insertSeparator();
	KStdAction::help(      this, SLOT( help()      ), actions )->plug( popupMenu );
	KStdAction::reportBug( this, SLOT( reportBug() ), actions )->plug( popupMenu );
	KStdAction::aboutApp(  this, SLOT( about()     ), actions )->plug( popupMenu );
}

void BoxContainerItem::showPassivePopup( QWidget* parent, QPtrList< KornMailSubject >* list, int total,
					 const QString &accountName, bool date )
{
	KPassivePopup *popup = new KPassivePopup( parent, "Passive popup" );
		
	QVBox *mainvlayout = popup->standardView( i18n( "KOrn - %1/%2 (total: %3)" ).arg( objId() ).arg( accountName )
			.arg( total ), "", QPixmap(), 0 );
	QGrid *mainglayout = new QGrid( date ? 3 : 2 ,mainvlayout, "Grid-Layout" );
	
	QLabel *title = new QLabel( i18n("From"), mainglayout, "from_label" );
	QFont font = title->font();
	font.setBold( true );
	title->setFont( font );
		
	title = new QLabel( i18n("Subject"), mainglayout, "subject_label" );
	font = title->font();
	font.setBold( true );
	title->setFont( font );
		
	if( date )
	{
		title = new QLabel( i18n("Date"), mainglayout, "date_label" );
		font = title->font();
		font.setBold( true );
		title->setFont( font );
	}
	
	for( KornMailSubject* subject = list->first(); subject; subject = list->next() )
	{
		new QLabel( subject->getSender(), mainglayout, "from-value" );
		new QLabel( subject->getSubject(), mainglayout, "subject-value" );
		if( date )
		{
			QDateTime tijd;
			tijd.setTime_t( subject->getDate() );
			new QLabel( tijd.toString(), mainglayout, "date-value" );
		}
	}
	
	popup->setAutoDelete( true ); //Now, now care for deleting these pointers.
	
	popup->setView( mainvlayout );
	
	popup->show(); //Display it
}

void BoxContainerItem::drawLabel( QLabel *label, const int count, const bool newMessages )
{
	//This would fail if bool have fome other values.
	short index = newMessages ? 1 : 0;
	
	bool hasAnim = _anims[ index ] && !_anims[ index ]->isEmpty();
	bool hasIcon = _icons[ index ] && !_icons[ index ]->isEmpty();
	bool hasBg = _bgColour[ index ] && _bgColour[ index ]->isValid();
	bool hasFg = _fgColour[ index ] && _fgColour[ index ]->isValid();
	
	QPixmap pixmap;
	
	label->setText( "" );
	//QToolTip::add( label, this->getTooltip() );
	
	if( hasAnim )
	{ //An animation can't have a foreground-colour and can't have a icon.
		setAnimIcon( label, *_anims[ index ] );

		hasFg = false;
		hasIcon = false;
	}
	
	if( hasIcon )
		pixmap = KGlobal::iconLoader()->loadIcon( *_icons[ index ], KIcon::Desktop, KIcon::SizeSmallMedium );
	
	if( hasIcon && hasFg )
	{
		if( hasBg )
		{
			label->setPixmap( calcComplexPixmap( pixmap, *_fgColour[ index ], _fonts[ index ], count ) );
			label->setBackgroundMode( Qt::FixedColor );
			label->setPaletteBackgroundColor( *_bgColour[ index ] );
		} else
		{
			label->setPixmap( calcComplexPixmap( pixmap, *_fgColour[ index ], _fonts[ index ], count ) );
		}
		return;
	}
	
	if( hasBg )
	{
		label->setBackgroundMode( Qt::FixedColor );
		label->setPaletteBackgroundColor( *_bgColour[ index ] );
	} else
	{
		label->setBackgroundMode( Qt::X11ParentRelative );
	}
	
	if( hasIcon )
	{
		label->setPixmap( pixmap );
	}
	
	if( hasFg )
	{
		if( _fonts[ index ] )
			label->setFont( *_fonts[ index ] );
		label->setPaletteForegroundColor( *_fgColour[ index ] );
		label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
		label->setText( QString::number( count ) );
	}
	
	if( hasFg || hasBg || hasIcon || hasAnim )
		label->show();
	else
		label->hide();
}

//This function makes a pixmap which is based on icon, but has a number painted on it.
QPixmap BoxContainerItem::calcComplexPixmap( const QPixmap &icon, const QColor& fgColour, const QFont* font, const int count )
{
	QPixmap result( icon );
	QPixmap numberPixmap( icon.size() );
	QImage iconImage( icon.convertToImage() );
	QImage numberImage;
	QRgb *rgbline;
	QPainter p;
	
	//Make a transparent number; first make a white number on a black background.
	//This pixmap also is the base alpha-channel, the foreground colour is added later.
	numberPixmap.fill( Qt::black );
	p.begin( &numberPixmap, false );
	p.setPen( Qt::white );
	if( font )
		p.setFont( *font );
	p.drawText( icon.rect(), Qt::AlignCenter, QString::number( count ) );
	p.end();

	//Convert to image and add the alpha channel.
	numberImage = numberPixmap.convertToImage();
	if( numberImage.depth() != 32 ) //Make sure depth is 32 (and thus can have an alpha channel)
		numberImage = numberImage.convertDepth( 32 );
	numberImage.setAlphaBuffer( true ); //Enable alpha channel
	for( int xx = 0; xx < numberImage.height(); ++xx )
	{
		rgbline = (QRgb*)numberImage.scanLine( xx );

		for( int yy = 0; yy < numberImage.width(); ++yy )
		{
			//Set colour and alpha channel
			rgbline[ yy ] = qRgba( fgColour.red(), fgColour.green(), fgColour.blue(), qRed( rgbline[ yy ] ) ); 
		}
	}

	//Merge icon and number and convert to result.
	KIconEffect::overlay( iconImage, numberImage );
	result.convertFromImage( iconImage );
	
	return result;
}

void BoxContainerItem::setAnimIcon( QLabel* label, const QString& anim )
{
	label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
	label->setMovie( QMovie( anim ) );
	label->show();
}

void BoxContainerItem::recheck()
{
	doRecheck();
}

void BoxContainerItem::reset()
{
	doReset();
}

void BoxContainerItem::view()
{
	doView();
}

void BoxContainerItem::runCommand()//Possible_unsafe?
{	
	if( _command->isEmpty() )
		return; //Don't execute an empty command
	runCommand( *_command );
}

void BoxContainerItem::help()
{
	kapp->invokeHelp();
}

void BoxContainerItem::reportBug()
{
	KBugReport bug( 0, true );
	bug.exec(); //modal: it doesn't recheck anymore
}

void BoxContainerItem::about()
{
	KAboutApplication about( 0, "KOrn About", true );
	about.exec();  //modal: it doesn't recheck anymore
} 

void BoxContainerItem::popup()
{
	doPopup();
}

void BoxContainerItem::showConfig()
{
	emit showConfiguration();
}

void BoxContainerItem::startTimer()
{
	doStartTimer();
}

void BoxContainerItem::stopTimer()
{
	doStopTimer();
}

void BoxContainerItem::processExited( KProcess* proc )
{
	delete proc;
}

#include "boxcontaineritem.moc"
