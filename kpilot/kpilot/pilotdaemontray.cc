/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2001-2007 by Adriaan de Groot <groot@kde.org>
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2006-2007 Jason 'vanRijn' Kasper <vr@movingparts.net>
** Copyright (C) 2008 by Bertjan Broeksema <b.broeksema@kdemail.net>
**
** This is the KPilot Daemon System tray.
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "pilotdaemontray.h"

#include <QtCore/QTimer>
#include <QtGui/QAction>
#include <QtGui/QMenu>

#include <kaboutapplicationdialog.h>
#include <khelpmenu.h>
#include <kmenu.h>

#include "kpilotConfig.h"
#include "options.h"
#include "syncAction.h"

class PilotDaemonTray::Private
{
public:
	Private() 
		: fBlinkTimer( 0L )
		, fConfigureConduitsMenuItem( 0L )
		, fDefaultSyncActionMenuItem( 0L )
		, fAboutDialog( 0L )
		, fKPilotMenuItem( 0L )
		, fSyncTypeMenu( 0L )
		, fSyncTypeActionsGroup( 0L )
	{}
	
	const KAboutData* fAboutData;
	
	/**
	 * Timer for blinking.
	 */
	QTimer *fBlinkTimer;
	
	IconShape fCurrentIcon;
	
	/**
	 * Remember which item in the context menu
	 * is "Configure Conduits" so we can enable / disable
	 * it as necessary.
	 */
	QAction* fConfigureConduitsMenuItem;
	
	QAction* fDefaultSyncActionMenuItem;

	QIcon fIcons[((int) NotListening) + 1];
	
	/**
	 * Window for the "About KPilot" information.
	 */
	KAboutApplicationDialog* fAboutDialog;
	
	/**
	 * Remember which item in the context menu
	 * is "Run KPilot" so we can enable / disable
	 * it as necessary.
	 */
	QAction* fKPilotMenuItem;
	
	/**
	 * Menu of sync types.
	 */
	QMenu* fSyncTypeMenu;
	
	QActionGroup *fSyncTypeActionsGroup;
};

PilotDaemonTray::PilotDaemonTray( QWidget* parent ) 
	: KSystemTrayIcon( parent )
	, d( new Private )
{
	FUNCTIONSETUP;
	
	setObjectName( "PilotDaemonTray" );
	setupWidget();
}

PilotDaemonTray::~PilotDaemonTray()
{
	KPILOT_DELETE( d );
}

void PilotDaemonTray::changeIcon(IconShape i)
{
	FUNCTIONSETUP;
	
	if( d->fIcons[i].isNull() )
	{
		WARNINGKPILOT << "Icon #"<<i<< " is NULL!";
	}
	
	setIcon ( d->fIcons[i] );
	d->fCurrentIcon = i;
}

void PilotDaemonTray::setAboutData( const KAboutData* aboutData )
{
	d->fAboutData = aboutData;
	
	KHelpMenu *helpMenu = new KHelpMenu( contextMenu(), aboutData );
	helpMenu->menu()->setIcon( KIcon( CSL1( "system-help" ) ) );
	contextMenu()->addMenu( helpMenu->menu() );
}

void PilotDaemonTray::selectDefaultSyncType()
{
	slotHandleDefaultSyncToggle( true );
}

void PilotDaemonTray::startBlinking()
{
	changeIcon( Busy );
	
	if( !d->fBlinkTimer )
	{
		d->fBlinkTimer = new QTimer(this);
		d->fBlinkTimer->setObjectName( CSL1( "blink timer" ) );
	}
	if( d->fBlinkTimer )
	{
		connect( d->fBlinkTimer, SIGNAL( timeout() ), this, SLOT( slotBusyTimer() ) );
		d->fBlinkTimer->setSingleShot( false );
		d->fBlinkTimer->start( 750 );
	}
}

void PilotDaemonTray::stopBlinking()
{
	changeIcon( Normal );
	
	if( d->fBlinkTimer )
	{
		d->fBlinkTimer->stop();
	}
}

void PilotDaemonTray::enableRunKPilot(bool b)
{
	FUNCTIONSETUP;
	
	d->fKPilotMenuItem->setEnabled( b );
	d->fConfigureConduitsMenuItem->setEnabled( b );
}

void PilotDaemonTray::setupWidget()
{
	FUNCTIONSETUP;
	KIconLoader::global()->addAppDir( CSL1("kpilot") );

#define L( idx, name ) \
	d->fIcons[idx] = loadIcon( CSL1( name ) ); \
	if( d->fIcons[idx].isNull() ) { WARNINGKPILOT << "No icon" << name; } \
	else { DEBUGKPILOT << "Loaded icon" << name; }

	L( Normal, "kpilotDaemon" )
	L( Busy, "kpilot_busysync" )
	L( NotListening, "kpilot_nosync" )
#undef L

	connect( this, SIGNAL( activated(QSystemTrayIcon::ActivationReason ) )
		, this, SLOT( slotHandleActivation( QSystemTrayIcon::ActivationReason ) ) );

	slotShowNotListening();
	QTimer::singleShot( 2000, this, SLOT(slotShowNormal() ) );

	d->fKPilotMenuItem = new QAction( i18n( "Start &KPilot" ), this );
	connect( d->fKPilotMenuItem, SIGNAL( triggered() )
		, this, SIGNAL( startKPilotRequest() ) );

	d->fConfigureConduitsMenuItem = new QAction( i18n("&Configure KPilot..."), this );
	connect( d->fConfigureConduitsMenuItem, SIGNAL( triggered() ), this
		, SIGNAL( startConfigurationRequest() ) );
	
	contextMenu()->addAction( d->fKPilotMenuItem );
	contextMenu()->addAction( d->fConfigureConduitsMenuItem );
	contextMenu()->addSeparator();

	// Configured default sync
	d->fDefaultSyncActionMenuItem = new QAction( i18nc("Default sync action"
		, "Default (%1)", SyncAction::SyncMode::name( (SyncAction::SyncMode::Mode)
			KPilotSettings::syncType() ) ), this );
	d->fDefaultSyncActionMenuItem->setCheckable( true );
	d->fDefaultSyncActionMenuItem->setData( (int) 0 );
	
	connect( d->fDefaultSyncActionMenuItem, SIGNAL( toggled( bool ) )
		, this, SLOT( slotHandleDefaultSyncToggle( bool ) ) );

	d->fSyncTypeMenu = new QMenu( i18n( "Next &Sync" ), contextMenu() );
	d->fSyncTypeMenu->setObjectName( "sync_type_menu" );
	d->fSyncTypeMenu->addAction( d->fDefaultSyncActionMenuItem );
	d->fSyncTypeMenu->addSeparator();
	
	QString once = i18nc( "Appended to names of sync types to indicate the sync"
		" will happen just one time", " (once)" );

	d->fSyncTypeActionsGroup = new QActionGroup( this );
	QAction *action;
	QString syncModeName;

#define MI(a,b) \
	syncModeName = SyncAction::SyncMode::name( a ); \
	if( a != KPilotSettings::syncType() ) \
		syncModeName.append( once ); \
	action = new QAction( syncModeName, this ); \
	action->setCheckable( true ); \
	action->setChecked( b ); \
	action->setData( (int) a ); \
	d->fSyncTypeActionsGroup->addAction( action ); \
	d->fSyncTypeMenu->addAction( action ); \

	// Keep this synchronized with kpilotui.rc and kpilot.cc if at all possible.
	MI( SyncAction::SyncMode::eHotSync, true );
	MI( SyncAction::SyncMode::eFullSync, false );
	MI( SyncAction::SyncMode::eCopyPCToHH, false );
	MI( SyncAction::SyncMode::eCopyHHToPC, false );
   /*
   TODO: Put these back when we're sure they still work. =:/
	MI( SyncAction::SyncMode::eBackup, false );
	MI( SyncAction::SyncMode::eRestore, false );
   */
#undef MI

	connect( d->fSyncTypeActionsGroup, SIGNAL( triggered( QAction* ) ), this
		, SLOT( slotHandleActionTrigger( QAction* ) ) );
	
	// See toggleDefaultSync(). This is only useful now all other action are
	// added to syncTypeActions.
	d->fDefaultSyncActionMenuItem->setChecked( true );

	contextMenu()->addMenu( d->fSyncTypeMenu );
}

void PilotDaemonTray::slotBusyTimer()
{
	if( d->fCurrentIcon == Busy )
	{
		changeIcon( Normal );
	}
	else if( d->fCurrentIcon == Normal )
	{
		changeIcon( Busy );
	}
}

void PilotDaemonTray::slotHandleActionTrigger( QAction* action )
{
	FUNCTIONSETUP;
	
	if( !action )
	{
		WARNINGKPILOT << "Ignored OL action pointer";
		return;
	}
	
	unsigned int actionData = action->data().toInt();
	
	// This should always pass, otherwise actionData does not contain a valid
	// SyncAction::SyncMode.
	Q_ASSERT( actionData >= 1 && actionData <= 6 );
	
	// Check if the selected sync mode is different than the default mode and
	// enable / disable the default syncaction checkbox accordingly.
	if( actionData != KPilotSettings::syncType() )
	{
		d->fDefaultSyncActionMenuItem->setChecked( false );
		d->fDefaultSyncActionMenuItem->setEnabled( true );
	}
	else
	{
		d->fDefaultSyncActionMenuItem->setChecked( true );
		d->fDefaultSyncActionMenuItem->setEnabled( false );
	}
	
	emit nextSyncChangedTo( actionData );
}

void PilotDaemonTray::slotHandleActivation( QSystemTrayIcon::ActivationReason reason )
{
	// Send out a request to start kpilot when the tray is clicked once.
	if( reason == QSystemTrayIcon::Trigger )
	{
		emit startKPilotRequest();
	}
}

void PilotDaemonTray::slotHandleDefaultSyncToggle( bool defaultActionEnabled )
{
	if( !defaultActionEnabled )
	{
		return;
	}
	
	foreach( QAction* action, d->fSyncTypeActionsGroup->actions() )
	{
		if( action->data().toUInt() == KPilotSettings::syncType() )
		{
			action->setChecked( true );
			d->fDefaultSyncActionMenuItem->setChecked( true );
			d->fDefaultSyncActionMenuItem->setEnabled( false );
			emit nextSyncChangedTo( KPilotSettings::syncType() );
			break;
		}
	}
}

void PilotDaemonTray::slotShowBusy()
{
	FUNCTIONSETUP;
	
	changeIcon( Busy );
}

void PilotDaemonTray::slotShowNormal()
{
	FUNCTIONSETUP;
	
	changeIcon( Normal );
}

void PilotDaemonTray::slotShowNotListening()
{
	FUNCTIONSETUP;
	
	changeIcon( NotListening );
}
