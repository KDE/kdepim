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

#include "newresultpage.h"

#include "resultlistwidget.h"
#include "resultitemwidget.h"

#include <crypto/taskcollection.h>

#include <kleo/stl_util.h>

#include <boost/mem_fn.hpp>

#include <KLocalizedString>

#include <QCheckBox>
#include <QHash>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QTimer>

#include <cassert>

static const int ProgressBarHideDelay = 2000; // 2 secs

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;
using namespace boost;

class NewResultPage::Private {
    NewResultPage* const q;
public:
    explicit Private( NewResultPage* qq );

    void progress( const QString & msg, int progress, int total );
    void result( const shared_ptr<const Task::Result> & result );
    void started( const shared_ptr<Task> & result );
    void allDone();
    void keepOpenWhenDone( bool keep );
    QLabel * labelForTag( const QString & tag );

    std::vector< shared_ptr<TaskCollection> > m_collections;
    QTimer m_hideProgressTimer;
    QProgressBar* m_progressBar;
    QHash<QString, QLabel*> m_progressLabelByTag;
    QVBoxLayout* m_progressLabelLayout;
    int m_lastErrorItemIndex;
    ResultListWidget* m_resultList;
    QCheckBox* m_keepOpenCB;
};

NewResultPage::Private::Private( NewResultPage* qq ) : q( qq ), m_lastErrorItemIndex( 0 )
{
    m_hideProgressTimer.setInterval( ProgressBarHideDelay );
    m_hideProgressTimer.setSingleShot( true );

    QBoxLayout* const layout = new QVBoxLayout( q );
    QWidget* const labels = new QWidget;
    m_progressLabelLayout = new QVBoxLayout( labels );
    layout->addWidget( labels );
    m_progressBar = new QProgressBar;
    layout->addWidget( m_progressBar );
    m_resultList = new ResultListWidget;
    connect( m_resultList, SIGNAL(linkActivated(QString)), q, SIGNAL(linkActivated(QString)) );
    layout->addWidget( m_resultList, 1 );
    m_keepOpenCB = new QCheckBox;
    m_keepOpenCB->setText( i18n( "Keep open after operation completed" ) );
    m_keepOpenCB->setChecked(true );
    connect( m_keepOpenCB, SIGNAL(toggled(bool)), q, SLOT(keepOpenWhenDone(bool)) );
    layout->addWidget( m_keepOpenCB );

    connect( &m_hideProgressTimer, SIGNAL(timeout()), m_progressBar, SLOT(hide()) );
}

void NewResultPage::Private::progress( const QString & msg, int progress, int total )
{
    Q_UNUSED( msg );
    assert( progress >= 0 );
    assert( total >= 0 );
    m_progressBar->setRange( 0, total );
    m_progressBar->setValue( progress );
}

void NewResultPage::Private::keepOpenWhenDone( bool )
{
}

void NewResultPage::Private::allDone()
{
    assert( !m_collections.empty() );
    if ( !m_resultList->isComplete() )
        return;
    m_progressBar->setRange( 0, 100 );
    m_progressBar->setValue( 100 );
    const bool errorOccurred =
        kdtools::any( m_collections, mem_fn( &TaskCollection::errorOccurred ) );
    m_collections.clear();
    Q_FOREACH ( const QString &i, m_progressLabelByTag.keys() ) { //krazy:exclude=foreach
        if ( !i.isEmpty() ) {
            m_progressLabelByTag.value( i )->setText( i18n( "%1: All operations completed.", i ) );
        } else {
            m_progressLabelByTag.value( i )->setText( i18n( "All operations completed." ) );
        }
    }
    if ( QAbstractButton * cancel = q->wizard()->button( QWizard::CancelButton ) )
        cancel->setEnabled( false );
    emit q->completeChanged();
    if ( !m_keepOpenCB->isChecked() && !errorOccurred )
        if ( QWizard * wiz = q->wizard() )
            if ( QAbstractButton * btn = wiz->button( QWizard::FinishButton ) )
                QTimer::singleShot( 500, btn, SLOT(animateClick()) );
    m_hideProgressTimer.start();
}

void NewResultPage::Private::result( const shared_ptr<const Task::Result> & )
{
}

void NewResultPage::Private::started( const shared_ptr<Task> & task )
{
    assert( task );
    const QString tag = task->tag();
    QLabel * const label = labelForTag( tag );
    assert( label );
    if ( tag.isEmpty() )
        label->setText( i18nc( "number, operation description", "Operation %1: %2", m_resultList->numberOfCompletedTasks() + 1, task->label() ) );
    else
        label->setText( i18nc( "tag( \"OpenPGP\" or \"CMS\"),  operation description", "%1: %2", tag, task->label() ) );
}

NewResultPage::NewResultPage( QWidget * parent ) : QWizardPage( parent ), d( new Private( this ) )
{
    setTitle( i18n( "<b>Results</b>" ) );
}

NewResultPage::~NewResultPage()
{
}

bool NewResultPage::keepOpenWhenDone() const
{
    return d->m_keepOpenCB->isChecked();
}

void NewResultPage::setKeepOpenWhenDone( bool keep )
{
    d->m_keepOpenCB->setChecked( keep );
}

void NewResultPage::setKeepOpenWhenDoneShown( bool on ) {
    d->m_keepOpenCB->setVisible( on );
    if ( !on )
        d->m_keepOpenCB->setChecked( true );
}

void NewResultPage::setTaskCollection( const shared_ptr<TaskCollection> & coll )
{
    //clear(); ### PENDING(marc) implement
    addTaskCollection( coll );
}

void NewResultPage::addTaskCollection( const shared_ptr<TaskCollection> & coll )
{
    assert( coll );
    if ( kdtools::contains( d->m_collections, coll ) )
        return;
    d->m_hideProgressTimer.stop();
    d->m_progressBar->show();
    d->m_collections.push_back( coll );
    d->m_resultList->addTaskCollection( coll );
    connect( coll.get(), SIGNAL(progress(QString,int,int)),
             this, SLOT(progress(QString,int,int)) );
    connect( coll.get(), SIGNAL(done()),
             this, SLOT(allDone()) );
    connect( coll.get(), SIGNAL(result(boost::shared_ptr<const Kleo::Crypto::Task::Result>)),
             this, SLOT(result(boost::shared_ptr<const Kleo::Crypto::Task::Result>)) );
    connect( coll.get(), SIGNAL(started(boost::shared_ptr<Kleo::Crypto::Task>)),
             this, SLOT(started(boost::shared_ptr<Kleo::Crypto::Task>)) );

    Q_FOREACH ( const shared_ptr<Task> & i, coll->tasks() ) { // create labels for all tags in collection
        assert( i );
        QLabel * l = d->labelForTag( i->tag() );
        assert( l ); (void)l;
    }
    emit completeChanged();
}

QLabel* NewResultPage::Private::labelForTag( const QString & tag ) {
    if ( QLabel * const label = m_progressLabelByTag.value( tag ) )
        return label;
    QLabel* label = new QLabel;
    label->setTextFormat( Qt::RichText );
    label->setWordWrap( true );
    m_progressLabelLayout->addWidget( label );
    m_progressLabelByTag.insert( tag, label );
    return label;
}

bool NewResultPage::isComplete() const
{
    return d->m_resultList->isComplete();
}


#include "moc_newresultpage.cpp"
