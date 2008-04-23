/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/resultpage.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#include "resultpage.h"
#include "resultpage_p.h"

#include "scrollarea.h"

#include <crypto/taskcollection.h>

#include <KLocalizedString>

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QProgressBar>
#include <QUrl>
#include <QVBoxLayout>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;
using namespace boost;

void ResultItemWidget::updateShowDetailsLabel()
{
    if ( !m_showDetailsLabel || !m_detailsLabel )
        return;
    
    const bool show = !m_detailsLabel->isVisible();
    m_showDetailsLabel->setText( QString("<a href=\"kleoresultitem://toggleDetails/\">%1</a>").arg( show ? i18n( "Show Details" ) : i18n( "Hide Details" ) ) );
}

ResultItemWidget::ResultItemWidget( const shared_ptr<const Task::Result> & result, const QString & label, QWidget * parent, Qt::WindowFlags flags) : QWidget( parent, flags ), m_result( result ), m_detailsLabel( 0 ), m_showDetailsLabel( 0 )
{
    assert( m_result );
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    QWidget* hbox = new QWidget;
    QHBoxLayout* hlay = new QHBoxLayout( hbox );
    hlay->setMargin( 0 );
    QLabel* overview = new QLabel;
    overview->setWordWrap( true );
    overview->setTextFormat( Qt::RichText );
    overview->setText( i18nc( "%1: action %2: result; example: Decrypting foo.txt: Succeeded", "<img src=\"%1\"/>%2: %3", m_result->icon(), label, m_result->overview() ) );
    connect( overview, SIGNAL(linkActivated(QString)), this, SLOT(slotLinkActivated(QString)) );

    hlay->addWidget( overview );
    layout->addWidget( hbox );

    const QString details = m_result->details();
 
    if ( details.isEmpty() )
        return;
    
    m_showDetailsLabel = new QLabel;
    connect( m_showDetailsLabel, SIGNAL(linkActivated(QString)), this, SLOT(slotLinkActivated(QString)) );
    hlay->addWidget( m_showDetailsLabel );

    m_detailsLabel = new QLabel;
    m_detailsLabel->setWordWrap( true );
    m_detailsLabel->setTextFormat( Qt::RichText );
    m_detailsLabel->setText( details );
    connect( m_detailsLabel, SIGNAL(linkActivated(QString)), this, SLOT(slotLinkActivated(QString)) );
    layout->addWidget( m_detailsLabel );
    m_detailsLabel->setVisible( false );
    layout->addWidget( hbox );
    
    updateShowDetailsLabel();
}

ResultItemWidget::~ResultItemWidget()
{
}

bool ResultItemWidget::detailsVisible() const
{
    return m_detailsLabel && m_detailsLabel->isVisible();
}

bool ResultItemWidget::hasErrorResult() const
{
    return m_result->hasError();
}

void ResultItemWidget::slotLinkActivated( const QString & link )
{
    const QUrl url( link );
    if ( url.scheme() != "kleoresultitem" ) {
        emit linkActivated( link );
        return;
    }
    
    if ( url.host() == "toggleDetails" ) {
        showDetails( !detailsVisible() );
        return;
    }
}

void ResultItemWidget::showDetails( bool show )
{
    if ( m_detailsLabel )
        m_detailsLabel->setVisible( show );
    updateShowDetailsLabel();
}

class ResultPage::Private {
    ResultPage* const q;
public:
    explicit Private( ResultPage* qq );

    void progress( const QString & msg, int progress, int total );
    void result( const shared_ptr<const Task::Result> & result );
    void started( const shared_ptr<Task> & result );
    void addResultWidget( ResultItemWidget* widget );
    
    shared_ptr<TaskCollection> m_tasks;
    QProgressBar* m_progressBar;
    QLabel* m_progressLabel;
    QLabel* m_progressDetails;
    int m_lastErrorItemIndex;
    ScrollArea* m_scrollArea;
};

void ResultPage::Private::addResultWidget( ResultItemWidget* widget )
{
    assert( widget );
    assert( m_scrollArea->widget() );
    assert( qobject_cast<QBoxLayout*>( m_scrollArea->widget()->layout() ) );
    QBoxLayout & blay = *static_cast<QBoxLayout*>( m_scrollArea->widget()->layout() );
    blay.insertWidget( widget->hasErrorResult() ? m_lastErrorItemIndex++ : ( blay.count() - 1 ), widget );
}

ResultPage::Private::Private( ResultPage* qq ) : q( qq ), m_lastErrorItemIndex( 0 )
{
    QBoxLayout* const layout = new QVBoxLayout( q );
    m_progressLabel = new QLabel;
    layout->addWidget( m_progressLabel );
    m_progressBar = new QProgressBar;
    layout->addWidget( m_progressBar );
    m_progressDetails = new QLabel;
    layout->addWidget( m_progressDetails );
    m_scrollArea = new ScrollArea;
    assert( qobject_cast<QBoxLayout*>( m_scrollArea->widget()->layout() ) );
    static_cast<QBoxLayout*>( m_scrollArea->widget()->layout() )->setSpacing( 2 );
    static_cast<QBoxLayout*>( m_scrollArea->widget()->layout() )->addStretch( 2 );
    layout->addWidget( m_scrollArea );
}

void ResultPage::Private::progress( const QString & msg, int progress, int total )
{
    assert( progress >= 0 );
    assert( total >= 0 );
    m_progressDetails->setText( msg );
    m_progressBar->setRange( 0, total );
    m_progressBar->setValue( progress );
}

void ResultPage::Private::result( const shared_ptr<const Task::Result> & result )
{
    assert( result );
    const shared_ptr<const Task> task = m_tasks->taskById( result->id() );
    assert( task );
    ResultItemWidget* wid = new ResultItemWidget( result, task->label() );
    addResultWidget( wid );
    if ( m_tasks->allTasksCompleted() ) {
        m_progressBar->setRange( 0, 100 );
        m_progressBar->setValue( 100 );
        emit q->completeChanged();
    }
}

void ResultPage::Private::started( const shared_ptr<Task> & task )
{
    assert( task );
    m_progressLabel->setText( i18nc( "number, operation description", "Operation %1: %2", m_tasks->numberOfCompletedTasks() + 1, task->label() ) );
}

ResultPage::ResultPage( QWidget* parent, Qt::WindowFlags flags ) : WizardPage( parent, flags ), d( new Private( this ) )
{
    setTitle( i18n( "<b>Results</b>" ) );
}

ResultPage::~ResultPage()
{
}

void ResultPage::setOperationCompleted()
{
    //### TODO
}

void ResultPage::setTaskCollection( const shared_ptr<TaskCollection> & coll )
{
    if ( d->m_tasks == coll )
        return;
    d->m_tasks = coll;
    assert( d->m_tasks );
    connect( d->m_tasks.get(), SIGNAL(progress(QString,int,int)),
             this, SLOT(progress(QString,int,int)) );
    connect( d->m_tasks.get(), SIGNAL(result(boost::shared_ptr<const Kleo::Crypto::Task::Result>)),
             this, SLOT(result(boost::shared_ptr<const Kleo::Crypto::Task::Result>)) );
    connect( d->m_tasks.get(), SIGNAL(started(boost::shared_ptr<Kleo::Crypto::Task>)),
             this, SLOT(started(boost::shared_ptr<Kleo::Crypto::Task>)) );
}

bool ResultPage::isComplete() const
{
    return d->m_tasks ? d->m_tasks->allTasksCompleted() : false;
}


#include "resultpage_p.moc"
#include "resultpage.moc"
