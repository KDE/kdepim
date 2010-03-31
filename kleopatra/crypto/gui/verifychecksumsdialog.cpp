/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/verifychecksumsdialog.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2010 Klarälvdalens Datakonsult AB

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

#include <config-kleopatra.h>

#include "verifychecksumsdialog.h"

#include <KDebug>
#include <KLocalizedString>
#include <KPushButton>
#include <KStandardGuiItem>
#include <KUrl>

#include <QHBoxLayout>
#include <QLabel>
#include <QStringList>
#include <QVBoxLayout>
#include <QHash>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QDirModel>
#include <QProgressBar>
#include <QDialogButtonBox>

#include <boost/static_assert.hpp>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;
using namespace boost;

namespace {

    static Qt::GlobalColor statusColor[] = {
        Qt::color0,   // Unknown - nothing
        Qt::green,    // OK
        Qt::red,      // Failed
        Qt::darkRed,  // Error
    };
    BOOST_STATIC_ASSERT((sizeof statusColor/sizeof *statusColor == VerifyChecksumsDialog::NumStatii));

    class ColorizedFileSystemModel : public QDirModel {
        Q_OBJECT
    public:
        explicit ColorizedFileSystemModel( QObject * parent=0 )
            : QDirModel( parent ),
              statusMap()
        {

        }

        /* reimp */ QVariant data( const QModelIndex & mi, int role=Qt::DisplayRole ) const {
            if ( mi.isValid() && role == Qt::BackgroundRole ) {
                const QHash<QString,VerifyChecksumsDialog::Status>::const_iterator
                    it = statusMap.find( filePath( mi ) );
                if ( it != statusMap.end() )
                    if ( const Qt::GlobalColor c = statusColor[*it] )
                        return c;
            }
            return QDirModel::data( mi, role );
        }

    public Q_SLOTS:
        void setStatus( const QString & file, VerifyChecksumsDialog::Status status ) {

            if ( status >= VerifyChecksumsDialog::NumStatii || file.isEmpty() )
                return;

            // canonicalize filename:
            const QModelIndex mi = index( file );
            const QString canonical = filePath( mi );
            if ( canonical.isEmpty() ) {
                qDebug() << Q_FUNC_INFO << ": can't locate file" << file;
                return;
            }

            const QHash<QString,VerifyChecksumsDialog::Status>::iterator
                it = statusMap.find( canonical );

            if ( it != statusMap.end() )
                if ( *it == status )
                    return; // nothing to do
                else
                    *it = status;
            else
                statusMap[canonical] = status;

            emitDataChangedFor( mi );
        }

        void clearStatusInformation() {
            using std::swap;

            QHash<QString,VerifyChecksumsDialog::Status> oldStatusMap;
            swap( statusMap, oldStatusMap );

            for ( QHash<QString,VerifyChecksumsDialog::Status>::const_iterator it = oldStatusMap.begin(), end = oldStatusMap.end() ; it != end ; ++it )
                emitDataChangedFor( it.key() );
        }

    private:
        void emitDataChangedFor( const QString & file ) {
            emitDataChangedFor( index( file ) );
        }
        void emitDataChangedFor( const QModelIndex & mi ) {
            const QModelIndex p = parent( mi );
            emit dataChanged( index( mi.row(), 0, p ), index( mi.row(), columnCount( p ) - 1, p ) );
        }

    private:
        QHash<QString,VerifyChecksumsDialog::Status> statusMap;
    };


    static int find_layout_item( const QBoxLayout & blay ) {
        for ( int i = 0, end = blay.count() ; i < end ; ++i )
            if ( QLayoutItem * item = blay.itemAt( i ) )
                if ( item->layout() )
                    return i;
        return 0;
    }

    struct BaseWidget {
        QSortFilterProxyModel proxy;
        QLabel label;
        QTreeView view;

        BaseWidget( QDirModel * model, QWidget * parent, QVBoxLayout * vlay )
            : proxy(),
              label( parent ),
              view( parent )
        {
            KDAB_SET_OBJECT_NAME( proxy );
            KDAB_SET_OBJECT_NAME( label );
            KDAB_SET_OBJECT_NAME( view );

            const int row = find_layout_item( *vlay );
            vlay->insertWidget( row,   &label );
            vlay->insertWidget( row+1, &view, 1 );

            proxy.setSourceModel( model );

            view.setModel( &proxy );
        }

        void setBase( const QString & base ) {
            label.setText( base );
            if ( QDirModel * fsm = qobject_cast<QDirModel*>( proxy.sourceModel() ) )
                view.setRootIndex( /*proxy.mapFromSource*/( fsm->index( base ) ) );
            else
                qWarning( "%s: expect a QDirModel-derived class as proxy.sourceModel(), got %s",
                          Q_FUNC_INFO, proxy.sourceModel() ? proxy.sourceModel()->metaObject()->className() : "null pointer" );
        }
    };

} // anon namespace


class VerifyChecksumsDialog::Private {
    friend class ::Kleo::Crypto::Gui::VerifyChecksumsDialog;
    VerifyChecksumsDialog * const q;
public:
    explicit Private( VerifyChecksumsDialog * qq )
        : q( qq ),
          bases(),
          model(),
          ui( q )
    {
        qRegisterMetaType<Status>( "Kleo::Crypto::Gui::VerifyChecksumsDialog::Status" );
    }

private:
    QStringList bases;
    ColorizedFileSystemModel model;
    
    struct UI {
        std::vector<BaseWidget*> baseWidgets;
        QLabel progressLabel;
        QProgressBar progressBar;
        QDialogButtonBox buttonBox;
        QVBoxLayout vlay;
        QHBoxLayout hlay;

        explicit UI( VerifyChecksumsDialog * q )
            : baseWidgets(),
              progressLabel( i18n("Progress:"), q ),
              progressBar( q ),
              buttonBox( QDialogButtonBox::Close, Qt::Horizontal, q ),
              vlay( q ),
              hlay()
        {
            KDAB_SET_OBJECT_NAME( progressLabel );
            KDAB_SET_OBJECT_NAME( progressBar );
            KDAB_SET_OBJECT_NAME( buttonBox );
            KDAB_SET_OBJECT_NAME( vlay );
            KDAB_SET_OBJECT_NAME( hlay );

            hlay.addWidget( &progressLabel );
            hlay.addWidget( &progressBar, 1 );

            vlay.addLayout( &hlay );
            vlay.addWidget( &buttonBox );

            QPushButton * close = closeButton();

            connect( close, SIGNAL(clicked()), q, SIGNAL(canceled()) );
            connect( close, SIGNAL(clicked()), q, SLOT(accept()) );
        }

        ~UI() {
            qDeleteAll( baseWidgets );
        }

        QPushButton * closeButton() const {
            return buttonBox.button( QDialogButtonBox::Close );
        }

        void setBases( const QStringList & bases, QDirModel * model ) {

            // create new BaseWidgets:
            for ( unsigned int i = baseWidgets.size(), end = bases.size() ; i < end ; ++i )
                baseWidgets.push_back( new BaseWidget( model, vlay.parentWidget(), &vlay ) );

            // shed surplus BaseWidgets:
            for ( unsigned int i = bases.size(), end = baseWidgets.size() ; i < end ; ++i ) {
                delete baseWidgets.back();
                baseWidgets.pop_back();
            }

            assert( static_cast<unsigned>( bases.size() ) == baseWidgets.size() );

            // update bases:
            for ( unsigned int i = 0 ; i < baseWidgets.size() ; ++i )
                baseWidgets[i]->setBase( bases[i] );

        }

        void setProgress( int cur, int tot ) {
            progressBar.setMaximum( tot );
            progressBar.setValue( cur );
            progressBar.setVisible( !tot || cur != tot );
            progressLabel.setVisible( !tot || cur != tot );
        }

    } ui;
};

VerifyChecksumsDialog::VerifyChecksumsDialog( QWidget * parent, Qt::WindowFlags flags )
    : QDialog( parent, flags ),
      d( new Private( this ) )
{

}

VerifyChecksumsDialog::~VerifyChecksumsDialog() {}

void VerifyChecksumsDialog::setBaseDirectories( const QStringList & bases ) {
    if ( d->bases == bases )
        return;
    d->bases = bases;
    d->ui.setBases( bases, &d->model );
}

// slot
void VerifyChecksumsDialog::setProgress( int cur, int tot ) {
    d->ui.setProgress( cur, tot );
}

// slot
void VerifyChecksumsDialog::setStatus( const QString & file, Status status ) {
    d->model.setStatus( file, status );
}

// slot
void VerifyChecksumsDialog::clearStatusInformation() {
    d->model.clearStatusInformation();
}

#include "verifychecksumsdialog.moc"
#include "moc_verifychecksumsdialog.cpp"
