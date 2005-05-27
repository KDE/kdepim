#ifndef _KPILOT_PILOTAPPCATEGORY_H
#define _KPILOT_PILOTAPPCATEGORY_H
/* pilotAppCategory.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** The class PilotAppCategory is the base class for "interpretations"
** of a PilotRecord. This is where the records change from a collection
** of bits to something with meaning. Subclasses of PilotAppCategory
** give specific meaning to records from specific databases.
**
** Almost everything is inline; as a crufty hack, the non-inline
** part of this class lives in pilotRecord.cc.
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

// #include <pi-macros.h>

#include <qstring.h>
#include <pi-appinfo.h>

#include "pilotRecord.h"

class QTextCodec;

class KDE_EXPORT PilotAppCategory : public PilotRecordBase
{
protected:			// Use protected since we will be subclassed
	/**
	* Pack whatever data the interpreted record holds into the given
	* buffer, of length @p size; return NULL to indicate failure,
	* otherwise @p buf. Set @p size to the actual size of data returned.
	* (all of this is dictated by the pilot-link interfaces).
	*/
	virtual void *pack_(void *buf, int *size) = 0;
	virtual void unpack(const void *, int = 0) = 0;


public:
	PilotAppCategory(int a=0, recordid_t i=0, int c=0) :
		PilotRecordBase(a,c,i)
	{} ;

	PilotAppCategory(const PilotRecord* rec) :
		PilotRecordBase( ((rec)?rec->attributes():0),
			((rec)?rec->id():0),
			((rec)?rec->category():0) )
	{} ;

	PilotAppCategory(const PilotAppCategory &copyFrom) :
		PilotRecordBase(copyFrom.attributes(),
			copyFrom.id(),
			copyFrom.category() )
	{} ;

	PilotAppCategory& operator=( const PilotAppCategory &r )
	{
		setAttributes( r.attributes() );
		setID( r.id() );
		setCategory( r.category() );
		return *this;
	} ;

	bool operator==(const PilotAppCategory &compareTo)
	{
		return (attributes() ==compareTo.attributes() &&
			id() ==compareTo.id() &&
			category() ==compareTo.category() );
	} ;

	virtual ~PilotAppCategory(void) {};

	/** @return a PilotRecord that contains all of the info of the
	*  subclass.  Remember to delete the PilotRecord when finished.
	*/
	virtual PilotRecord* pack();

	virtual QString getTextRepresentation(bool=false) {return i18n("Unknown record type");};

	void setCategory( int c) { return PilotRecordBase::setCategory(c); }
	bool setCategory(struct CategoryAppInfo &info,const QString &label);
	bool setCat(struct CategoryAppInfo &info,const QString &label) KDE_DEPRECATED
		{ return setCategory(info,label); }

protected:
	static QTextCodec *pilotCodec;
public:
	static QTextCodec *codec()
		{ if (pilotCodec) return pilotCodec; else return setupPilotCodec(QString::null); } ;
	static QTextCodec *setupPilotCodec(const QString &);
	static QString codecName();
};



#endif
