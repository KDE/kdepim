/* -*- mode: c++; c-basic-offset:4 -*-
    models/keylistmodel.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/
#ifndef __KLEOPATRA_MODELS_KEYLISTMODEL_H__
#define __KLEOPATRA_MODELS_KEYLISTMODEL_H__

#include <QAbstractItemModel>

#include <vector>

namespace GpgME {
    class Key;
}

namespace Kleo {

    class AbstractKeyListModel : public QAbstractItemModel {
	Q_OBJECT
    public:
	explicit AbstractKeyListModel( QObject * parent=0 );
	~AbstractKeyListModel();

        static AbstractKeyListModel * createFlatKeyListModel( QObject * parent=0 );
        static AbstractKeyListModel * createHierarchicalKeyListModel( QObject * parent=0 );

        GpgME::Key key( const QModelIndex & idx ) const;
        std::vector<GpgME::Key> keys( const QList<QModelIndex> & indexes ) const;

        using QAbstractItemModel::index;
        QModelIndex index( const GpgME::Key & key ) const;
        QList<QModelIndex> indexes( const std::vector<GpgME::Key> & keys ) const;

        QModelIndex addKey( const GpgME::Key & key );
        QList<QModelIndex> addKeys( const std::vector<GpgME::Key> & keys );

        void clear();

	enum Columns {
	    PrettyName,
	    PrettyEMail,
	    ValidFrom,
	    ValidUntil,
	    TechnicalDetails,
	    Fingerprint,
#if 0
	    /* OpenPGP only, really */
	    LongKeyID,
	    ShortKeyID,
	    /* X509 only, really */
	    Issuer,
	    Subject,
	    SerialNumber,
#endif

	    NumColumns,
	    Icon = PrettyName // which column shall the icon be displayed in?
	};

	/* reimp */ int numColumns() const;
	/* reimp */ QVariant headerData( int section, Qt::Orientation o, Qt::ItemDataRole role=Qt::DisplayRole ) const;
	/* reimp */ QVariant data( const QModelIndex & index, Qt::ItemDataRole role=Qt::DisplayRole ) const;

    private:
        virtual GpgME::Key doMapToKey( const QModelIndex & index ) const = 0;
        virtual QModelIndex doMapFromKey( const GpgME::Key & key ) const = 0;
	virtual QList<QModelIndex> doAddKeys( const std::vector<GpgME::Key> & keys ) = 0;
    };

}

#endif /* __KLEOPATRA_MODELS_KEYLISTMODEL_H__ */
