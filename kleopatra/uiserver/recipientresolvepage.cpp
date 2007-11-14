/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/recipientresolvepage.cpp

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

#include "recipientresolvepage.h"

#include "kleo-assuan.h"

#include <utils/stl_util.h>

#include <gpgme++/key.h>

#include <boost/bind.hpp>

#include <algorithm>

using namespace Kleo;
using namespace boost;
using namespace GpgME;

class RecipientResolveWidget::Private {
    friend class ::Kleo::RecipientResolveWidget;
    RecipientResolveWidget * const q;
public:
    explicit Private( RecipientResolveWidget * qq )
        : q( qq ),
          ui( q )
    {

    }

private:
    struct Ui {

        explicit Ui( RecipientResolveWidget * q )

        {

        }
    } ui;
};

RecipientResolveWidget::RecipientResolveWidget( QWidget * p )
    : QWidget( p ), d( new Private( this ) )
{

}

RecipientResolveWidget::~RecipientResolveWidget() {}

void RecipientResolveWidget::setIdentifier( const QString & id ) {
    notImplemented();
}

void RecipientResolveWidget::setCertificates( const std::vector<Key> & keys ) {
    notImplemented();
}

GpgME::Key RecipientResolveWidget::chosenCertificate() const {
    notImplemented();
}

bool RecipientResolveWidget::isComplete() const {
    notImplemented();
}


class RecipientResolvePage::Private {
    friend class ::Kleo::RecipientResolvePage;
    RecipientResolvePage * const q;
public:
    explicit Private( RecipientResolvePage * qq )
        : q( qq ),
          ui( q )
    {

    }

private:
    struct Ui {
        std::vector<RecipientResolveWidget*> widgets;

        explicit Ui( RecipientResolvePage * q )

        {

        }

    } ui;
};


RecipientResolvePage::RecipientResolvePage( QWidget * p )
    : QWizardPage( p ), d( new Private( this ) )
{

}

RecipientResolvePage::~RecipientResolvePage() {}

bool RecipientResolvePage::isComplete() const {
    return kdtools::all( d->ui.widgets.begin(), d->ui.widgets.end(),
                         bind( &RecipientResolveWidget::isComplete, _1 ) );
}

void RecipientResolvePage::ensureIndexAvailable( unsigned int idx ) {
    // implement this like decryptverifywizard ->
    // {operations,result}page (all these should be factored into a
    // separate class...)
    for ( unsigned int i = d->ui.widgets.size() ; i < idx+1 ; ++i )
        d->ui.widgets.push_back( new RecipientResolveWidget( this ) );
}

unsigned int RecipientResolvePage::numRecipientResolveWidgets() const {
    return d->ui.widgets.size();
}

RecipientResolveWidget * RecipientResolvePage::recipientResolveWidget( unsigned int idx ) const {
    return d->ui.widgets.at( idx );
}
