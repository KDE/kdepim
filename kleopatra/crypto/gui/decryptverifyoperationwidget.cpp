/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/decryptverifyoperationwidget.cpp

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

#include <config-kleopatra.h>

#include "decryptverifyoperationwidget.h"

#include <utils/archivedefinition.h>

#include "libkleo/ui/filenamerequester.h"

#include <KLocalizedString>

#include <QLabel>
#include <QCheckBox>
#include <QStackedLayout>
#include <QComboBox>

#include <boost/shared_ptr.hpp>

using namespace Kleo;
using namespace Kleo::Crypto::Gui;
using namespace boost;

class DecryptVerifyOperationWidget::Private {
    friend class ::Kleo::Crypto::Gui::DecryptVerifyOperationWidget;
    DecryptVerifyOperationWidget * const q;
public:
    explicit Private( DecryptVerifyOperationWidget * qq );
    ~Private();

    void enableDisableWidgets() {
        const bool detached = ui.verifyDetachedCB.isChecked();
        const bool archive  = ui.archiveCB.isChecked();
        ui.archiveCB.setEnabled( !detached );
        ui.archivesCB.setEnabled( archive && !detached );
    }
private:
    struct UI {
        QGridLayout   glay;
        QLabel         inputLB;
        QStackedLayout inputStack;
        QLabel            inputFileNameLB;
        FileNameRequester inputFileNameRQ;
        //------
        QCheckBox      verifyDetachedCB;
        //------
        QLabel         signedDataLB;
        QStackedLayout signedDataStack;
        QLabel            signedDataFileNameLB;
        FileNameRequester signedDataFileNameRQ;
        //------
        QHBoxLayout    hlay;
        QCheckBox       archiveCB;
        QComboBox       archivesCB;

        explicit UI( DecryptVerifyOperationWidget * q );
    } ui;
};

DecryptVerifyOperationWidget::Private::UI::UI( DecryptVerifyOperationWidget * q )
    : glay( q ),
      inputLB( i18n("Input file:"), q ),
      inputStack(),
      inputFileNameLB( q ),
      inputFileNameRQ( q ),
      verifyDetachedCB( i18n("&Input file is a detached signature"), q ),
      signedDataLB( i18n("&Signed data:"), q ),
      signedDataStack(),
      signedDataFileNameLB( q ),
      signedDataFileNameRQ( q ),
      hlay(),
      archiveCB( i18n("&Input file is an archive; unpack with:"), q ),
      archivesCB( q )
{
    KDAB_SET_OBJECT_NAME( glay );
    KDAB_SET_OBJECT_NAME( inputLB );
    KDAB_SET_OBJECT_NAME( inputStack );
    KDAB_SET_OBJECT_NAME( inputFileNameLB );
    KDAB_SET_OBJECT_NAME( inputFileNameRQ );
    KDAB_SET_OBJECT_NAME( verifyDetachedCB );
    KDAB_SET_OBJECT_NAME( signedDataLB );
    KDAB_SET_OBJECT_NAME( signedDataStack );
    KDAB_SET_OBJECT_NAME( signedDataFileNameLB );
    KDAB_SET_OBJECT_NAME( signedDataFileNameRQ );
    KDAB_SET_OBJECT_NAME( hlay );
    KDAB_SET_OBJECT_NAME( archiveCB );
    KDAB_SET_OBJECT_NAME( archivesCB );

    inputStack.setMargin( 0 );
    signedDataStack.setMargin( 0 );

    signedDataLB.setEnabled( false );
    signedDataFileNameLB.setEnabled( false );
    signedDataFileNameRQ.setEnabled( false );
    archivesCB.setEnabled( false );

    glay.setMargin( 0 );
    glay.addWidget( &inputLB, 0, 0 );
    glay.addLayout( &inputStack, 0, 1 );
    inputStack.addWidget( &inputFileNameLB );
    inputStack.addWidget( &inputFileNameRQ );

    glay.addWidget( &verifyDetachedCB, 1, 0, 1, 2 );

    glay.addWidget( &signedDataLB, 2, 0 );
    glay.addLayout( &signedDataStack, 2, 1 );
    signedDataStack.addWidget( &signedDataFileNameLB );
    signedDataStack.addWidget( &signedDataFileNameRQ );

    glay.addLayout( &hlay, 3, 0, 1, 2 );
    hlay.addWidget( &archiveCB );
    hlay.addWidget( &archivesCB, 1 );

    connect(&verifyDetachedCB, &QCheckBox::toggled, &signedDataLB, &QLabel::setEnabled);
    connect(&verifyDetachedCB, &QCheckBox::toggled, &signedDataFileNameLB, &QLabel::setEnabled);
    connect(&verifyDetachedCB, &QCheckBox::toggled, &signedDataFileNameRQ, &FileNameRequester::setEnabled);
    connect( &verifyDetachedCB, SIGNAL(toggled(bool)), q, SLOT(enableDisableWidgets()) );
    connect( &archiveCB, SIGNAL(toggled(bool)), q, SLOT(enableDisableWidgets()) );
}


DecryptVerifyOperationWidget::Private::Private( DecryptVerifyOperationWidget * qq )
    : q( qq ),
      ui( q )
{

}

DecryptVerifyOperationWidget::Private::~Private() {}

DecryptVerifyOperationWidget::DecryptVerifyOperationWidget( QWidget * p )
    : QWidget( p ), d( new Private( this ) )
{
    setMode( DecryptVerifyOpaque );
}

DecryptVerifyOperationWidget::~DecryptVerifyOperationWidget() {}

void DecryptVerifyOperationWidget::setArchiveDefinitions( const std::vector< shared_ptr<ArchiveDefinition> > & archiveDefinitions ) {
    d->ui.archivesCB.clear();
    Q_FOREACH( const shared_ptr<ArchiveDefinition> & ad, archiveDefinitions )
        d->ui.archivesCB.addItem( ad->label(), qVariantFromValue( ad ) );
}

static const int Mutable = 1;
static const int Const   = 0;

void DecryptVerifyOperationWidget::setMode( Mode mode ) {
    setMode( mode, shared_ptr<ArchiveDefinition>() );
}

void DecryptVerifyOperationWidget::setMode( Mode mode, const shared_ptr<ArchiveDefinition> & ad  ) {
    d->ui.verifyDetachedCB.setChecked( mode != DecryptVerifyOpaque );

    QWidget * inputWidget;
    QWidget * signedDataWidget;
    if ( mode == VerifyDetachedWithSignedData ) {
        inputWidget      = &d->ui.inputFileNameRQ;
        signedDataWidget = &d->ui.signedDataFileNameLB;
    } else {
        inputWidget      = &d->ui.inputFileNameLB;
        signedDataWidget = &d->ui.signedDataFileNameRQ;
    }

    d->ui.inputStack.setCurrentWidget( inputWidget );
    d->ui.signedDataStack.setCurrentWidget( signedDataWidget );

    d->ui.inputLB.setBuddy( inputWidget );
    d->ui.signedDataLB.setBuddy( signedDataWidget );

    d->ui.archiveCB.setChecked( ad.get() != 0 );
    for ( int i = 0, end = d->ui.archivesCB.count() ; i != end ; ++i )
        if ( ad == d->ui.archivesCB.itemData( i ).value< shared_ptr<ArchiveDefinition> >() ) {
            d->ui.archivesCB.setCurrentIndex( i );
            return;
        }
}

DecryptVerifyOperationWidget::Mode DecryptVerifyOperationWidget::mode() const {
    if ( d->ui.verifyDetachedCB.isChecked() )
        if ( d->ui.inputStack.currentIndex() == Const )
            return VerifyDetachedWithSignature;
        else
            return VerifyDetachedWithSignedData;
    else
        return DecryptVerifyOpaque;
}

void DecryptVerifyOperationWidget::setInputFileName( const QString & name ) {
    d->ui.inputFileNameLB.setText( name );
    d->ui.inputFileNameRQ.setFileName( name );
}

QString DecryptVerifyOperationWidget::inputFileName() const {
    if ( d->ui.inputStack.currentIndex() == Const )
        return d->ui.inputFileNameLB.text();
    else
        return d->ui.inputFileNameRQ.fileName();
}

void DecryptVerifyOperationWidget::setSignedDataFileName( const QString & name ) {
    d->ui.signedDataFileNameLB.setText( name );
    d->ui.signedDataFileNameRQ.setFileName( name );
}

QString DecryptVerifyOperationWidget::signedDataFileName() const {
    if ( d->ui.signedDataStack.currentIndex() == Const )
        return d->ui.signedDataFileNameLB.text();
    else
        return d->ui.signedDataFileNameRQ.fileName();
}

shared_ptr<ArchiveDefinition> DecryptVerifyOperationWidget::selectedArchiveDefinition() const {
    if ( mode() == DecryptVerifyOpaque && d->ui.archiveCB.isChecked() )
        return d->ui.archivesCB.itemData( d->ui.archivesCB.currentIndex() ).value< shared_ptr<ArchiveDefinition> >();
    else
        return shared_ptr<ArchiveDefinition>();
}

#include "moc_decryptverifyoperationwidget.cpp"
