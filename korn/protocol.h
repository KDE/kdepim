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

class QGroupBox;
class QObject;
class QStringList;
class QWidget;

template< class T> class QPtrList;
template< class T> class QPtrVector;
template< class T, class S> class QMap;

#include <qstring.h>

class Protocol
{
public:
	Protocol() {}
	virtual ~Protocol() {}

	virtual const Protocol* getProtocol( KConfigGroup* ) const = 0;
	virtual KMailDrop* createMaildrop( KConfigGroup* ) const = 0;
	virtual QMap< QString, QString > * createConfig( KConfigGroup* config, const QString& password ) const = 0;
	virtual QString configName() const { return "not specified"; }

	virtual void configFillGroupBoxes( QStringList* ) const = 0;
	virtual void configFields( QPtrVector< QWidget >* vector, const QObject*, QPtrList< AccountInput >* ) const = 0;
	virtual void readEntries( QMap< QString, QString >* ) const = 0;
	virtual void writeEntries( QMap< QString, QString >* ) const = 0;

	virtual unsigned short defaultPort( bool ) const { return 0; }

	//Functions that return a derived class.
	//This way, no explicit cast is needed
	virtual const KIO_Protocol* getKIOProtocol() const { return 0; }
};

#endif //PROTOCOL_H

