/* -*- mode: c++; c-basic-offset:4 -*-
    wizard.cpp

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

#include "wizard.h"
#include "wizardpage.h"
#include "kleo-assuan.h"

#include <KPushButton>

#include <QStackedWidget>
#include <QWizard>

#include <map>

#include <cassert>

using namespace Kleo;

class Wizard::Private {
    friend class ::Wizard;
    Wizard * const q;
public:
    explicit Private( Wizard * qq );
    ~Private();

    void updateButtonStates();
    bool isLastPage( int id ) const;
    KPushButton* nextButton() const;

private:
    std::vector<int> pageOrder;
    std::map<int, WizardPage*> idToPage;
    int currentId;
    QStackedWidget * stack;
    KGuiItem finishItem;
    KGuiItem nextItem;
};


Wizard::Private::Private( Wizard * qq )
    : q( qq ), currentId( -1 ), stack( new QStackedWidget )
{
    QWizard wiz;
    nextItem = KGuiItem( wiz.buttonText( QWizard::NextButton ) );
    finishItem = KGuiItem( wiz.buttonText( QWizard::FinishButton ) );
    q->setButtons( KDialog::Try | KDialog::Cancel );
    q->setMainWidget( stack );    
    q->resize( QSize( 640, 480 ).expandedTo( q->sizeHint() ) );
}

Wizard::Private::~Private() {}



KPushButton * Wizard::Private::nextButton() const
{
    return q->button( KDialog::Try );
}

bool Wizard::Private::isLastPage( int id ) const
{
    return !pageOrder.empty() ? pageOrder.back() == id : false;
}

void Wizard::Private::updateButtonStates()
{
    const bool isLast = isLastPage( currentId );
    q->setButtonGuiItem( KDialog::Try, isLast ? finishItem : nextItem );
    nextButton()->setEnabled( q->canGoToNextPage() );

}

Wizard::Wizard( QWidget * parent, Qt::WFlags f )
  : KDialog( parent, f ), d( new Private( this ) )
{
    
}

Wizard::~Wizard() {}

void Wizard::setPage( int id, WizardPage* widget )
{
    assuan_assert( id != InvalidPage );
    assuan_assert( d->idToPage.find( id ) == d->idToPage.end() );
    d->idToPage[id] = widget;
    d->stack->addWidget( widget );
    connect( widget, SIGNAL( completeChanged() ), this, SLOT( updateButtonStates() ) );

}

void Wizard::setPageOrder( const std::vector<int>& pageOrder )
{
    d->pageOrder = pageOrder;
}

void Wizard::setCurrentPage( int id )
{
    d->currentId = id;
    if ( id == InvalidPage )
        return;
    WizardPage * const widget = page( id );
    assert( widget && d->stack->indexOf( widget ) != -1 );
    d->stack->setCurrentWidget( widget );
    d->updateButtonStates();
}

void Wizard::skipPage( int id )
{
    const std::vector<int>::iterator it = qBinaryFind( d->pageOrder.begin(), d->pageOrder.end(), id );
    if ( it == d->pageOrder.end() )
        return;
    
    if ( currentPage() == id )
        next();

    d->pageOrder.erase( it ); 

}
        
int Wizard::currentPage() const
{
    return d->currentId;
}

bool Wizard::canGoToNextPage() const
{
    const WizardPage * const current = currentPageWidget();
    return current ? current->isComplete() : false;
}

void Wizard::next()
{
    std::vector<int>::const_iterator it = qBinaryFind( d->pageOrder.begin(), d->pageOrder.end(), d->currentId );
    assert( it != d->pageOrder.end() );
    ++it;
    if ( it == d->pageOrder.end() ) // "Finish"
    {
        d->currentId = InvalidPage;
        close();
    }
    else // "next"
    {
        setCurrentPage( *it );
    }
}

const WizardPage* Wizard::page( int id ) const
{
    if ( id == InvalidPage )
        return 0;

    const std::map<int, WizardPage*>::const_iterator it = d->idToPage.find( id );
    assuan_assert( it != d->idToPage.end() );
    return (*it).second;
}

const WizardPage* Wizard::currentPageWidget() const
{
    return page( d->currentId );
}


WizardPage* Wizard::currentPageWidget()
{
    return page( d->currentId );
}

void Wizard::onNext( int currentId )
{
    Q_UNUSED( currentId )
}

WizardPage* Wizard::page( int id )
{
    if ( id == InvalidPage )
        return 0;

    const std::map<int, WizardPage*>::const_iterator it = d->idToPage.find( id );
    assuan_assert( it != d->idToPage.end() );
    return (*it).second;
}

#include "moc_wizard.cpp"

