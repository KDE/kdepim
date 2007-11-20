/* -*- mode: c++; c-basic-offset:4 -*-
    taskprogressitem.cpp

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


#include "taskprogressitem.h"

#include "task.h"

#include <KLocale>

#include <QLabel>
#include <QVBoxLayout>

using namespace Kleo;

class TaskProgressItem::Private {
    friend class ::Kleo::TaskProgressItem;
    TaskProgressItem * const q;
public:
    Private( const boost::shared_ptr<Task> & task, TaskProgressItem * qq );
    ~Private();
    void progress( const QString & what, int current, int total );
    void error( int err, const QString & details );
    void result( const boost::shared_ptr<const Kleo::Task::Result> & result );

private:
    QLabel * dummyLabel;
};

TaskProgressItem::Private::Private( const boost::shared_ptr<Task> & task, TaskProgressItem* qq ) : q( qq )
{
    QVBoxLayout * layout = new QVBoxLayout( q );
    dummyLabel = new QLabel;
    layout->addWidget( dummyLabel );
    q->connect( task.get(), SIGNAL( progress( QString, int, int ) ),
                q, SLOT( progress( QString, int, int ) ) );
    q->connect( task.get(), SIGNAL( error( int, QString ) ),
                q, SLOT( error( int, QString ) ) );
    q->connect( task.get(), SIGNAL(result( boost::shared_ptr<const Kleo::Task::Result> ) ),
                q, SLOT( result( boost::shared_ptr<const Kleo::Task::Result> ) ) );
}

TaskProgressItem::Private::~Private()
{
}

void TaskProgressItem::Private::progress( const QString & what, int current, int total )
{
    dummyLabel->setText( i18n( "progress: %1 (%2/%3)", what, current, total ) );
}

void TaskProgressItem::Private::error( int err, const QString & details )
{
    dummyLabel->setText( i18n( "error (%1): %2", err, details ) );
}

void TaskProgressItem::Private::result( const boost::shared_ptr<const Kleo::Task::Result> & result )
{
    dummyLabel->setText( i18n( "result received" ) );
}

TaskProgressItem::TaskProgressItem( const boost::shared_ptr<Task> & task, QWidget * parent, Qt::WindowFlags flags ) : QWidget( parent, flags ), d( new Private( task, this ) )
{
}

#include "moc_taskprogressitem.cpp"

