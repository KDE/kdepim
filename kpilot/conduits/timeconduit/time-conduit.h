#ifndef _Time_CONDUIT_H
#define _Time_CONDUIT_H
// Time-conduit.cc
//
// Copyright (C) 2002 by Reinhold Kainhofer
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$
//

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/



#include <plugin.h>

#include <kapplication.h>

class TimeConduit : public ConduitAction
{
Q_OBJECT
public:
	TimeConduit(
		KPilotDeviceLink *o,
		const char *n = 0L,
		const QStringList &a = QStringList() );
	virtual ~TimeConduit();

public slots:
	virtual void exec();
	void syncPCToPalm();
	void syncPalmToPC();

protected:
	void readConfig();
private:
	int fDirection;
} ;




// $Log$
// Revision 1.21  2002/07/23 00:52:02  kainhofe
// Reorder the resolution methods
//
// Revision 1.20  2002/07/20 18:50:45  kainhofe
// added a terrible hack to add new contacts to the addressbook. Need to fix kabc for this...
//
// Revision 1.19  2002/07/09 22:40:18  kainhofe
// backup database fixes, prevent duplicate vcal entries, fixed the empty record that was inserted on the palm on every sync
//
// Revision 1.18  2002/07/01 23:25:46  kainhofe
// implemented categories syncing, many things seem to work, but still every sync creates an empty zombie.
//
// Revision 1.17  2002/06/30 22:17:50  kainhofe
// some cleanup. Changes from the palm are still not applied to the pc, pc->palm still disabled.
//
// Revision 1.16  2002/06/30 16:23:23  kainhofe
// Started rewriting the addressbook conduit to use libkabc instead of direct dcop communication with Time. Palm->PC is enabled (but still creates duplicate addresses), the rest is completely untested and thus disabled for now
//
// Revision 1.15  2002/05/15 17:15:32  gioele
// kapp.h -> kapplication.h
// I have removed KDE_VERSION checks because all that files included "options.h"
// which #includes <kapplication.h> (which is present also in KDE_2).
// BTW you can't have KDE_VERSION defined if you do not include
// - <kapplication.h>: KDE3 + KDE2 compatible
// - <kdeversion.h>: KDE3 only compatible
//
// Revision 1.14  2002/04/16 18:22:12  adridg
// Wishlist fix from David B: handle formatted names when syncing
//
// Revision 1.13  2001/12/10 22:10:17  adridg
// Make the conduit compile, for Danimo, but it may not work
//
// Revision 1.12  2001/10/31 23:54:45  adridg
// CVS_SILENT: Ongoing conduits ports
//
#endif
