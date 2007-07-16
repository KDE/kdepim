/*
 * Copyright (C) 2005, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Thisprogram is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "process_drop.h"

#include <kconfigbase.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>

#include <QByteArray>
#include <QRegExp>
#include <QString>

ProcessDrop::ProcessDrop()
	: KPollableDrop(),
	_valid( true ),
	_waitForRechecked( false ),
	_process( 0 ),
	_program( new QString() ),
	_receivedBuffer( new QByteArray() )
{
}

ProcessDrop::~ProcessDrop()
{
	delete _process;
	delete _program;
	delete _receivedBuffer;
}

bool ProcessDrop::valid()
{
	return _valid;
}

void ProcessDrop::recheck()
{
	_waitForRechecked = true;

	//Make the process if it doesn't exist yet.
	if( !_process && !_program->isEmpty() )
	{
		_process = new KProcess;
		_process->setShellCommand(*_program);
		connect(_process, SIGNAL(finished(int, QProcess::ExitStatus)),
				SLOT(slotFinished(int, QProcess::ExitStatus)));

		connect( _process, SIGNAL( processExited( K3Process* ) ), this, SLOT( slotExited( K3Process* ) ) );
		connect( _process, SIGNAL(readyReadStandardOutput ()), this, SLOT( slotDataReceived() ) );
	}

	//Start process if it is not already running
	if( _process && (_process->state ()!=QProcess::Running) )
	{
		_process->start();
		_valid = _process->waitForStarted();
		if( !_valid )
			kWarning() << i18n( "Could not start process %1", *_program ) << endl;
	}

}

KMailDrop *ProcessDrop::clone() const
{
	return new ProcessDrop();
}

QString ProcessDrop::type() const
{
	return "process";
}

bool ProcessDrop::readConfigGroup ( const KConfigBase& cfg )
{
	*_program = cfg.readEntry( "program", "" );

	//Delete process to make sure nothing of the process actually runs.
	//It is recreated at first recheck.
	delete _process; _process = 0;

	return true;
}

bool ProcessDrop::writeConfigGroup ( KConfigBase& ) const
{
	return true;
}

void ProcessDrop::slotExited(int, QProcess::ExitStatus)
{
}

void ProcessDrop::slotDataReceived()
{
	QString line;
	QRegExp lastNumber( "(.*\\D|)(\\d+)\\D*");
	int index;
	int result = -1;

	//Append the data to the buffer
	_receivedBuffer->append( _process->readAllStandardOutput () );
	_receivedBuffer->replace( '\n', '\0' );

	//Search for numbers in every completed line
	while( ( index = _receivedBuffer->indexOf( '\0' ) ) != -1 )
	{
		line = _receivedBuffer->left( index );
		_receivedBuffer->remove( 0, index + 1 );
		if( lastNumber.exactMatch( line ) )
		{ //Number is found
			result = lastNumber.cap( 2 ).toInt();
		}
	}

	if( result >= 0 )
	{
		emit( changed( result, this ) );
		if( _waitForRechecked )
		{
			emit( rechecked() );
			_waitForRechecked = false;
		}
	}
}


#include "process_drop.moc"
