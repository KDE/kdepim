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
#include "resultitemwidget.h"
#include "scrollarea.h"

#include <crypto/taskcollection.h>

#include <KLocalizedString>

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;
using namespace boost;

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
    static_cast<QBoxLayout*>( m_scrollArea->widget()->layout() )->setMargin( 0 );
    static_cast<QBoxLayout*>( m_scrollArea->widget()->layout() )->setSpacing( 0 );
    static_cast<QBoxLayout*>( m_scrollArea->widget()->layout() )->addStretch( 1 );
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
        m_progressLabel->setText( i18n( "All operations completed." ) );
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


#include "resultpage.moc"
