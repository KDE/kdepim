/* memofileconduit.cc			KPilot
**
** Copyright (C) 2008 by Jason 'vanRijn' Kasper <vR@movingparts.net>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "memofileconduit.h"

#include <kglobal.h>

#include "idmapping.h"
#include "options.h"
#include "memofilepcdataproxy.h"
#include "memofilepcrecord.h"
#include "memofilehhrecord.h"
#include "memofilehhdataproxy.h"
#include "memofilesettings.h"
#include "pilotMemo.h"


class MemofileConduit::Private
{
public:
	Private() :
           _DEFAULT_MEMODIR( QDir::homePath() + CSL1( "/MyMemos" ) ),
	   fMemoAppInfo( 0L ),
	   _memofiles( 0L ),
           fHHDataProxy( 0 ),
           fPCDataProxy( 0 )
	{
	}
        // configuration settings...
	QString	_DEFAULT_MEMODIR;
	QString	_memo_directory;
	bool	_sync_private;

        PilotMemoInfo	*fMemoAppInfo;
	QList<PilotMemo> fMemoList;

	// our categories
	MemoCategoryMap fCategories;

	Memofiles * _memofiles;
};


inline bool _equal(const QString & str1, const QString & str2)
{
	return ( str1.isEmpty() && str2.isEmpty() ) || ( str1 == str2 );
}


MemofileConduit::MemofileConduit( KPilotLink *o, const QVariantList &a)
	: RecordConduit( o, a, CSL1( "MemoDB" ), CSL1( "Memofile Conduit" )),
          d( new MemofileConduit::Private )
{
}

MemofileConduit::~MemofileConduit()
{
	KPILOT_DELETE( d );
}

void MemofileConduit::loadSettings()
{
	FUNCTIONSETUP;

	MemofileSettings::self()->readConfig();

       	QString dir(MemofileConduitSettings::directory());
	if (dir.isEmpty()) {
		dir = _DEFAULT_MEMODIR;

		DEBUGKPILOT
			<< ": no directory given to us.  defaulting to: ["
			<< _DEFAULT_MEMODIR
			<< "]";
	}

	d->_memo_directory = dir;
	d->_sync_private = MemofileConduitSettings::syncPrivate();


	DEBUGKPILOT
		<< ": Settings... "
		<< "  directory: [" << d->_memo_directory
		<< "], sync private: [" << d->_sync_private
		<< "]";

	return true;
}

bool MemofileConduit::initDataProxies()
{
	FUNCTIONSETUP;

	if( !fDatabase )
	{
		addSyncLogEntry( i18n( "Error: Handheld database is not loaded." ) );
		return false;
	}

        fHHDataProxy = new MemofileHHDataProxy( fDatabase );
	fBackupDataProxy = new MemofileHHDataProxy( fLocalDatabase );
	fPCDataProxy = new MemofilePCDataProxy( fMapping, d->_memo_directory );

	fHHDataProxy->loadAllRecords();
	fBackupDataProxy->loadAllRecords();
	fPCDataProxy->loadAllRecords();

	return true;
}

bool MemofileConduit::equal( const Record *pcRec, const HHRecord *hhRec ) const
{
	FUNCTIONSETUP;

	// empty records are never equal!
	if ( !pcRec || !hhRec )
	{
       		DEBUGKPILOT  << "pcRec or hhRec are null";
		return false;
	}

	const MemofilePCRecord* mPCRec = static_cast<const MemofilePCRecord*>( pcRec );
	const MemofileHHRecord* mHHrec = static_cast<const MemofileHHRecord*>( hhRec );

       	if ( !mPCRec || !mHHRec )
	{
       		DEBUGKPILOT  << "mPCRec or mHHRec are null";
                return false;
	}

	if ( !_equal( mHHRec.text(), mPCRec.text() ) )
	{
		DEBUGKPILOT  << "text not equal [pc,hh]: [" << mPCRec.text()
			<< ", " << mHHRec.text() << "]";
		return false;
	}

	// Check that the name of the category of the HH record
	// is one matching the PC record.
	QString cat = fHHDataProxy->bestMatchCategory( pcRec->categories(),
                                                       hhRec->category() );

	if( hhRec->category() != "Unfiled" && !_equal( cat, hhRec->category() ) )
	{
		DEBUGKPILOT  << "category not equal: " << cat << ", " << hhRec->category();
		return false;
	}

        return true;
}

Record* MemofileConduit::createPCRecord( const HHRecord *hhRec )
{
	FUNCTIONSETUP;

	Record* rec = new MemofilePCRecord( PilotMemo().pack(), "Unfiled" );
	copy( hhRec, rec );

	Q_ASSERT( equal( rec, hhRec ) );

	return rec;
}

HHRecord* MemofileConduit::createHHRecord( const Record *pcRec )
{
	FUNCTIONSETUP;

	HHRecord* hhRec = new MemofileHHRecord( PilotMemo().pack(), "Unfiled" );
	copy( pcRec, hhRec );

	Q_ASSERT( equal( pcRec, hhRec ) );

	return hhRec;
}

void MemofileConduit::_copy( const Record *from, HHRecord *to )
{
	FUNCTIONSETUP;

	const MemofilePCRecord* pcFrom
		= static_cast<const MemofilePCRecord*>( from );
	MemofileHHRecord* hhTo = static_cast<MemofileHHRecord*>( to );

        hhTo->setText( pcFrom->text() );
        hhTo->setCategory( pcFrom->category() );
}

void MemofileConduit::_copy( const HHRecord *from, Record *to  )
{
	FUNCTIONSETUP;

	const MemofileHHRecord* hhFrom
		= static_cast<const MemofileHHRecord*>( from );
	MemofilePCRecord* pcTo = static_cast<MemofilePCRecord*>( to );

        pcTo->setText( hhFrom->text() );
        pcTo->setCategory( hhFrom->category() );
}
