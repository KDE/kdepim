/* -*- mode: c++; c-basic-offset:4 -*-
    models/subkeylistmodel.h

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
#ifndef __KLEOPATRA_MODELS_SUBKEYLISTMODEL_H__
#define __KLEOPATRA_MODELS_SUBKEYLISTMODEL_H__

#include <QAbstractTableModel>

#include <vector>

#include <utils/pimpl_ptr.h>

namespace GpgME {
    class Key;
    class Subkey;
}

namespace Kleo {

    class SubkeyListModel : public QAbstractTableModel {
        Q_OBJECT
    public:
        explicit SubkeyListModel( QObject * parent=0 );
        ~SubkeyListModel();

        GpgME::Key key() const;

        enum Columns {
            ID,
            Type,
            ValidFrom,
            ValidUntil,
            Status,
            Bits,

            NumColumns,
            Icon = ID // which column shall the icon be displayed in?
        };

        GpgME::Subkey subkey( const QModelIndex & idx ) const;
        std::vector<GpgME::Subkey> subkeys( const QList<QModelIndex> & indexes ) const;

        using QAbstractTableModel::index;
        QModelIndex index( const GpgME::Subkey & subkey, int col=0 ) const;
        QList<QModelIndex> indexes( const std::vector<GpgME::Subkey> & subkeys ) const;

    public Q_SLOTS:
        void setKey( const GpgME::Key & key );
        void clear();

    public:
        /* reimp */ int columnCount( const QModelIndex & pidx=QModelIndex() ) const;
        /* reimp */ int rowCount( const QModelIndex & pidx=QModelIndex() ) const;
        /* reimp */ QVariant headerData( int section, Qt::Orientation o, int role=Qt::DisplayRole ) const;
        /* reimp */ QVariant data( const QModelIndex & index, int role=Qt::DisplayRole ) const;

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
    };

}

#endif /* __KLEOPATRA_MODELS_SUBKEYLISTMODEL_H__ */
