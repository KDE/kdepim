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
	virtual bool exec();

public slots:
	void syncPCToPalm();
	void syncPalmToPC();

protected:
	void readConfig();
private:
	int fDirection;
} ;

#endif
