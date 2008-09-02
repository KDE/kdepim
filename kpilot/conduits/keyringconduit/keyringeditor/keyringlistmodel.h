#ifndef KEYRINGLISTMODEL_H
#define KEYRINGLISTMODEL_H
/* keyringlistmodel.h			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>
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

#include <QAbstractListModel>

class KeyringHHDataProxy;
class KeyringHHRecord;

class KeyringListModel : public QAbstractListModel
{
public:
	/**
	 * KeyringListModel does not take over owner ship of proxy and does not delete
	 * the proxy or pointers to records when it gets deleted.
	 */
	explicit KeyringListModel( KeyringHHDataProxy* proxy, QObject *parent = 0 );
	
	/**
	 * Returns the number of rows under the given parent.
	 */
	int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
	
	/**
	 * Returns the data stored under the given role for the item referred to by
	 * the index.
	 */
	QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
	
	QVariant headerData ( int section, Qt::Orientation orientation
		, int role = Qt::DisplayRole ) const;
	
	bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() );
	
	bool addRecord( KeyringHHRecord* record );
	
	KeyringHHRecord *record( const QModelIndex & index );

private:
	QList<KeyringHHRecord*> fRecords;

};


#endif
