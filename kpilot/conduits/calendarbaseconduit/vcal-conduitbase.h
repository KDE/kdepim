#ifndef _KPILOT_VCAL_CONDUITBASE_H
#define _KPILOT_VCAL_CONDUITBASE_H
/* vcal-conduit.h                       KPilot
**
** Copyright (C) 2002-2003 Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the vcal-conduit plugin.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <recordConduit.h>


namespace KCal
{
class Calendar;
class Incidence;
}

class PilotRecord;
class PilotSerialDatabase;
class PilotLocalDatabase;
class PilotAppCategory;
class VCalConduitSettings;




class VCalConduitBase : public RecordConduit
{
Q_OBJECT
public:
	VCalConduitBase( QString name, KPilotDeviceLink *o, const char *n = 0L,
		const QStringList &a = QStringList() );
	virtual ~VCalConduitBase();

	class VCalEntry : public RecordConduit::PCEntry
	{
	public:
		VCalEntry( KCal::Incidence *inc );
		virtual ~VCalEntry();
		virtual QString uid() const;
		virtual recordid_t recid() const;
		virtual void setRecid( recordid_t recid );
		virtual bool isEmpty() const;
		virtual bool isArchived() const;
		virtual bool makeArchived();
		virtual bool insertCategory( QString cat );
		virtual bool operator ==( const VCalEntry &ent ) { return mIncidence == ent.mIncidence; }
		KCal::Incidence *incidence() const { return mIncidence; }
	protected:
		KCal::Incidence *mIncidence;
	};
	
	class VCalDataBase : public RecordConduit::PCData
	{
	public:
		class Iterator : public RecordConduit::PCData::Iterator
		{
		public:
			Iterator( KCal::Incidence::List &lst ) : mIt( lst.begin() ) {}
			virtual PCEntry *operator*() { return new VCalEntry( *mIt ); }
			virtual void operator++() { ++mIt; }
			KCal::Incidence::List::Iterator mIt;
		};
	public:
		VCalDataBase( RecordConduit *conduit, VCalConduitSettings *cfg );
		virtual ~VCalDataBase();

		/*************************************************************
		 * pure virtuals, must be implemented by childclasses
		 *************************************************************/
		 
		virtual bool loadData();
		virtual bool initData() = 0;
		virtual bool saveData();
		virtual QString description() const { return i18n("calendar"); }
		virtual bool isEmpty() const;
		virtual PCData::Iterator begin();
		virtual bool atEnd( const PCData::Iterator & );
		virtual bool increaseNextModified( PCData::Iterator & );
		virtual PCEntry *findByUid( QString uid ) const;
		virtual const QStringList uids() const;
		virtual bool updateEntry( const PCEntry* entry );
		virtual bool removeEntry( const PCEntry* entry );
		
		void setConfig( VCalConduitSettings*cfg ) { mConfig = cfg; }
		VCalConduitSettings *config() const { return mConfig; }
		KCal::Calendar *calendar() const { return mCalendar; }

		/*************************************************************
		 * non-pure virtuals, might be reimplemented by childclasses
		 *************************************************************/

	protected:
		KCal::Calendar *mCalendar;
		KCal::Incidence::List mIncidences;
		KCal::Incidence::List::ConstIterator mIncidencesIt;

		QString mCalendarFile;
		VCalConduitSettings*mConfig;
	}; 
	
	/************* INTERFACE for child classes ******************/
	
	virtual void readConfig();
	
	
	enum {
		eqFlagsName=0x04,
		eqFlagsAdress=0x08,
		eqFlagsPhones=0x10,
		eqFlagsNote=0x20,
		eqFlagsCustom=0x40,
	};

// 	virtual bool _equal( const PilotAppCategory *palmEntry, PCEntry *pcEntry, 
// 				int flags = eqFlagsAlmostAll ) const;
// 	virtual bool _copy( PilotAppCategory *toPalmEntry, PCEntry *fromPCEntry );
// 	virtual bool _copy( PCEntry *toPCEntry, PilotAppCategory *fromPalmEntry );
	virtual bool smartMergeEntry( PCEntry *pcEntry, PilotAppCategory *backupEntry, 
		PilotAppCategory *palmEntry );



protected:
	virtual VCalConduitSettings *config()=0;
	virtual const QString getTitle(PilotAppCategory*de)=0;
//	virtual PilotAppCategory *createPalmEntry( PilotRecord *rec ) = 0;
	void setCategory( KCal::Incidence *toIncidence, const PilotAppCategory *fromRecord );
	void setCategory( PilotAppCategory *toRecord, const KCal::Incidence *fromIncidence );

} ;

#endif
