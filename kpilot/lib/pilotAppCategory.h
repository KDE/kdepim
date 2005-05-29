#ifndef _KPILOT_PILOTAPPCATEGORY_H
#define _KPILOT_PILOTAPPCATEGORY_H
/* pilotAppCategory.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
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

/**
* @file The class PilotAppCategory is the base class for "interpretations"
* of a PilotRecord. This is where the records change from a collection
* of bits to something with meaning. Subclasses of PilotAppCategory
* give specific meaning to records from specific databases.
*
* Almost everything is inline; as a crufty hack, the non-inline
* part of this class lives in pilotRecord.cc.
*/

#include <qstring.h>
#include <pi-appinfo.h>

#include "pilotRecord.h"

class QTextCodec;

/**
* Base class for interpretations of the binary blobs. Also
* exposes the common characteristics of all entries (in general,
* these characteristics are copied from the binary blob that
* this interpretation represents).
*
* Since this is the base of interpretation of the binary blobs,
* we include codec() here, which is used to translate from and to
* the handheld native (8 bit) encoding.
*/
class KDE_EXPORT PilotAppCategory : public PilotRecordBase
{
protected:			// Use protected since we will be subclassed
	/**
	* Pack whatever data the interpreted record holds into the given
	* buffer, of length @p size; return NULL to indicate failure,
	* otherwise @p buf. Set @p size to the actual size of data returned.
	* (all of this is dictated by the pilot-link interfaces).
	*
	* Subclasses must reimplement this to give a @em specific
	* meaning to the binary blob.
	*
	* @param buf Data buffer containing the blob.
	* @param size Size of the buffer (in bytes). As input, the maximum
	*        size of the buffer. As output, the number of bytes used.
	*/
	virtual void *pack_(void *buf, int *size) = 0;

	/** Unpack the binary blob @p buf of size @p size into
	* some structure with meaning.
	*/
	virtual void unpack(const void *buf, int size) = 0;


public:
	/** Constructor with no data. Use the indicated values
	* of the characteristics. @em Note that the order of the
	* parameters is subtly different from that in PilotRecordBase .
	*/
	PilotAppCategory(int a=0, recordid_t i=0, int c=0) :
		PilotRecordBase(a,c,i)
	{} ;

	/** Constructor. Use the common characteristics values from the
	* given record @p rec.
	*/
	PilotAppCategory(const PilotRecord* rec) :
		PilotRecordBase( ((rec)?rec->attributes():0),
			((rec)?rec->id():0),
			((rec)?rec->category():0) )
	{} ;

	/** Copy constructor. */
	PilotAppCategory(const PilotAppCategory &copyFrom) :
		PilotRecordBase(copyFrom.attributes(),
			copyFrom.id(),
			copyFrom.category() )
	{} ;

	/** Assignment operator. I rather doubt that this is useful. */
	PilotAppCategory& operator=( const PilotAppCategory &r )
	{
		setAttributes( r.attributes() );
		setID( r.id() );
		setCategory( r.category() );
		return *this;
	} ;

	/** Comparison operator. Not really useful, since it also
	* wants the same record ID.
	*/
	bool operator==(const PilotAppCategory &compareTo)
	{
		return (attributes() ==compareTo.attributes() &&
			id() ==compareTo.id() &&
			category() ==compareTo.category() );
	} ;

	/** Destructor. VIrtual, since we will be subclassed. */
	virtual ~PilotAppCategory(void) {};

	/** @return a PilotRecord that contains all of the info of the
	*  subclass.  Remember to delete the PilotRecord when finished.
	* Calls pack_() to get the work done.
	*/
	virtual PilotRecord* pack();

	/** Returns a text representation of this (interpreted) data.
	* The text may use Qt rich text tags if @p rt is true. The
	* default implementation just returns a junk message.
	*
	* @param rt Use right text (if needed) if and only if @p rt is true.
	*/
	virtual QString getTextRepresentation(bool rt=false)
		{ Q_UNUSED(rt); return i18n("Unknown record type"); }

	/** Sets the category number to @p c (between 0 and 15). */
	void setCategory( int c ) { return PilotRecordBase::setCategory(c); }
	/** Sets the category number by looking up the string @p label
	* in the category table @p info . Sets the category to 0 (unfiled)
	* if no match is found.
	*
	* @param info AppInfo structure containing the labels (in handheld
	*        native encoding).
	* @param label The label to look for.
	*/
	bool setCategory(struct CategoryAppInfo &info,const QString &label);
	bool KDE_DEPRECATED setCat(struct CategoryAppInfo &info,const QString &label)
		{ return setCategory(info,label); }

protected:
	static QTextCodec *pilotCodec;
public:
	/** Static translaion function that maps handheld native (8 bit,
	* usually latin1 but sometimes someting else) encoded data to
	* a Unicode string. Converts the @p len characters in @p c
	* to a Unicode string.
	*/
	static QString fromPilot( const char *c, int len );

	/** Static translation function that maps a QString onto the
	* native 8 bit encoding of the handheld. Writes the result into
	* the buffer @p buf which has size @p len. Returns the length
	* of the result. Zero-fills the buffer as needed.
	*/
	static int toPilot( const QString &s, char *buf, int len);

	/** Get the codec for use in translating strings from handheld
	* native encoding to QString and vice-versa.
	*/
	static QTextCodec *codec()
		{ if (pilotCodec) return pilotCodec; else return setupPilotCodec(QString::null); } ;
	/** Create a codec for translating handheld native 8 bit to Unicode,
	* using the given codec @p name -- this will often be latin1, but
	* might be something else for, say, Russian-language Pilots.
	* If @p name is empty, use latin1.
	*/
	static QTextCodec *setupPilotCodec(const QString &name);
	/** Returns the name of the codec being used. */
	static QString codecName();
};



#endif
