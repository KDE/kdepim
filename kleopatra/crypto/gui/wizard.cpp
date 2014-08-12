/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/wizard.cpp

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

#include "wizard.h"
#include "wizardpage.h"

#include <utils/kleo_assert.h>

#include <KDebug>
#include <KGuiItem>
#include <KLocalizedString>
#include <QPushButton>
#include <KStandardGuiItem>

#include <QDialogButtonBox>
#include <QFrame>
#include <QLabel>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <map>
#include <set>

#include <cassert>

using namespace Kleo::Crypto::Gui;

class Wizard::Private {
    friend class ::Wizard;
    Wizard * const q;
public:
    explicit Private( Wizard * qq );
    ~Private();

    void updateButtonStates();
    bool isLastPage( int id ) const;
    int previousPage() const;
    void updateHeader();

private:
    std::vector<int> pageOrder;
    std::set<int> hiddenPages;
    std::map<int, WizardPage*> idToPage;
    int currentId;
    QStackedWidget* stack;
    QPushButton* nextButton;
    QPushButton* backButton;
    QPushButton* cancelButton;
    KGuiItem finishItem;
    KGuiItem nextItem;
    QFrame* titleFrame;
    QLabel* titleLabel;
    QLabel* subTitleLabel;
    QFrame* explanationFrame;
    QLabel* explanationLabel;
    QTimer* nextPageTimer;
};


Wizard::Private::Private( Wizard * qq )
    : q( qq ), currentId( -1 ), stack( new QStackedWidget )
{
    nextPageTimer = new QTimer( q );
    nextPageTimer->setInterval( 0 );
    connect( nextPageTimer, SIGNAL(timeout()), q, SLOT(next()) );
    nextItem = KGuiItem( i18n( "&Next" ) );
    finishItem = KStandardGuiItem::ok();
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
    
    top->addWidget( stack );
    
    explanationFrame = new QFrame;
    explanationFrame->setFrameShape( QFrame::StyledPanel );
    explanationFrame->setAutoFillBackground( true );
    explanationFrame->setBackgroundRole( QPalette::Base );
    QVBoxLayout* const explanationLayout = new QVBoxLayout( explanationFrame );
    explanationLabel = new QLabel;
    explanationLabel->setWordWrap( true );
    explanationLayout->addWidget( explanationLabel );
    top->addWidget( explanationFrame );
    explanationFrame->setVisible( false );

    QWidget* buttonWidget = new QWidget;
    QHBoxLayout* buttonLayout = new QHBoxLayout( buttonWidget );
    QDialogButtonBox * const box = new QDialogButtonBox;
    
    cancelButton = box->addButton( QDialogButtonBox::Cancel );
    q->connect( cancelButton, SIGNAL(clicked()), q, SLOT(reject()) );

    backButton = new QPushButton;
    backButton->setText( i18n( "Back" ) );
    q->connect( backButton, SIGNAL(clicked()), q, SLOT(back()) );
    box->addButton( backButton, QDialogButtonBox::ActionRole );

    nextButton = new QPushButton;
    KGuiItem::assign(nextButton, nextItem );
    q->connect( nextButton, SIGNAL(clicked()), q, SLOT(next()) );
    box->addButton( nextButton, QDialogButtonBox::ActionRole );
    buttonLayout->addWidget( box );
    
    top->addWidget( buttonWidget );
    
    q->connect( q, SIGNAL(rejected()), q, SIGNAL(canceled()) ); 
}

Wizard::Private::~Private() { qDebug(); }



bool Wizard::Private::isLastPage( int id ) const
{
    return !pageOrder.empty() ? pageOrder.back() == id : false;
}

void Wizard::Private::updateButtonStates()
{
    const bool isLast = isLastPage( currentId );
    const bool canGoToNext = q->canGoToNextPage();
    WizardPage* const page = q->page( currentId );
    const KGuiItem customNext = page ? page->customNextButton() : KGuiItem();
    if ( customNext.text().isEmpty() && customNext.icon().isNull() )
        KGuiItem::assign(nextButton, isLast ? finishItem : nextItem );
    else
        KGuiItem::assign( nextButton, customNext );
    nextButton->setEnabled( canGoToNext );
    cancelButton->setEnabled( !isLast || !canGoToNext );
    backButton->setEnabled( q->canGoToPreviousPage() );
    if ( page && page->autoAdvance() && page->isComplete() )
        nextPageTimer->start();
}

void Wizard::Private::updateHeader()
{
    WizardPage * const widget = q->page( currentId );
    assert( !widget || stack->indexOf( widget ) != -1 );
    if ( widget )
        stack->setCurrentWidget( widget );
    const QString title = widget ? widget->title() : QString();
    const QString subTitle = widget ? widget->subTitle() : QString();
    const QString explanation = widget ? widget->explanation() : QString();
    titleFrame->setVisible( !title.isEmpty() || !subTitle.isEmpty() || !explanation.isEmpty() );
    titleLabel->setVisible( !title.isEmpty() );
    titleLabel->setText( title );
    subTitleLabel->setText( subTitle );
    subTitleLabel->setVisible( !subTitle.isEmpty() );
    explanationFrame->setVisible( !explanation.isEmpty() );
    explanationLabel->setVisible( !explanation.isEmpty() );
    explanationLabel->setText( explanation );    
    q->resize( q->sizeHint().expandedTo( q->size() ) );
}

Wizard::Wizard( QWidget * parent, Qt::WindowFlags f )
  : QDialog( parent, f ), d( new Private( this ) )
{
    
}

Wizard::~Wizard() { qDebug(); }

void Wizard::setPage( int id, WizardPage* widget )
{
    kleo_assert( id != InvalidPage );
    kleo_assert( d->idToPage.find( id ) == d->idToPage.end() );
    d->idToPage[id] = widget;
    d->stack->addWidget( widget );
    connect( widget, SIGNAL(completeChanged()), this, SLOT(updateButtonStates()) );
    connect( widget, SIGNAL(titleChanged()), this, SLOT(updateHeader()) );
    connect( widget, SIGNAL(subTitleChanged()), this, SLOT(updateHeader()) );
    connect( widget, SIGNAL(explanationChanged()), this, SLOT(updateHeader()) );
    connect( widget, SIGNAL(autoAdvanceChanged()), this, SLOT(updateButtonStates()) );
    connect( widget, SIGNAL(windowTitleChanged(QString)), this, SLOT(setWindowTitle(QString)) );
}

void Wizard::setPageOrder( const std::vector<int>& pageOrder )
{
    d->pageOrder = pageOrder;
    d->hiddenPages.clear();
    if ( pageOrder.empty() )
        return;
    setCurrentPage( pageOrder.front() );
}

void Wizard::setCurrentPage( int id )
{
    d->currentId = id;
    if ( id == InvalidPage )
        return;
    d->updateHeader();
    d->updateButtonStates();
}

void Wizard::setPageVisible( int id, bool visible )
{
    if ( visible )
        d->hiddenPages.erase( id );
    else
        d->hiddenPages.insert( id );
    if ( currentPage() == id && !visible )
        next(); 
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
    WizardPage* const current = currentPageWidget();
    if ( current )
        current->onNext();
    onNext( d->currentId );
    std::vector<int>::const_iterator it = qBinaryFind( d->pageOrder.begin(), d->pageOrder.end(), d->currentId );
    assert( it != d->pageOrder.end() );
    
    do {
        ++it;
    }
    while ( d->hiddenPages.find( *it ) != d->hiddenPages.end() );

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

    do {
        --it;
    } while ( it != pageOrder.begin() && hiddenPages.find( *it ) != hiddenPages.end() );
    return *it; 
}

void Wizard::back()
{
    onBack( d->currentId );
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
    kleo_assert( it != d->idToPage.end() );
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
    kleo_assert( it != d->idToPage.end() );
    return (*it).second;
}

#include "moc_wizard.cpp"
