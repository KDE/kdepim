/* -*- mode: c++; c-basic-offset:4 -*-
    dialogs/lookupcertificatesdialog.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#include "lookupcertificatesdialog.h"

#include "ui_lookupcertificatesdialog.h"

#include <models/keylistmodel.h>
#include <models/keylistsortfilterproxymodel.h>

#include <utils/headerview.h>

#include <gpgme++/key.h>

#include <KLineEdit>
#include <KLocale>

#include <QTreeView>
#include <QLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QLabel>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Dialogs;
using namespace GpgME;

static const int minimalSearchTextLength = 2; // ### TODO: make that KIOSK-able

static const QHeaderView::ResizeMode resize_modes[AbstractKeyListModel::NumColumns] = {
    QHeaderView::Stretch,          // Name
    QHeaderView::Stretch, // EMail
    QHeaderView::Fixed, // Valid From
    QHeaderView::Fixed, // Valid Until
    QHeaderView::Fixed, // Details
    QHeaderView::Fixed, // Fingerprint
};

static void adjust_header( HeaderView * hv ) {
    for ( int i = 0, end = AbstractKeyListModel::NumColumns ; i < end ; ++i )
        hv->setSectionResizeMode( i, resize_modes[i] );
}

class LookupCertificatesDialog::Private {
    friend class ::Kleo::Dialogs::LookupCertificatesDialog;
    LookupCertificatesDialog * const q;
public:
    explicit Private( LookupCertificatesDialog * qq );
    ~Private();

private:
    void slotSelectionChanged() {
        enableDisableWidgets();
    }
    void slotSearchTextChanged() {
        enableDisableWidgets();
    }
    void slotSearchClicked() {
        emit q->searchTextChanged( ui.findED->text() );
    }
    void slotDetailsClicked() {
        assert( q->selectedCertificates().size() == 1 );
        emit q->detailsRequested( q->selectedCertificates().front() );
    }
    void slotSaveAsClicked() {
        emit q->saveAsRequested( q->selectedCertificates() );
    }

    void enableDisableWidgets();

    QString searchText() const { return ui.findED->text().trimmed(); }
    QModelIndexList selectedIndexes() const { return ui.resultTV->selectionModel()->selectedRows(); }
private:
    AbstractKeyListModel * model;
    KeyListSortFilterProxyModel proxy;
    bool passive;

    struct Ui : Ui_LookupCertificatesDialog {

        explicit Ui( LookupCertificatesDialog * q )
            : Ui_LookupCertificatesDialog()
        {
            setupUi( q );

            saveAsPB->hide(); // ### not yet implemented in LookupCertificatesCommand

            findED->setClearButtonShown( true );

            importPB()->setText( i18n("Import") );

            HeaderView * hv = new HeaderView( Qt::Horizontal );
            KDAB_SET_OBJECT_NAME( hv );
            resultTV->setHeader( hv );
            adjust_header( hv );

            connect( resultTV,   SIGNAL(itemDoubleClicked(QModelIndex)),
                     importPB(), SLOT(animateClick()) );
            connect( resultTV->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                     q, SLOT(slotSelectionChanged()) );

            findED->setFocus();
        }

        QPushButton * importPB() const { return buttonBox->button( QDialogButtonBox::Save ); }
        QPushButton * closePB()  const { return buttonBox->button( QDialogButtonBox::Close ); }
    } ui;
};

LookupCertificatesDialog::Private::Private( LookupCertificatesDialog * qq )
    : q( qq ),
      model( AbstractKeyListModel::createFlatKeyListModel() ),
      proxy(),
      passive( false ),
      ui( q )
{
    KDAB_SET_OBJECT_NAME( model );
    KDAB_SET_OBJECT_NAME( proxy );

    proxy.setSourceModel( model );
    ui.resultTV->setModel( &proxy );

    enableDisableWidgets();
}

LookupCertificatesDialog::Private::~Private() {}

LookupCertificatesDialog::LookupCertificatesDialog( QWidget * p, Qt::WindowFlags f )
    : QDialog( p, f ), d( new Private( this ) )
{

}

LookupCertificatesDialog::~LookupCertificatesDialog() {}

void LookupCertificatesDialog::setCertificates( const std::vector<Key> & certs ) {
    d->model->setKeys( certs );
}

std::vector<Key> LookupCertificatesDialog::selectedCertificates() const {
    return d->proxy.keys( d->selectedIndexes() );
}

void LookupCertificatesDialog::setPassive( bool on ) {
    if ( d->passive == on )
        return;
    d->passive = on;
    d->enableDisableWidgets();
}

bool LookupCertificatesDialog::isPassive() const {
    return d->passive;
}

void LookupCertificatesDialog::accept() {
    assert( !d->selectedIndexes().empty() );
    emit importRequested( selectedCertificates() );
    QDialog::accept();
}

void LookupCertificatesDialog::Private::enableDisableWidgets() {
    // enable/disable everything except 'close', based on passive:
    Q_FOREACH( QObject * const o, q->children() )
        if ( QWidget * const w = qobject_cast<QWidget*>( o ) )
            w->setDisabled( passive && w != ui.closePB() );

    if ( passive )
        return;

    ui.findPB->setEnabled( searchText().length() > minimalSearchTextLength );

    const QModelIndexList selection = selectedIndexes();

    ui.detailsPB->setEnabled(   selection.size() == 1 );
    ui.saveAsPB->setEnabled(    selection.size() == 1 );
    ui.importPB()->setEnabled( !selection.empty() );
}

#include "moc_lookupcertificatesdialog.cpp"

