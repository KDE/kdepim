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

#ifndef QT_NO_DIRMODEL

#include <KDebug>
#include <KLocalizedString>
#include <KMessageBox>

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
#include <QPushButton>
#include <QHeaderView>

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
                        return QColor(c);
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
                kDebug() << "can't locate file " << file;
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

            for ( QHash<QString,VerifyChecksumsDialog::Status>::const_iterator it = oldStatusMap.constBegin(), end = oldStatusMap.constEnd() ; it != end ; ++it )
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

            QRect r;
            for( int i = 0; i < proxy.columnCount(); ++i )
                view.resizeColumnToContents( i );

            // define some minimum sizes
            view.header()->resizeSection( 0, qMax( view.header()->sectionSize( 0 ), 220 ) );
            view.header()->resizeSection( 1, qMax( view.header()->sectionSize( 1 ), 75 ) );
            view.header()->resizeSection( 2, qMax( view.header()->sectionSize( 2 ), 75 ) );
            view.header()->resizeSection( 3, qMax( view.header()->sectionSize( 3 ), 140 ) );

            for( int i = 0; i < proxy.rowCount(); ++i )
                r = r.united( view.visualRect( proxy.index( proxy.columnCount() - 1, i ) ) );
            view.setMinimumSize( QSize( qBound( r.width() + 4 * view.frameWidth(), 220+75+75+140 + 4 * view.frameWidth(), 1024 ), // 100 is the default defaultSectionSize
                                        qBound( r.height(), 220, 512 ) ) );
        }

        void setBase( const QString & base ) {
            label.setText( base );
            if ( QDirModel * fsm = qobject_cast<QDirModel*>( proxy.sourceModel() ) ) {
                view.setRootIndex( proxy.mapFromSource( fsm->index( base ) ) );
            } else {
                kWarning() << "expect a QDirModel-derived class as proxy.sourceModel(), got ";
                if ( !proxy.sourceModel() ) {
                    kWarning() << "a null pointer";
                } else {
                    kWarning() << proxy.sourceModel()->metaObject()->className();
                }
            }
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
          errors(),
          model(),
          ui( q )
    {
        qRegisterMetaType<Status>( "Kleo::Crypto::Gui::VerifyChecksumsDialog::Status" );
    }

private:
    void slotErrorButtonClicked() {
        KMessageBox::errorList( q, i18n("The following errors and warnings were recorded:"),
                                errors, i18n("Checksum Verification Errors") );
    }

private:
    void updateErrors() {
        const bool active = ui.isProgressBarActive();
        ui.progressLabel.setVisible(  active );
        ui.progressBar.  setVisible(  active );
        ui.errorLabel.   setVisible( !active );
        ui.errorButton.  setVisible( !active && !errors.empty() );
        if ( errors.empty() )
            ui.errorLabel.setText( i18n("No errors occurred" ) );
        else
            ui.errorLabel.setText( i18np( "One error occurred", "%1 errors occurred", errors.size() ) );
    }

private:
    QStringList bases;
    QStringList errors;
    ColorizedFileSystemModel model;

    struct UI {
        std::vector<BaseWidget*> baseWidgets;
        QLabel progressLabel;
        QProgressBar progressBar;
        QLabel errorLabel;
        QPushButton errorButton;
        QDialogButtonBox buttonBox;
        QVBoxLayout vlay;
        QHBoxLayout hlay[2];

        explicit UI( VerifyChecksumsDialog * q )
            : baseWidgets(),
              progressLabel( i18n("Progress:"), q ),
              progressBar( q ),
              errorLabel( i18n("No errors occurred"), q ),
              errorButton( i18nc("Show Errors","Show"), q ),
              buttonBox( QDialogButtonBox::Close, Qt::Horizontal, q ),
              vlay( q )
        {
            KDAB_SET_OBJECT_NAME( progressLabel );
            KDAB_SET_OBJECT_NAME( progressBar );
            KDAB_SET_OBJECT_NAME( errorLabel );
            KDAB_SET_OBJECT_NAME( errorButton );
            KDAB_SET_OBJECT_NAME( buttonBox );
            KDAB_SET_OBJECT_NAME( vlay );
            KDAB_SET_OBJECT_NAME( hlay[0] );
            KDAB_SET_OBJECT_NAME( hlay[1] );

            errorButton.setAutoDefault( false );

            hlay[0].addWidget( &progressLabel );
            hlay[0].addWidget( &progressBar, 1 );

            hlay[1].addWidget( &errorLabel, 1 );
            hlay[1].addWidget( &errorButton );

            vlay.addLayout( &hlay[0] );
            vlay.addLayout( &hlay[1] );
            vlay.addWidget( &buttonBox );

            errorLabel.hide();
            errorButton.hide();

            QPushButton * close = closeButton();

            connect( close, SIGNAL(clicked()), q, SIGNAL(canceled()) );
            connect( close, SIGNAL(clicked()), q, SLOT(accept()) );

            connect( &errorButton, SIGNAL(clicked()), q, SLOT(slotErrorButtonClicked()) );
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
        }

        bool isProgressBarActive() const {
            const int tot = progressBar.maximum();
            const int cur = progressBar.value();
            return !tot || cur != tot ;
        }

    } ui;
};

VerifyChecksumsDialog::VerifyChecksumsDialog( QWidget * parent, Qt::WindowFlags flags )
    : QDialog( parent, flags ),
      d( new Private( this ) )
{

}

VerifyChecksumsDialog::~VerifyChecksumsDialog() {}

// slot
void VerifyChecksumsDialog::setBaseDirectories( const QStringList & bases ) {
    if ( d->bases == bases )
        return;
    d->bases = bases;
    d->ui.setBases( bases, &d->model );
}

// slot
void VerifyChecksumsDialog::setErrors( const QStringList & errors ) {
    if ( d->errors == errors )
        return;
    d->errors = errors;
    d->updateErrors();
}

// slot
void VerifyChecksumsDialog::setProgress( int cur, int tot ) {
    d->ui.setProgress( cur, tot );
    d->updateErrors();
}

// slot
void VerifyChecksumsDialog::setStatus( const QString & file, Status status ) {
    d->model.setStatus( file, status );
}

// slot
void VerifyChecksumsDialog::clearStatusInformation() {
    d->errors.clear();
    d->updateErrors();
    d->model.clearStatusInformation();
}

#include "verifychecksumsdialog.moc"
#include "moc_verifychecksumsdialog.cpp"

#endif // QT_NO_DIRMODEL
