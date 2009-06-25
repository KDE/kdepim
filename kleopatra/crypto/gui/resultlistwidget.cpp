/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/resultlistwidget.cpp

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

#include "resultlistwidget.h"

#include "emailoperationspreferences.h"

#include <crypto/gui/resultitemwidget.h>
#include <crypto/taskcollection.h>

#include <utils/scrollarea.h>

#include <KLocalizedString>
#include <KPushButton>
#include <KStandardGuiItem>

#include <QLabel>
#include <QMoveEvent>
#include <QVBoxLayout>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;
using namespace boost;

class ResultListWidget::Private {
    ResultListWidget* const q;
public:
    explicit Private( ResultListWidget* qq );

    void result( const shared_ptr<const Task::Result> & result );
    void started( const shared_ptr<Task> & task );
    void detailsToggled( bool );
    void allTasksDone();

    void addResultWidget( ResultItemWidget* widget );
    void setupSingle();
    void setupMulti();
    void resizeIfStandalone();
    
    shared_ptr<TaskCollection> m_tasks;
    bool m_standaloneMode;
    int m_lastErrorItemIndex;
    ScrollArea * m_scrollArea;
    KPushButton * m_closeButton;
    QVBoxLayout * m_layout;
    QLabel * m_progressLabel;
};

ResultListWidget::Private::Private( ResultListWidget* qq ) 
    : q( qq ),
    m_tasks(),
    m_standaloneMode( false ),
    m_lastErrorItemIndex( 0 ),
    m_scrollArea( 0 ),
    m_closeButton( 0 ),
    m_layout( 0 ),
    m_progressLabel( 0 )
{
    m_layout = new QVBoxLayout( q );
    m_layout->setMargin( 0 );
    m_layout->setSpacing( 0 );
    m_progressLabel = new QLabel;
    m_progressLabel->setWordWrap( true );
    m_layout->addWidget( m_progressLabel );
    m_progressLabel->setVisible( false );
    
    m_closeButton = new KPushButton;
    m_closeButton->setGuiItem( KStandardGuiItem::close() );
    q->connect( m_closeButton, SIGNAL(clicked()), q, SLOT(close()) );
    m_layout->addWidget( m_closeButton );
    m_closeButton->setVisible( false );
}

ResultListWidget::ResultListWidget( QWidget* parent, Qt::WindowFlags f ) : QWidget( parent, f ), d( new Private( this ) )
{
}

ResultListWidget::~ResultListWidget()
{
    if ( !d->m_standaloneMode )
        return;
    EMailOperationsPreferences prefs;
    prefs.setDecryptVerifyPopupGeometry( geometry() );
    prefs.writeConfig();
}

void ResultListWidget::Private::setupSingle()
{
    m_layout->addStretch();
}

void ResultListWidget::Private::resizeIfStandalone()
{
    if ( m_standaloneMode )
        q->resize( q->size().expandedTo( q->sizeHint() ) );
}

void ResultListWidget::Private::setupMulti()
{
    m_scrollArea = new ScrollArea;
    assert( qobject_cast<QBoxLayout*>( m_scrollArea->widget()->layout() ) );
    static_cast<QBoxLayout*>( m_scrollArea->widget()->layout() )->setMargin( 0 );
    static_cast<QBoxLayout*>( m_scrollArea->widget()->layout() )->setSpacing( 2 );
    static_cast<QBoxLayout*>( m_scrollArea->widget()->layout() )->addStretch();
    m_layout->insertWidget( 0, m_scrollArea );
}

void ResultListWidget::Private::addResultWidget( ResultItemWidget* widget )
{
    assert( widget );
    assert( m_tasks && !m_tasks->isEmpty() );
    
    assert( m_scrollArea );
    assert( m_scrollArea->widget() );
    assert( qobject_cast<QBoxLayout*>( m_scrollArea->widget()->layout() ) );
    QBoxLayout & blay = *static_cast<QBoxLayout*>( m_scrollArea->widget()->layout() );
    blay.insertWidget( widget->hasErrorResult() ? m_lastErrorItemIndex++ : ( blay.count() - 1 ), widget );

    widget->show();
    resizeIfStandalone();
}

void ResultListWidget::Private::allTasksDone() {
    m_progressLabel->setVisible( false );
    resizeIfStandalone();
    emit q->completeChanged();
}

void ResultListWidget::Private::result( const shared_ptr<const Task::Result> & result )
{
    assert( result );
    assert( m_tasks && !m_tasks->isEmpty() );
    ResultItemWidget* wid = new ResultItemWidget( result );
    q->connect( wid, SIGNAL(detailsToggled(bool)), q, SLOT(detailsToggled(bool)) );
    q->connect( wid, SIGNAL(linkActivated(QString)), q, SIGNAL(linkActivated(QString)) );
    q->connect( wid, SIGNAL(closeButtonClicked()), q, SLOT(close()) );
    addResultWidget( wid );
}

bool ResultListWidget::isComplete() const
{
    return d->m_tasks ? d->m_tasks->allTasksCompleted() : false;
}

void ResultListWidget::setTaskCollection( const shared_ptr<TaskCollection> & coll )
{
    assert( !d->m_tasks );
    assert( coll && !coll->isEmpty() );
    d->m_tasks = coll;
    connect( d->m_tasks.get(), SIGNAL(result(boost::shared_ptr<const Kleo::Crypto::Task::Result>)),
             this, SLOT(result(boost::shared_ptr<const Kleo::Crypto::Task::Result>)) );
    connect( d->m_tasks.get(), SIGNAL(started(boost::shared_ptr<Kleo::Crypto::Task>)),
             this, SLOT(started(boost::shared_ptr<Kleo::Crypto::Task>)) );
    connect( d->m_tasks.get(), SIGNAL(done()), this, SLOT(allTasksDone()) );
    d->setupMulti();
    setStandaloneMode( d->m_standaloneMode );
}

void ResultListWidget::Private::detailsToggled( bool ) {
    resizeIfStandalone();
}

void ResultListWidget::Private::started( const shared_ptr<Task> & task )
{
    assert( m_tasks );
    assert( task );
    assert( m_progressLabel );
    m_progressLabel->setText( i18nc( "number, operation description", "Operation %1: %2", m_tasks->numberOfCompletedTasks() + 1, task->label() ) );
    resizeIfStandalone();
}

void ResultListWidget::setStandaloneMode( bool standalone ) {
    d->m_standaloneMode = standalone;
    if ( !d->m_tasks || d->m_tasks->isEmpty() )
        return;
    d->m_closeButton->setVisible( standalone );
    d->m_progressLabel->setVisible( standalone );
}

#include "resultlistwidget.moc"
