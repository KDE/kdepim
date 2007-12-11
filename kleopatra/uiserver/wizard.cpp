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

#include <KLocale>
#include <KPushButton>

#include <QDialogButtonBox>
#include <QFrame>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>
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
    int previousPage() const;

private:
    std::vector<int> pageOrder;
    std::map<int, WizardPage*> idToPage;
    int currentId;
    QStackedWidget* stack;
    QPushButton* nextButton;
    QPushButton* backButton;
    QString finishItem;
    QString nextItem;
    QFrame* titleFrame;
    QLabel* titleLabel;
    QLabel* subTitleLabel;
};


Wizard::Private::Private( Wizard * qq )
    : q( qq ), currentId( -1 ), stack( new QStackedWidget )
{
    const QWizard wiz;
    nextItem = wiz.buttonText( QWizard::NextButton );
    finishItem = wiz.buttonText( QWizard::FinishButton );
    QVBoxLayout * const top = new QVBoxLayout( q );
    top->setMargin( 0 );
    titleFrame = new QFrame;
    titleFrame->setFrameShape( QFrame::StyledPanel );
    titleFrame->setAutoFillBackground( true );
    titleFrame->setBackgroundRole( QPalette::Base );
    QVBoxLayout* const titleLayout = new QVBoxLayout( titleFrame );
    titleLabel = new QLabel;
    titleLayout->addWidget( titleLabel );
    subTitleLabel = new QLabel;
    subTitleLabel->setWordWrap( true );
    titleLayout->addWidget( subTitleLabel );
    top->addWidget( titleFrame );
    titleFrame->setVisible( false );
    QWidget* const contentWidget = new QWidget;
    QVBoxLayout* const contentLayout = new QVBoxLayout( contentWidget );
    contentLayout->addWidget( stack );
    QDialogButtonBox * const box = new QDialogButtonBox;
    QAbstractButton* const cancelButton = box->addButton( QDialogButtonBox::Cancel );
    q->connect( cancelButton, SIGNAL( clicked() ), q, SLOT( reject() ) );

    backButton = new QPushButton;
    backButton->setText( wiz.buttonText( QWizard::BackButton ) );
    q->connect( backButton, SIGNAL( clicked() ), q, SLOT( back() ) );
    box->addButton( backButton, QDialogButtonBox::ActionRole );

    nextButton = new QPushButton;
    nextButton->setText( nextItem );
    q->connect( nextButton, SIGNAL( clicked() ), q, SLOT( next() ) );
    box->addButton( nextButton, QDialogButtonBox::ActionRole );

    contentLayout->addWidget( box );
    top->addWidget( contentWidget );

    q->connect( q, SIGNAL( rejected() ), q, SIGNAL( canceled() ) ); 
    q->resize( QSize( 640, 480 ).expandedTo( q->sizeHint() ) );
}

Wizard::Private::~Private() {}



bool Wizard::Private::isLastPage( int id ) const
{
    return !pageOrder.empty() ? pageOrder.back() == id : false;
}

void Wizard::Private::updateButtonStates()
{
    const bool isLast = isLastPage( currentId );
    nextButton->setText( isLast ? finishItem : nextItem );
    nextButton->setEnabled( q->canGoToNextPage() );
    backButton->setEnabled( q->canGoToPreviousPage() );
}

Wizard::Wizard( QWidget * parent, Qt::WFlags f )
  : QDialog( parent, f ), d( new Private( this ) )
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
    const QString title = widget->title();
    const QString subTitle = widget->subTitle();
    d->titleFrame->setVisible( !title.isEmpty() || !subTitle.isEmpty() );
    d->titleLabel->setText( title );
    d->subTitleLabel->setText( subTitle );
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

bool Wizard::canGoToPreviousPage() const
{
    const int prev = d->previousPage();
    if ( prev == InvalidPage ) 
        return false;
    const WizardPage * const prevPage = page( prev );
    assert( prevPage );
    return !prevPage->isCommitPage();
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

int Wizard::Private::previousPage() const
{
   if ( pageOrder.empty() )
        return InvalidPage;

    std::vector<int>::const_iterator it = qBinaryFind( pageOrder.begin(), pageOrder.end(), currentId );
    if ( it == pageOrder.begin() || it == pageOrder.end() )
        return InvalidPage;

    --it;
    return *it; 
}

void Wizard::back()
{
    const int prev = d->previousPage();
    if ( prev == InvalidPage ) 
        return;
    setCurrentPage( prev );
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


void Wizard::onBack( int currentId )
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
