/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/signencryptwizard.cpp

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

#include "signencryptwizard.h"

#include "recipientresolvepage.h"
#include "resultdisplaywidget.h"
#include "wizardresultpage.h"
#include "task.h"
#include "kleo-assuan.h"

#include <utils/stl_util.h>

#include <kmime/kmime_header_parsing.h>

#include <gpgme++/key.h>

#include <KLocale>
#include <KPushButton>

#include <QStackedWidget>
#include <QTimer>
#include <QWizard>

#include <boost/bind.hpp>

using namespace Kleo;
using namespace boost;
using namespace GpgME;
using namespace KMime::Types;

class SignEncryptWizard::Private {
    friend class ::Kleo::SignEncryptWizard;
    SignEncryptWizard * q;
public:
    explicit Private( SignEncryptWizard * qq );
    ~Private();

    void setPage( SignEncryptWizard::Page page, QWizardPage* widget ); 
    void restart();
    void next(); 
    void selectPage( SignEncryptWizard::Page id );
    QWizardPage* wizardPage( SignEncryptWizard::Page id ) const;
    QWizardPage* currentWizardPage() const;
    KPushButton* nextButton() const;
    bool isLastPage( SignEncryptWizard::Page ) const;
    void updateButtonStates();

private:
    Mode mode;
    std::map<SignEncryptWizard::Page, QWizardPage*> idToPage;
    std::vector<SignEncryptWizard::Page> pageOrder;
    Page currentId;
    KGuiItem finishItem;
    KGuiItem nextItem;

    RecipientResolvePage * recipientResolvePage;
    WizardResultPage * resultPage;
    QStackedWidget * stack;
};


SignEncryptWizard::Private::Private( SignEncryptWizard * qq )
    : q( qq ),
      mode( EncryptOrSignFiles ),
      currentId( SignEncryptWizard::NoPage ),
      recipientResolvePage( new RecipientResolvePage ),
      resultPage( new WizardResultPage ),
      stack( new QStackedWidget )
{
    QWizard wiz;
    nextItem = KGuiItem( wiz.buttonText( QWizard::NextButton ) );
    finishItem = KGuiItem( wiz.buttonText( QWizard::FinishButton ) );
    q->setMainWidget( stack );    
    q->setButtons( KDialog::Try | KDialog::Cancel );
    setPage( SignEncryptWizard::ResolveRecipientsPage, recipientResolvePage );
    setPage( SignEncryptWizard::ResultPage, resultPage );
    q->connect( q, SIGNAL( tryClicked() ), q, SLOT( next() ) );
    q->resize( QSize( 640, 480 ).expandedTo( q->sizeHint() ) );
}

KPushButton * SignEncryptWizard::Private::nextButton() const
{
    return q->button( KDialog::Try );
}


void SignEncryptWizard::Private::updateButtonStates()
{
    const bool isLast = isLastPage( currentId );
    q->setButtonGuiItem( KDialog::Try, isLast ? finishItem : nextItem );
    nextButton()->setEnabled( q->canGoToNextPage() );
}

bool SignEncryptWizard::Private::isLastPage( SignEncryptWizard::Page id ) const
{
    return !pageOrder.empty() ? pageOrder.back() == id : false;
}

void SignEncryptWizard::Private::setPage( SignEncryptWizard::Page id, QWizardPage * widget )
{
    assuan_assert( id != SignEncryptWizard::NoPage );
    assuan_assert( idToPage.find( id ) == idToPage.end() );
    idToPage[id] = widget;
    stack->addWidget( widget );
    q->connect( widget, SIGNAL( completeChanged() ), q, SLOT( updateButtonStates() ) );
}

void SignEncryptWizard::Private::restart()
{
}

void SignEncryptWizard::Private::selectPage( SignEncryptWizard::Page id )
{
    currentId = id;
    if ( id == SignEncryptWizard::NoPage )
        return;
    QWizardPage * const page = wizardPage( id );
    assert( page && stack->indexOf( page ) != -1 );
    stack->setCurrentWidget( page );
    updateButtonStates();
}

void SignEncryptWizard::Private::next()
{
    assert( currentId != NoPage );
    if ( currentId == SignEncryptWizard::ResolveRecipientsPage )
        QTimer::singleShot( 0, q, SIGNAL( recipientsResolved() ) );
    std::vector<SignEncryptWizard::Page>::const_iterator it = qBinaryFind( pageOrder.begin(), pageOrder.end(), currentId );
    assert( it != pageOrder.end() );
    ++it;
    if ( it == pageOrder.end() ) // "Finish"
    {
        currentId = NoPage;
        q->close();
    }
    else // "next"
    {
        selectPage( *it );
    }
}

SignEncryptWizard::Private::~Private() {}

SignEncryptWizard::SignEncryptWizard( QWidget * p, Qt::WindowFlags f )
    : KDialog( p, f ), d( new Private( this ) )
{
}


SignEncryptWizard::~SignEncryptWizard() {}

void SignEncryptWizard::setMode( Mode mode ) {
    std::vector<Page> pageOrder;
    switch ( mode )
    {
    case EncryptEMail:
        pageOrder.push_back( ResolveRecipientsPage );
        pageOrder.push_back( ResultPage );
        break;
    case SignEMail:
        pageOrder.push_back( ResultPage );
        break;
    case EncryptOrSignFiles:
    default:
        break;
    }
    // EncryptEMail:
    //   1. RecipientResolvePage
    //   2. ResultPage
    // SignEMail:
    //   1. SignerResolvePage
    //   2. ResultPage
    // EncryptOrSignFiles:
    //   1. OperationsPage
    //   2. ObjectsPage
    //   3. RecipientsPage
    //   4. ResultPage
    assuan_assert( mode == EncryptEMail || mode == SignEMail || !"Other cases are not yet implemented" );
    d->pageOrder = pageOrder;

    d->mode = mode;
    d->selectPage( pageOrder.front() );
}

void SignEncryptWizard::setProtocol( Protocol proto ) {
    assuan_assert( d->mode == EncryptEMail || d->mode == SignEMail );
    d->recipientResolvePage->setProtocol( proto );
}

void SignEncryptWizard::setSigningMode( TriState mode ) {
    notImplemented();
}

void SignEncryptWizard::setEncryptionMode( TriState mode ) {
    notImplemented();
}

void SignEncryptWizard::setFiles( const QStringList & files ) {
    notImplemented();
}

QFileInfoList SignEncryptWizard::resolvedFiles() const {
    notImplemented();
}

bool SignEncryptWizard::signingSelected() const {
    notImplemented();
}

bool SignEncryptWizard::encryptionSelected() const {
    notImplemented();
}

void SignEncryptWizard::setRecipientsAndCandidates( const std::vector<Mailbox> & recipients, const std::vector< std::vector<Key> > & keys ) {
    assuan_assert( !keys.empty() );
    assuan_assert( (size_t)recipients.size() == keys.size() );
    assuan_assert( d->mode == EncryptEMail );

    d->recipientResolvePage->ensureIndexAvailable( keys.size() - 1 );

    for ( unsigned int i = 0, end = keys.size() ; i < end ; ++i ) {
        RecipientResolveWidget * const rr = d->recipientResolvePage->recipientResolveWidget( i );
        assuan_assert( rr );
        rr->setIdentifier( recipients[i].prettyAddress() );
        rr->setCertificates( keys[i] );
    }
}

void SignEncryptWizard::setSignersAndCandidates( const std::vector<Mailbox> & signers, const std::vector< std::vector<Key> > & keys ) {
    assuan_assert( !keys.empty() );
    assuan_assert( d->mode == SignEMail );
    notImplemented();
}

QWizardPage* SignEncryptWizard::Private::wizardPage( SignEncryptWizard::Page id ) const
{
    if ( id == SignEncryptWizard::NoPage )
        return 0;

    const std::map<SignEncryptWizard::Page, QWizardPage*>::const_iterator it = idToPage.find( id );   
    assuan_assert( it != idToPage.end() );
    return (*it).second;
}

QWizardPage* SignEncryptWizard::Private::currentWizardPage() const
{
    return wizardPage( currentId );
}

bool SignEncryptWizard::canGoToNextPage() const {
    const QWizardPage * const current = d->currentWizardPage();
    return current ? current->isComplete() : false;
}

void SignEncryptWizard::connectTask( const shared_ptr<Task> & task, unsigned int idx ) {
    assuan_assert( task );
    ResultDisplayWidget* const item = new ResultDisplayWidget;
    item->setLabel( task->label() );
    connect( task.get(), SIGNAL( progress( QString, int, int ) ),
             item, SLOT( setProgress( QString, int, int ) ) );
    connect( task.get(), SIGNAL( error( int, QString ) ),
             item, SLOT( setError( int, QString ) ) );
    connect( task.get(), SIGNAL(result( boost::shared_ptr<const Kleo::Task::Result> ) ),
             item, SLOT( setResult( boost::shared_ptr<const Kleo::Task::Result> ) ) );
    d->resultPage->addResultItem( item );
}

std::vector<Key> SignEncryptWizard::resolvedCertificates() const {
    std::vector<Key> result;
    for ( unsigned int i = 0, end = d->recipientResolvePage->numRecipientResolveWidgets() ; i < end ; ++i )
        result.push_back( d->recipientResolvePage->recipientResolveWidget( i )->chosenCertificate() );
    return result;
}

std::vector<Key> SignEncryptWizard::resolvedSigners() const {
    notImplemented();
}

void SignEncryptWizard::next()
{
    d->next();
}

SignEncryptWizard::Page SignEncryptWizard::currentPage() const
{
    return d->currentId;
}

#include "moc_signencryptwizard.cpp"
