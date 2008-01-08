/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/decryptverifywizard.cpp

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

#include "decryptverifywizard.h"

#include "decryptverifyoperationwidget.h"
#include "decryptverifyresultwidget.h"
#include "scrollarea.h"

#include <utils/filenamerequester.h>
#include <utils/stl_util.h>

#include <KLocale>

#include <QScrollArea>
#include <QWizardPage>
#include <QLayout>
#include <QLabel>
#include <QEventLoop>
#include <QPointer>
#include <QAbstractButton>
#include <QScrollBar>

#include <boost/bind.hpp>

#include <vector>
#include <cassert>

using namespace Kleo;
using namespace boost;

namespace {

    class HLine : public QFrame {
        Q_OBJECT
    public:
        explicit HLine( QWidget * p=0, Qt::WindowFlags f=0 )
            : QFrame( p, f )
        {
            setFrameStyle( QFrame::HLine|QFrame::Sunken );
        }
    };

    class OperationsPage : public QWizardPage {
        Q_OBJECT
    public:
        explicit OperationsPage( QWidget * p=0 );
        ~OperationsPage();

        void setOutputDirectory( const QString & dir ) {
            m_ui.outputDirectoryFNR.setFileName( dir );
        }

        QString outputDirectory() const {
            return m_ui.outputDirectoryFNR.fileName();
        }

        void ensureIndexAvailable( unsigned int idx );

        DecryptVerifyOperationWidget * widget( unsigned int idx ) {
            return m_widgets.at( idx );
        }

    private:
        std::vector<DecryptVerifyOperationWidget*> m_widgets;

        struct UI {
            QLabel            outputDirectoryLB;
            FileNameRequester outputDirectoryFNR;
            ScrollArea       scrollArea; // ### replace with KDScrollArea when done
            QVBoxLayout     vlay;
            QHBoxLayout      hlay;

            explicit UI( OperationsPage * q );
        } m_ui;
    };


    class ResultPage : public QWizardPage {
        Q_OBJECT
    public:
        explicit ResultPage( QWidget * p=0 );
        ~ResultPage();

        void ensureIndexAvailable( unsigned int idx );

        DecryptVerifyResultWidget * widget( unsigned int idx ) {
            return m_widgets.at( idx );
        }

    private Q_SLOTS:
        void slotMaybeNoMoreProgress() {
            wizard()->button( QWizard::CancelButton )
                ->setEnabled( kdtools::any( m_widgets.begin(), m_widgets.end(),
                                            bind( &DecryptVerifyResultWidget::operationInProgress, _1 ) ) );
        }

    private:
        std::vector<DecryptVerifyResultWidget*> m_widgets;

        struct UI {
            ScrollArea   scrollArea; // ### replace with KDScrollArea when done
            QVBoxLayout vlay;
            
            explicit UI( ResultPage * q );
        } m_ui;
    };
}

class DecryptVerifyWizard::Private {
    friend class ::Kleo::DecryptVerifyWizard;
    DecryptVerifyWizard * const q;
public:
    Private( DecryptVerifyWizard * qq );
    ~Private();

    void ensureIndexAvailable( unsigned int idx ) {
        operationsPage.ensureIndexAvailable( idx );
        resultPage.ensureIndexAvailable( idx );
    }

private:
    OperationsPage operationsPage;
    ResultPage resultPage;
};


DecryptVerifyWizard::DecryptVerifyWizard( QWidget * p, Qt::WindowFlags f )
    : QWizard( p, f ), d( new Private( this ) )
{

}

DecryptVerifyWizard::~DecryptVerifyWizard() {}

void DecryptVerifyWizard::setOutputDirectory( const QString & dir ) {
    d->operationsPage.setOutputDirectory( dir );
}

QString DecryptVerifyWizard::outputDirectory() const {
    return d->operationsPage.outputDirectory();
}

DecryptVerifyOperationWidget * DecryptVerifyWizard::operationWidget( unsigned int idx ) {
    d->ensureIndexAvailable( idx );
    return d->operationsPage.widget( idx );
}

DecryptVerifyResultWidget * DecryptVerifyWizard::resultWidget( unsigned int idx ) {
    d->ensureIndexAvailable( idx );
    return d->resultPage.widget( idx );
}

bool DecryptVerifyWizard::waitForOperationSelection() {
    if ( !isVisible() )
        return true;

    assert( button( NextButton ) );
    assert( button( CancelButton ) );

    QEventLoop loop;
    QPointer<QObject> that = this;
    connect( this, SIGNAL(currentIdChanged(int)), &loop, SLOT(quit()) );
    connect( this, SIGNAL(finished(int)), &loop, SLOT(quit()) );
    loop.exec();

    return that && currentPage() == &d->resultPage ;
}




DecryptVerifyWizard::Private::Private( DecryptVerifyWizard * qq )
    : q( qq ),
      operationsPage( q ),
      resultPage( q )
{
    q->setOptions( q->options() | NoBackButtonOnStartPage | NoBackButtonOnLastPage );
    q->addPage( &operationsPage );
    q->addPage( &resultPage );
}

DecryptVerifyWizard::Private::~Private() {}







OperationsPage::OperationsPage( QWidget * p )
    : QWizardPage( p ), m_widgets(), m_ui( this )
{
    setTitle( i18n("Choose operations to be performed") );
    setSubTitle( i18n("Here you can check and, if needed, override "
                      "the operations Kleopatra detected for the input given.") );
    setCommitPage( true );
    setButtonText( QWizard::CommitButton, i18n("&Decrypt/Verify") );
}

OperationsPage::~OperationsPage() {}

OperationsPage::UI::UI( OperationsPage * q )
    : outputDirectoryLB( i18n("&Output directory:"), q ),
      outputDirectoryFNR( q ),
      scrollArea( q ),
      vlay( q ),
      hlay()
{
    KDAB_SET_OBJECT_NAME( outputDirectoryLB );
    KDAB_SET_OBJECT_NAME( outputDirectoryFNR );
    KDAB_SET_OBJECT_NAME( scrollArea );

    KDAB_SET_OBJECT_NAME( vlay );
    KDAB_SET_OBJECT_NAME( hlay );

    assert( qobject_cast<QBoxLayout*>(scrollArea.widget()->layout()) );
    static_cast<QBoxLayout*>(scrollArea.widget()->layout())->addStretch( 1 );
    outputDirectoryLB.setBuddy( &outputDirectoryFNR );

    hlay.setMargin( 0 );

    vlay.addWidget( &scrollArea, 1 );
    vlay.addLayout( &hlay );
    hlay.addWidget( &outputDirectoryLB );
    hlay.addWidget( &outputDirectoryFNR );
}

void OperationsPage::ensureIndexAvailable( unsigned int idx ) {

    if ( idx < m_widgets.size() )
        return;

    assert( m_ui.scrollArea.widget() );
    assert( qobject_cast<QBoxLayout*>( m_ui.scrollArea.widget()->layout() ) );
    QBoxLayout & blay = *static_cast<QBoxLayout*>( m_ui.scrollArea.widget()->layout() );

    for ( unsigned int i = m_widgets.size() ; i < idx+1 ; ++i ) {
        if ( i )
            blay.insertWidget( blay.count()-1, new HLine( m_ui.scrollArea.widget() ) );
        DecryptVerifyOperationWidget * w = new DecryptVerifyOperationWidget( m_ui.scrollArea.widget() );
        blay.insertWidget( blay.count()-1, w );
        w->show();
        m_widgets.push_back( w );
    }
}






ResultPage::ResultPage( QWidget * p )
    : QWizardPage( p ), m_widgets(), m_ui( this )
{
    setTitle( i18n("Results") );
    setSubTitle( i18n("Detailed operation results") );
    setButtonText( QWizard::FinishButton, i18n("&OK") );
}

ResultPage::~ResultPage() {}

ResultPage::UI::UI( ResultPage * q )
    : scrollArea( q ),
      vlay( q )
{
    KDAB_SET_OBJECT_NAME( scrollArea );
    KDAB_SET_OBJECT_NAME( vlay );

    assert( qobject_cast<QBoxLayout*>(scrollArea.widget()->layout()) );
    static_cast<QBoxLayout*>(scrollArea.widget()->layout())->addStretch( 1 );

    vlay.addWidget( &scrollArea );
}

void ResultPage::ensureIndexAvailable( unsigned int idx ) {

    if ( idx < m_widgets.size() )
        return;

    assert( m_ui.scrollArea.widget() );
    assert( qobject_cast<QBoxLayout*>( m_ui.scrollArea.widget()->layout() ) );
    QBoxLayout & blay = *static_cast<QBoxLayout*>( m_ui.scrollArea.widget()->layout() );

    for ( unsigned int i = m_widgets.size() ; i < idx+1 ; ++i ) {
        if ( i )
            blay.insertWidget( blay.count()-1, new HLine( m_ui.scrollArea.widget() ) );
        DecryptVerifyResultWidget * w = new DecryptVerifyResultWidget( m_ui.scrollArea.widget() );
        connect( w, SIGNAL(operationStateChanged()), this, SLOT(slotMaybeNoMoreProgress()) );
        blay.insertWidget( blay.count()-1, w );
        w->show();
        m_widgets.push_back( w );
    }
    
}





#include "decryptverifywizard.moc"
#include "moc_decryptverifywizard.cpp"
