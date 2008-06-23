/*
 * Copyright (C) 2004-2006, Mart Kelder (mart@kelder31.nl)
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
#include "boxcontaineritemadaptor.h"

#include "mailsubject.h"
#include "settings.h"

#include <kaboutapplicationdialog.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kbugreport.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpassivepopup.h>
#include <kmenu.h>
#include <kprocess.h>
#include <kshortcut.h>
#include <kstandardaction.h>
#include <ktoolinvocation.h>
#include <kvbox.h>
#include <kcomponentdata.h>

#include <QColor>
#include <QFont>
#include <QDateTime>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <q3ptrlist.h>
#include <QString>

#include <QMovie>

BoxContainerItem::BoxContainerItem( QObject * parent )
	: AccountManager( parent ),
	_settings( 0 )
{

	new BoxContainerItemAdaptor( this );
#ifdef __GNUC__
#warning Put some useful DBus object path here
#endif
	QDBusConnection::sessionBus().registerObject( "/", this, QDBusConnection::ExportAdaptors );
}

BoxContainerItem::~BoxContainerItem()
{
}
	
void BoxContainerItem::readConfig( BoxSettings* settings, BoxSettings *config_box_settings, const int index )
{
	//Read information about how the thing have to look like
	_settings = settings;
	
	//Sets the object ID for the DBUS-object
#ifdef __GNUC__
#warning Port me to DBus (using DBus object path instead?)
#endif
//	this->setObjId( config->readEntry( "name", "" ).toUtf8() );
	
	//Read the settings of the reimplemented class.
	//It is important to read this after the box-settings, because the
	//setCount-function is called in AccountManager::readConfig
	AccountManager::readConfig( settings, config_box_settings, index );
}

void BoxContainerItem::runCommand( const QString& cmd )
{
	KProcess m_process;
	m_process.setEnv("NUMBEROFMESSAGES", QString::number( totalMessages() ) );
	if(hasNewMessages() )
	   m_process.setEnv("HASNEWMESSAGES", "yes" );
        m_process.setShellCommand(cmd);
	m_process.execute ();
}

void BoxContainerItem::mouseButtonPressed( Qt::MouseButton state )
{
	Qt::MouseButton button;
	if( state & Qt::LeftButton )
		button = Qt::LeftButton;
	else if( state & Qt::RightButton )
		button = Qt::RightButton;
	else if( state & Qt::MidButton )
		button = Qt::MidButton;
	else
		return; //Invalid mouse button

	if( _settings->clickCommand( button, BoxSettings::Recheck ) )
		doRecheck();
	if( _settings->clickCommand( button, BoxSettings::Reset ) )
		doReset();
	if( _settings->clickCommand( button, BoxSettings::View ) )
		doView();
	if( _settings->clickCommand( button, BoxSettings::Run ) )
		runCommand();
	if( _settings->clickCommand( button, BoxSettings::Popup ) )
		doPopup();
}

void BoxContainerItem::fillKMenu( KMenu* popupMenu, KActionCollection* actions )
{
	/*popupMenu->insertItem( i18n( "&Configure" ), this, SLOT( slotConfigure() ) );
	popupMenu->insertItem( i18n( "&Recheck" ), this, SLOT( slotRecheck() ) );
	popupMenu->insertItem( i18n( "R&eset Counter" ), this, SLOT( slotReset() ) );
	popupMenu->insertItem( i18n( "&View Emails" ), this, SLOT( slotView() ) );
	popupMenu->insertItem( i18n( "R&un Command" ), this, SLOT( slotRunCommand() ) );*/
	
        KAction *action  = new KAction(i18n("&Configure"), this);
        actions->addAction("configure", action );
        connect(action, SIGNAL(triggered(bool)), SLOT( slotConfigure()  ));
	popupMenu->addAction(action);
        action  = new KAction(i18n("&Recheck"), this);
        actions->addAction("recheck", action );
        connect(action, SIGNAL(triggered(bool)), SLOT( slotRecheck()    ));
        popupMenu->addAction(action);
        action  = new KAction(i18n("R&eset Counter"), this);
        actions->addAction("reset", action );
        connect(action, SIGNAL(triggered(bool)), SLOT( slotReset()      ));
        popupMenu->addAction(action);
        action  = new KAction(i18n("&View Emails"), this);
        actions->addAction("view", action );
        connect(action, SIGNAL(triggered(bool)), SLOT( slotView()       ));
        popupMenu->addAction(action);
        action  = new KAction(i18n("R&un Command"), this);
        actions->addAction("run", action );
        connect(action, SIGNAL(triggered(bool)), SLOT( slotRunCommand() ));
        popupMenu->addAction(action);
	popupMenu->addSeparator();
	popupMenu->addAction( KStandardAction::help(      this, SLOT( help()      ), actions ) );
	popupMenu->addAction( KStandardAction::reportBug( this, SLOT( reportBug() ), actions ) );
	popupMenu->addAction( KStandardAction::aboutApp(  this, SLOT( about()     ), actions ) );
}

void BoxContainerItem::showPassivePopup( QWidget* parent, QList< KornMailSubject >* list, int total,
					 const QString &accountName, bool date )
{
	KPassivePopup *popup = new KPassivePopup( parent );
	popup->setObjectName( "Passive popup" );
		
	KVBox *mainvlayout = popup->standardView( QString( "KOrn - %1/%2 (total: %3)" ).arg( _settings->boxName() ).arg( accountName )
			.arg( total ), "", QPixmap(), 0 );
	QWidget *mainglayout_wid = new QWidget( mainvlayout );
	QGridLayout *mainglayout = new QGridLayout( mainglayout_wid );
	
	QLabel *title = new QLabel( i18n("From"), mainglayout_wid );
	mainglayout->addWidget( title, 0, 0 );
	QFont font = title->font();
	font.setBold( true );
	title->setFont( font );
		
	title = new QLabel( i18n("Subject"), mainglayout_wid );
	mainglayout->addWidget( title, 0, 1 );
	font = title->font();
	font.setBold( true );
	title->setFont( font );
		
	//Display only column 3 if 'date' is true.
	if( date )
	{
		title = new QLabel( i18n("Date"), mainglayout_wid );
		mainglayout->addWidget( title, 0, 2 );
		font = title->font();
		font.setBold( true );
		title->setFont( font );
	}
	
	for( int xx = 0; xx < list->size(); ++xx )
	{
		//Make a label, add it to the layout and place it into the right position.
		mainglayout->addWidget( new QLabel( list->at( xx ).getSender(), mainglayout_wid ), xx + 1, 0 );
		mainglayout->addWidget( new QLabel( list->at( xx ).getSubject(), mainglayout_wid ), xx + 1, 1 );
		if( date )
		{
			QDateTime tijd;
			tijd.setTime_t( list->at( xx ).getDate() );
			mainglayout->addWidget( new QLabel( tijd.toString(), mainglayout_wid ), xx + 1, 2 );
		}
	}
	
	popup->setAutoDelete( true ); //Now, now care for deleting these pointers.
	
	popup->setView( mainvlayout );
	
	popup->show(); //Display it
}

void BoxContainerItem::drawLabel( QLabel *label, const int count, const bool newMessages )
{
	//This would fail if bool have fome other values.
	BoxSettings::State messageState = newMessages ? BoxSettings::New : BoxSettings::Normal;

	if( !_settings )
	{
		kWarning() <<"Drawing label before settings are initialized!";
		return;
	}
	
	bool hasAnim = _settings->hasAnimation( messageState ) && !_settings->animation( messageState ).isEmpty();
	bool hasIcon = _settings->hasIcon( messageState ) && !_settings->icon( messageState ).isEmpty();
	bool hasBg = _settings->hasBackgroundColor( messageState ) && _settings->backgroundColor( messageState ).isValid();
	bool hasFg = _settings->hasForegroundColor( messageState ) && _settings->foregroundColor( messageState ).isValid();
	
	QPixmap pixmap;
	QPalette palette = label->palette();
	
	if( label->movie() ) //Delete movie pointer
		delete label->movie();
	//label->setMovie( 0 ); //TODO: crash in KDE4!
	
	label->setText( "" );
	//label->setToolTip( this->getTooltip() );
	
	if( hasAnim )
	{ //An animation can't have a foreground-colour and can't have a icon.
		setAnimIcon( label, _settings->animation( messageState ) );

		hasFg = false;
		hasIcon = false;
	}
	
	if( hasIcon )
		pixmap = KIconLoader::global()->loadIcon( _settings->icon( messageState ), KIconLoader::Desktop, KIconLoader::SizeSmallMedium );
	
	if( hasIcon && hasFg )
	{
		if( hasBg )
		{
			label->setPixmap( calcComplexPixmap( pixmap, _settings->foregroundColor( messageState ),
						             _settings->font( messageState ), count ) );
			//label->setBackgroundMode( Qt::FixedColor );
			//label->setPaletteBackgroundColor( _settings->backgroundColor( messageState ) );
			label->setBackgroundRole( QPalette::Window );
			palette.setColor( label->backgroundRole(), _settings->backgroundColor( messageState ) );
		} else
		{
			label->setPixmap( calcComplexPixmap( pixmap, _settings->foregroundColor( messageState ),
						             _settings->font( messageState ), count ) );
		}
		return;
	}
	
	if( hasBg )
	{
		//label->setBackgroundMode( Qt::FixedColor );
		//label->setPaletteBackgroundColor( *_bgColour[ index ] );
		label->setBackgroundRole( QPalette::Window );
		palette.setColor( label->backgroundRole(), _settings->backgroundColor( messageState ) );
	} else
	{
		//label->setBackgroundMode( Qt::X11ParentRelative );
		label->setBackgroundRole( QPalette::NoRole );
		palette.setColor( QPalette::Window, Qt::transparent );
	}
	
	if( hasIcon )
	{
		label->setPixmap( pixmap );
	}
	
	if( hasFg )
	{
		if( _settings->hasFont( messageState ) )
			label->setFont( _settings->font( messageState ) );
		palette.setColor( label->foregroundRole(), _settings->foregroundColor( messageState ) );
		//label->setPaletteForegroundColor( *_fgColour[ index ] );
		label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
		label->setText( QString::number( count ) );
	}

	label->setPalette( palette );
	label->setAutoFillBackground( true );
	
	if( hasFg || hasBg || hasIcon || hasAnim )
		label->show();
	else
		label->hide();
}

bool BoxContainerItem::makePixmap( QPixmap& pixmap, const int count, const bool newMessages )
{
	BoxSettings::State messageState = newMessages ? BoxSettings::New : BoxSettings::Normal;
	QPixmap otherPixmap;
	QPainter p;
	bool isEmpty = true;

	p.begin( &pixmap );

	//draw background
	if( _settings->hasBackgroundColor( messageState ) && _settings->backgroundColor( messageState ).isValid() )
	{
		//Fill with background color
		p.fillRect( pixmap.rect(), QBrush( _settings->backgroundColor( messageState ) ) );
	}
	else
	{
		//Clear background
		//TODO: Check if it is still works good: transparent might not receive mouse clicks (documentation QIcon)
		p.fillRect( pixmap.rect(), QBrush( Qt::transparent ) );
		isEmpty = false;
	}

	//Draw pixmap
	if( _settings->hasIcon( messageState ) && !_settings->icon( messageState ).isEmpty() )
	{
		otherPixmap = KIconLoader::global()->loadIcon( _settings->icon( messageState ), KIconLoader::Desktop, KIconLoader::SizeSmallMedium );
		if( !otherPixmap.isNull() )
		{
			p.drawPixmap( pixmap.rect(), otherPixmap, otherPixmap.rect() );
			isEmpty = false;
		}
	}

	//Draw text
	if( _settings->hasForegroundColor( messageState ) && _settings->foregroundColor( messageState ).isValid() )
	{
		p.setFont( _settings->font( messageState ) );
		p.setPen( _settings->foregroundColor( messageState ) );
		p.drawText( pixmap.rect(), Qt::AlignCenter, QString::number( count ) );
		isEmpty = false;
	}
	
	p.end();

	return isEmpty;
}

//This function makes a pixmap which is based on icon, but has a number painted on it.
QPixmap BoxContainerItem::calcComplexPixmap( const QPixmap &icon, const QColor& fgColour, const QFont &font, const int count )
{
	QPixmap result( icon );
	QPainter p;

	p.setCompositionMode( QPainter::CompositionMode_DestinationOver );
	p.begin( &result );
	p.setPen( fgColour );
	p.setFont( font );
	p.drawText( icon.rect(), Qt::AlignCenter, QString::number( count ) );
	
	return result;
}

void BoxContainerItem::setAnimIcon( QLabel* label, const QString& anim )
{
	label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
	label->setMovie( new QMovie( anim ) );
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
	if( _settings->command().isEmpty() )
		return; //Don't execute an empty command
	runCommand( _settings->command() );
}

void BoxContainerItem::help()
{
	KToolInvocation::invokeHelp();
}

void BoxContainerItem::reportBug()
{
	KBugReport bug( 0, true );
	bug.exec(); //modal: it doesn't recheck anymore
}

void BoxContainerItem::about()
{
	KAboutApplicationDialog about( KGlobal::mainComponent().aboutData() );
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

#include "boxcontaineritem.moc"
