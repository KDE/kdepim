/*
 * This file is part of krss
 *
 * Copyright (C) 2009 Frank Osterfeld <osterfeld@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <QCoreApplication>

#include <krss/importitemsjob.h>
#include <krss/resource.h>
#include <krss/resourcemanager.h>

#include <iostream>

using namespace KRss;

static void printUsage() {
    std::cerr << "Usage: itemimporter <resourceIdentifier> <feedUrl> <itemfile>" << std::endl;
}

class JobExecutor : public QObject {
  Q_OBJECT
public:
    void execJob( KJob* job ) {
        connect( job, SIGNAL(finished(KJob*)), this, SLOT(finished(KJob*)) );
        job->start();
    }

private Q_SLOTS:
    void finished( KJob* job ) {
        if ( job->error() )
            std::cerr << job->error() << qPrintable( job->errorString() ) << std::endl;
        QCoreApplication::quit();
    }
};

int main( int argc, char** argv ) {
    QCoreApplication app( argc, argv );

    if ( argc != 4 ) {
        printUsage();
        return 1;
    }
    
    const QString resourceId = QString::fromLocal8Bit( argv[1] );
    const QString url = QString::fromLocal8Bit( argv[2] );
    const QString file = QString::fromLocal8Bit( argv[3] );

    const Resource* const r = ResourceManager::self()->resource( resourceId );
    if ( !r ) {
        std::cerr << "Resource does not exist: " << qPrintable(resourceId) << std::endl;
        return 1;
    }
    
    ImportItemsJob* job = r->createImportItemsJob( url, file );
    JobExecutor exec;
    exec.execJob( job );
    return app.exec();
}

#include "itemimporter.moc"

