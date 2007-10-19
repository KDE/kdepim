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

#include "decryptverifyoperationwidget.h"

#include <utils/filenamerequester.h>

#include <KLocale>
#include <KIcon>

#include <QLayout>
#include <QLabel>
#include <QCheckBox>
#include <QToolButton>

using namespace Kleo;

class DecryptVerifyOperationWidget::Private {
    friend class ::Kleo::DecryptVerifyOperationWidget;
    DecryptVerifyOperationWidget * const q;
public:
    explicit Private( DecryptVerifyOperationWidget * qq );
    ~Private();

private:
    void slotSwap();

private:
    struct UI {
        QGridLayout glay;
        QLabel       inputLB, inputFileNameLB;
        QHBoxLayout  hlay;
        QCheckBox     verifyDetachedCB;
        QToolButton   swapTB;
        QLabel       signedDataLB;
        FileNameRequester signedDataFNR;

        explicit UI( DecryptVerifyOperationWidget * q );
    } ui;
};

DecryptVerifyOperationWidget::Private::UI::UI( DecryptVerifyOperationWidget * q )
    : glay( q ),
      inputLB( i18n("Input file:"), q ),
      inputFileNameLB( q ),
      hlay(),
      verifyDetachedCB( i18n("&Input file is a detached signature"), q ),
      swapTB( q ),
      signedDataLB( i18n("&Signed data:"), q ),
      signedDataFNR( q )
{
    KDAB_SET_OBJECT_NAME( glay );
    KDAB_SET_OBJECT_NAME( inputLB );
    KDAB_SET_OBJECT_NAME( inputFileNameLB );
    KDAB_SET_OBJECT_NAME( hlay );
    KDAB_SET_OBJECT_NAME( verifyDetachedCB );
    KDAB_SET_OBJECT_NAME( swapTB );
    KDAB_SET_OBJECT_NAME( signedDataLB );
    KDAB_SET_OBJECT_NAME( signedDataFNR );

    swapTB.setIcon( KIcon("swap") );
    swapTB.setEnabled( false );
    signedDataLB.setEnabled( false );
    signedDataFNR.setEnabled( false );
    signedDataLB.setBuddy( &signedDataFNR );

    glay.addWidget( &inputLB, 0, 0 );
    glay.addWidget( &inputFileNameLB, 0, 1 );

    glay.addLayout( &hlay, 1, 0, 1, 2 );
    hlay.addWidget( &verifyDetachedCB );
    hlay.addWidget( &swapTB );

    glay.addWidget( &signedDataLB, 2, 0 );
    glay.addWidget( &signedDataFNR, 2, 1 );

    connect( &verifyDetachedCB, SIGNAL(toggled(bool)),
             &swapTB, SLOT(setEnabled(bool)) );
    connect( &verifyDetachedCB, SIGNAL(toggled(bool)),
             &signedDataLB, SLOT(setEnabled(bool)) );
    connect( &verifyDetachedCB, SIGNAL(toggled(bool)),
             &signedDataFNR, SLOT(setEnabled(bool)) );
}


DecryptVerifyOperationWidget::Private::Private( DecryptVerifyOperationWidget * qq )
    : q( qq ),
      ui( q )
{
    connect( &ui.swapTB, SIGNAL(clicked()), q, SLOT(slotSwap()) );
}

DecryptVerifyOperationWidget::Private::~Private() {}

DecryptVerifyOperationWidget::DecryptVerifyOperationWidget( QWidget * p )
    : QWidget( p ), d( new Private( this ) )
{

}

DecryptVerifyOperationWidget::~DecryptVerifyOperationWidget() {}

void DecryptVerifyOperationWidget::setVerifyDetached( bool on ) {
    d->ui.verifyDetachedCB.setChecked( on );
}

bool DecryptVerifyOperationWidget::isVerifyDetached() const {
    return d->ui.verifyDetachedCB.isChecked();
}

void DecryptVerifyOperationWidget::setInputFileName( const QString & name ) {
    d->ui.inputFileNameLB.setText( name );
}

QString DecryptVerifyOperationWidget::inputFileName() const {
    return d->ui.inputFileNameLB.text();
}

void DecryptVerifyOperationWidget::setSignedDataFileName( const QString & name ) {
    d->ui.signedDataFNR.setFileName( name );
}

QString DecryptVerifyOperationWidget::signedDataFileName() const {
    return d->ui.signedDataFNR.fileName();
}

void DecryptVerifyOperationWidget::Private::slotSwap() {
    const QString tmp = ui.signedDataFNR.fileName();
    ui.signedDataFNR.setFileName( ui.inputFileNameLB.text() );
    ui.inputFileNameLB.setText( tmp );
}

#include "moc_decryptverifyoperationwidget.cpp"
