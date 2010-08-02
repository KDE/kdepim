/*
 * Copyright (C) 2005, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#ifndef PROTOCOL_H
#define PROTOCOL_H

class AccountInput;
class KConfigGroup;
class KIO_Protocol;
class KMailDrop;

class TQGroupBox;
class TQObject;
class TQStringList;
class TQWidget;

template< class T> class TQPtrList;
template< class T> class TQPtrVector;
template< class T, class S> class TQMap;

#include <tqstring.h>

class Protocol
{
public:
	Protocol() {}
	virtual ~Protocol() {}

	virtual const Protocol* getProtocol( KConfigGroup* ) const = 0;
	virtual KMailDrop* createMaildrop( KConfigGroup* ) const = 0;
	virtual TQMap< TQString, TQString > * createConfig( KConfigGroup* config, const TQString& password ) const = 0;
	virtual TQString configName() const { return "not specified"; }

	virtual void configFillGroupBoxes( TQStringList* ) const = 0;
	virtual void configFields( TQPtrVector< TQWidget >* vector, const TQObject*, TQPtrList< AccountInput >* ) const = 0;
	virtual void readEntries( TQMap< TQString, TQString >* ) const = 0;
	virtual void writeEntries( TQMap< TQString, TQString >* ) const = 0;

	virtual unsigned short defaultPort( bool ) const { return 0; }

	//Functions that return a derived class.
	//This way, no explicit cast is needed
	virtual const KIO_Protocol* getKIOProtocol() const { return 0; }
};

#endif //PROTOCOL_H

