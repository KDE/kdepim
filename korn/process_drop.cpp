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

#include <qbytearray.h>
#include <qregexp.h>
#include <qstring.h>

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
	
	//Make the process if it doens't exist yet.
	if( !_process && !_program->isEmpty() )
	{
		_process = new KShellProcess;
		*_process << *_program;
		connect( _process, SIGNAL( processExited( KProcess* ) ), this, SLOT( slotExited( KProcess* ) ) );
		connect( _process, SIGNAL( receivedStdout( KProcess*, char*, int ) ), this, SLOT( slotDataReceived( KProcess *, char*, int) ) );
	}

	//Start process if it is not already running
	if( _process && !_process->isRunning() )
	{
		_valid = _process->start( KProcess::NotifyOnExit, KProcess::Stdout );
		if( !_valid )
			kdWarning() << i18n( "Could not start process %1" ).arg( *_program ) << endl; 
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

void ProcessDrop::slotExited( KProcess* )
{
}

void ProcessDrop::slotDataReceived( KProcess *proc, char* data, int length )
{
	QString line;
	QRegExp lastNumber( "(.*\\D|)(\\d+)\\D*");
	int index;
	int result = -1;

	//Test if the message comes from the right process
	if( proc != _process ) 
	{
		kdDebug() << "Got wrong process in slotDataReceived()" << endl;
		return;
	}

	//Append the data to the buffer
	_receivedBuffer->append( QByteArray( data, length ) );
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
