/* -*- mode: c++; c-basic-offset:4 -*-
    wizardresultpage.cpp

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


#include "wizardresultpage.h"

#include "scrollarea.h"

#include <KLocale>

#include <QVBoxLayout>

#include <cassert>

using namespace Kleo;

class WizardResultPage::Private {
    friend class ::WizardResultPage;
    WizardResultPage * const q;
public:
    explicit Private( WizardResultPage * qq );
    ~Private();
    
private:
    ScrollArea* scrollArea;
};


WizardResultPage::Private::Private( WizardResultPage * qq )
  : q( qq )
{
    QVBoxLayout* layout = new QVBoxLayout( q );
    scrollArea = new ScrollArea;
    assert( qobject_cast<QBoxLayout*>( scrollArea->widget()->layout() ) );
    static_cast<QBoxLayout*>( scrollArea->widget()->layout() )->addStretch( 1 );
    layout->addWidget( scrollArea );
}

WizardResultPage::Private::~Private() {}



WizardResultPage::WizardResultPage( QWidget * parent )
  : WizardPage( parent ), d( new Private( this ) )
{
    setTitle( i18n( "<b>Results</b>" ) );
}


void WizardResultPage::addResultItem( QWidget* widget )
{
    assert( d->scrollArea->widget() );
    assert( qobject_cast<QBoxLayout*>( d->scrollArea->widget()->layout() ) );
    QBoxLayout & blay = *static_cast<QBoxLayout*>( d->scrollArea->widget()->layout() );
    blay.insertWidget( blay.count() - 1, widget );

}

WizardResultPage::~WizardResultPage() {}


bool WizardResultPage::isComplete() const
{
    return true;
}

