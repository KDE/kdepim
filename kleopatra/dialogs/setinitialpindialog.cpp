/* -*- mode: c++; c-basic-offset:4 -*-
    dialogs/setinitialpindialog.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009 Klarälvdalens Datakonsult AB

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

#include "setinitialpindialog.h"

#include "ui_setinitialpindialog.h"

#include <KIconLoader>
#include <KLocalizedString>
#include <KIcon>

#include <QTextDocument> // for Qt::escape

#include <gpgme++/error.h>

#include <boost/static_assert.hpp>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Dialogs;
using namespace GpgME;

enum State {
    Unknown = 0,
    NotSet,
    AlreadySet,
    Ongoing,
    Ok,
    Failed,
    NumStates
};

const char * icons[] = {
    // PENDING(marc) use better icons, once available
    "",                          // Unknown
    "",                          // NotSet
    "security-medium",           // AlreadySet
    "movie-process-working-kde", // Ongoing
    "security-high",             // Ok
    "security-low",              // Failed
};

BOOST_STATIC_ASSERT(( sizeof icons / sizeof (*icons) == NumStates ));
BOOST_STATIC_ASSERT(( sizeof("movie-") == 7 ));

static void update_widget( State state, bool delay, QLabel * resultLB, QLabel * lb, QPushButton * pb, QLabel * statusLB ) {
    assert( state >= 0 ); assert( state < NumStates );
    const char * icon = icons[state];
    if ( qstrncmp( icon, "movie-", sizeof("movie-")-1 ) == 0 )
        resultLB->setMovie( KIconLoader::global()->loadMovie( QLatin1String(icon+sizeof("movie-")), KIconLoader::NoGroup ) );
    else if ( icon && *icon )
        resultLB->setPixmap( QIcon::fromTheme( QLatin1String(icon) ).pixmap( 32 ) );
    else
        resultLB->setPixmap( QPixmap() );
    lb->setEnabled( ( state == NotSet || state == Failed ) && !delay );
    pb->setEnabled( ( state == NotSet || state == Failed ) && !delay );
    if ( state == AlreadySet )
        statusLB->setText( xi18nc("@info","No NullPin found. <warning>If this PIN was not set by you personally, the card might have been tampered with.</warning>") );
}

static QString format_error( const Error & err ) {
    if ( err.isCanceled() )
        return i18nc("@info","Canceled setting PIN.");
    if ( err )
        return xi18nc("@info",
                     "There was an error setting the PIN: <message>%1</message>.",
                     QString::fromLocal8Bit( err.asString() ).toHtmlEscaped() );
    else
        return i18nc("@info","PIN set successfully.");
}

class SetInitialPinDialog::Private {
    friend class ::Kleo::Dialogs::SetInitialPinDialog;
    SetInitialPinDialog * const q;
public:
    explicit Private( SetInitialPinDialog * qq )
        : q( qq ),
          nksState( Unknown ),
          sigGState( Unknown ),
          ui( q )
    {
        
    }

private:
    void slotNksButtonClicked() {
        nksState = Ongoing;
        ui.nksStatusLB->clear();
        updateWidgets();
        emit q->nksPinRequested();
    }

    void slotSigGButtonClicked() {
        sigGState = Ongoing;
        ui.sigGStatusLB->clear();
        updateWidgets();
        emit q->sigGPinRequested();
    }

private:
    void updateWidgets() {
        update_widget( nksState,  false,
                       ui.nksResultIcon,  ui.nksLB,  ui.nksPB,  ui.nksStatusLB  );
        update_widget( sigGState, nksState == NotSet || nksState == Failed || nksState == Ongoing,
                       ui.sigGResultIcon, ui.sigGLB, ui.sigGPB, ui.sigGStatusLB );
        ui.closePB()->setEnabled( q->isComplete() );
        ui.cancelPB()->setEnabled( !q->isComplete() );
    }

private:
    State nksState, sigGState;

    struct UI : public Ui::SetInitialPinDialog {
        explicit UI( Dialogs::SetInitialPinDialog * qq )
            : Ui::SetInitialPinDialog()
        {
            setupUi( qq );

            closePB()->setEnabled( false );

            connect( closePB(), SIGNAL(clicked()), qq, SLOT(accept()) );
        }

        QAbstractButton * closePB() const {
            assert( dialogButtonBox );
            return dialogButtonBox->button( QDialogButtonBox::Close );
        }

        QAbstractButton * cancelPB() const {
            assert( dialogButtonBox );
            return dialogButtonBox->button( QDialogButtonBox::Cancel );
        }

    } ui;
};

SetInitialPinDialog::SetInitialPinDialog( QWidget * p, Qt::WindowFlags f )
    : QDialog( p, f ), d( new Private( this ) )
{

}

SetInitialPinDialog::~SetInitialPinDialog() {}

void SetInitialPinDialog::setNksPinPresent( bool on ) {
    d->nksState = on ? AlreadySet : NotSet ;
    d->updateWidgets();
}

void SetInitialPinDialog::setSigGPinPresent( bool on ) {
    d->sigGState = on ? AlreadySet : NotSet ;
    d->updateWidgets();
}

void SetInitialPinDialog::setNksPinSettingResult( const Error & err ) {
    d->ui.nksStatusLB->setText( format_error( err ) );
    d->nksState =
        err.isCanceled() ? NotSet :
        err              ? Failed :
        Ok ;
    d->updateWidgets();
}

void SetInitialPinDialog::setSigGPinSettingResult( const Error & err ) {
    d->ui.sigGStatusLB->setText( format_error( err ) );
    d->sigGState =
        err.isCanceled() ? NotSet :
        err              ? Failed :
        Ok ;
    d->updateWidgets();
}

bool SetInitialPinDialog::isComplete() const {
    return ( d->nksState  == Ok || d->nksState  == AlreadySet )
        && ( d->sigGState == Ok || d->sigGState == AlreadySet );
}

#include "moc_setinitialpindialog.cpp"
