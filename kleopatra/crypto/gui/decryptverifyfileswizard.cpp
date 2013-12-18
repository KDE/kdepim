/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/decryptverifywizard.cpp

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

#include "decryptverifyfileswizard.h"

#include "decryptverifyoperationwidget.h"

#include <crypto/gui/resultpage.h>
#include <crypto/gui/wizardpage.h>

#include <crypto/task.h>
#include <crypto/taskcollection.h>

#include <utils/scrollarea.h>
#include <utils/kleo_assert.h>

#include <kleo/stl_util.h>
#include "libkleo/ui/filenamerequester.h"

#include <KLocale>
#include <KGuiItem>

#include <QBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QTimer>
#include <QTreeView>

#include <boost/bind.hpp>

#include <vector>
#include <cassert>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;
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

    class OperationsWidget : public WizardPage {
        Q_OBJECT
    public:
        explicit OperationsWidget( QWidget * p=0 );
        ~OperationsWidget();

        void setOutputDirectory( const QString & dir ) {
            m_ui.outputDirectoryFNR.setFileName( dir );
        }

        QString outputDirectory() const {
            return m_ui.outputDirectoryFNR.fileName();
        }

        bool useOutputDirectory() const {
            return m_ui.useOutputDirectoryCB.isChecked();
        }

        void ensureIndexAvailable( unsigned int idx );

        DecryptVerifyOperationWidget * widget( unsigned int idx ) {
            return m_widgets.at( idx );
        }

        bool isComplete() const { return true; }
    private:
        std::vector<DecryptVerifyOperationWidget*> m_widgets;

        struct UI {
            QCheckBox useOutputDirectoryCB; 
            QLabel            outputDirectoryLB;
            FileNameRequester outputDirectoryFNR;
            ScrollArea       scrollArea; // ### replace with KDScrollArea when done
            QVBoxLayout     vlay;
            QHBoxLayout      hlay;

            explicit UI( OperationsWidget * q );
        } m_ui;
    };
}

class DecryptVerifyFilesWizard::Private {
    friend class ::Kleo::Crypto::Gui::DecryptVerifyFilesWizard;
    DecryptVerifyFilesWizard * const q;
public:
    Private( DecryptVerifyFilesWizard * qq );
    ~Private();

    void ensureIndexAvailable( unsigned int idx ) {
        operationsPage.ensureIndexAvailable( idx );
    }

private:
    OperationsWidget operationsPage;
    Gui::ResultPage resultPage;
};


DecryptVerifyFilesWizard::DecryptVerifyFilesWizard( QWidget * p, Qt::WindowFlags f )
    : Wizard( p, f ), d( new Private( this ) )
{

}

DecryptVerifyFilesWizard::~DecryptVerifyFilesWizard() {}

void DecryptVerifyFilesWizard::setOutputDirectory( const QString & dir ) {
    d->operationsPage.setOutputDirectory( dir );
}

QString DecryptVerifyFilesWizard::outputDirectory() const {
    return d->operationsPage.outputDirectory();
}

bool DecryptVerifyFilesWizard::useOutputDirectory() const {
    return d->operationsPage.useOutputDirectory();
}

DecryptVerifyOperationWidget * DecryptVerifyFilesWizard::operationWidget( unsigned int idx ) {
    d->ensureIndexAvailable( idx );
    return d->operationsPage.widget( idx );
}

void DecryptVerifyFilesWizard::onNext( int id )
{
    if ( id == OperationsPage )
        QTimer::singleShot( 0, this, SIGNAL(operationPrepared()) );
    Wizard::onNext( id );
}

void DecryptVerifyFilesWizard::setTaskCollection( const shared_ptr<TaskCollection> & coll )
{
    kleo_assert( coll );
    d->resultPage.setTaskCollection( coll );
}

DecryptVerifyFilesWizard::Private::Private( DecryptVerifyFilesWizard * qq )
    : q( qq ),
      operationsPage( q ),
      resultPage( q )
{
    q->setPage( DecryptVerifyFilesWizard::OperationsPage, &operationsPage );
    q->setPage( DecryptVerifyFilesWizard::ResultPage, &resultPage );
    connect( &resultPage, SIGNAL(linkActivated(QString)), q, SIGNAL(linkActivated(QString)) );

    std::vector<int> order;
    order.push_back( DecryptVerifyFilesWizard::OperationsPage );
    order.push_back( DecryptVerifyFilesWizard::ResultPage );
    q->setPageOrder( order );
    operationsPage.setCommitPage( true );
}

DecryptVerifyFilesWizard::Private::~Private() {}







OperationsWidget::OperationsWidget( QWidget * p )
    : WizardPage( p ), m_widgets(), m_ui( this )
{
    setTitle( i18n("<b>Choose operations to be performed</b>") );
    setSubTitle( i18n("Here you can check and, if needed, override "
                      "the operations Kleopatra detected for the input given.") );
    setCommitPage( true );
    setCustomNextButton( KGuiItem( i18n( "&Decrypt/Verify" ) ) );
}

OperationsWidget::~OperationsWidget() {}

OperationsWidget::UI::UI( OperationsWidget * q )
    : useOutputDirectoryCB( i18n( "Create all output files in a single folder" ), q ),
      outputDirectoryLB( i18n("&Output folder:"), q ),
      outputDirectoryFNR( q ),
      scrollArea( q ),
      vlay( q ),
      hlay()
{
    KDAB_SET_OBJECT_NAME( useOutputDirectoryCB );
    KDAB_SET_OBJECT_NAME( outputDirectoryLB );
    KDAB_SET_OBJECT_NAME( outputDirectoryFNR );
    KDAB_SET_OBJECT_NAME( scrollArea );

    KDAB_SET_OBJECT_NAME( vlay );
    KDAB_SET_OBJECT_NAME( hlay );

    outputDirectoryFNR.setFilter( QDir::Dirs );

    useOutputDirectoryCB.setChecked( true );
    connect( &useOutputDirectoryCB, SIGNAL(toggled(bool)), &outputDirectoryLB, SLOT(setEnabled(bool)) );
    connect( &useOutputDirectoryCB, SIGNAL(toggled(bool)), &outputDirectoryFNR, SLOT(setEnabled(bool)) );

    assert( qobject_cast<QBoxLayout*>(scrollArea.widget()->layout()) );
    static_cast<QBoxLayout*>(scrollArea.widget()->layout())->addStretch( 1 );
    outputDirectoryLB.setBuddy( &outputDirectoryFNR );

    hlay.setMargin( 0 );

    vlay.addWidget( &scrollArea, 1 );
    vlay.addWidget( &useOutputDirectoryCB );
    vlay.addLayout( &hlay );
    hlay.addWidget( &outputDirectoryLB );
    hlay.addWidget( &outputDirectoryFNR );
}

void OperationsWidget::ensureIndexAvailable( unsigned int idx ) {

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

#include "decryptverifyfileswizard.moc"
