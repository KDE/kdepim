/* KPilot
**
** Copyright (C) 2004 by Adriaan de Groot
**
** This file is part of the Perl conduit, a conduit for KPilot that
** is intended to showcase how to use perl code inside a conduit.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

/*
** This file is based on the perlembed examples, from
** http://search.cpan.org/dist/perl/pod/perlembed.pod
*/

#ifdef DEBUG
#undef DEBUG
#define DEBUG (1)
#else
#define DEBUG (1)
#endif

#include "options.h"


#include "perl-conduit.h"  // The Conduit action
#include "perlconduit.h"   // The settings class

#include <qthread.h>
#include <qapplication.h>

#include <EXTERN.h>
#include <perl.h>

extern "C"
{
long version_conduit_perl = KPILOT_PLUGIN_API;
const char *id_conduit_perl =
	"$Id$";
}

/* From the cvs log mailing list:
Am Dienstag, 13. April 2004 22:26 schrieb Adriaan de Groot:
> On Tuesday 13 April 2004 20:53, Reinhold Kainhofer wrote:
> > rename fPerl to my_perl to make it compile again...
>
> Note that the renaming is needed because perl 5.8 is a brain-dead abortion,
> and the code works fine with perl 5.6.
*/
class PerlThread : public QThread
{
public:
	PerlThread(QObject *parent,
		const QString &pilotPath,
		int fd) : fParent(parent),
		fPath(pilotPath),fSocket(fd) { } ;
	virtual void run();

	QString result() const { return fResult; } ;

protected:
	QObject *fParent;
	PerlInterpreter *my_perl;
	QString fPath;
	int fSocket;

	QString fResult;
} ;


PerlConduit::PerlConduit(KPilotDeviceLink *d,
	const char *n,
	const QStringList &l) :
	ConduitAction(d,n,l)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << id_conduit_perl << endl;
#endif
	fConduitName=i18n("Perl");

	(void) id_conduit_perl;
}

PerlConduit::~PerlConduit()
{
	FUNCTIONSETUP;
}

/* virtual */ bool PerlConduit::exec()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": In exec() @" << (unsigned long) this << endl;
#endif

	fThread = new PerlThread(this,
		fHandle->pilotPath(),
		/* fHandle-> */pilotSocket()) ;
	fThread->start();
	startTickle();
	return true;
}

/* virtual */ bool PerlConduit::event(QEvent *e)
{
	FUNCTIONSETUP;

	if (e->type() == QEvent::User)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Perl thread done." << endl;
#endif
		QString r;
		addSyncLogEntry(i18n("Perl returned %1.").arg(fThread->result()));
		stopTickle();
		delayDone();
		return true;
	}
	else return ConduitAction::event(e);
}

static const char *perl_args[] = { "", "-e", "0" } ;

void PerlThread::run()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Thread starting." << endl;
#endif

	my_perl = perl_alloc();
	perl_construct(my_perl);
	perl_parse(my_perl, NULL, 3, const_cast<char **>(perl_args), NULL);
	perl_run(my_perl);

	eval_pv( (CSL1("%kpilot=(") +
		CSL1("device=>\"%1\",").arg(fPath) +
		CSL1("socket=%1,").arg(fSocket) +
		// Add more data here in same style, don't forget " and ,
		CSL1("version=%1);").arg(KPILOT_PLUGIN_API)).latin1(),
		TRUE);

	eval_pv(PerlConduitSettings::expression().latin1(),TRUE);

	SV *retval = get_sv("a",FALSE);
	if (retval)
	{
		fResult.setNum(SvIV(retval));
	}
	else
	{
		fResult = i18n("No value");
	}


#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Thread woken with " << fResult << endl;
#endif


	perl_destruct(my_perl);
	perl_free(my_perl);

	QApplication::postEvent(fParent,new QEvent(QEvent::User));
}

