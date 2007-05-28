/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#include <libkmobiletools/weaver.h>

#include <kglobal.h>
#include <libkmobiletools/job.h>
using namespace ThreadWeaver;

class ThreadWeaverPrivate {
    public:
        ThreadWeaverPrivate(KMobileTools::Weaver *_parent)
    : parent(_parent)
        {};
        KMobileTools::Weaver *parent;
};

K_GLOBAL_STATIC(KMobileTools::Weaver, s_instance)

KMobileTools::Weaver::Weaver( QObject* parent )
    : ThreadWeaver::Weaver(parent), d(new ThreadWeaverPrivate(this) )
{
    connect(this, SIGNAL(jobDone(ThreadWeaver::Job*) ), this, SLOT(slotJobDone(ThreadWeaver::Job*) ) );
}

void KMobileTools::Weaver::slotJobDone(ThreadWeaver::Job* j)
{
    if( ! j->inherits("KMobileTools::Job")) return; ///@TODO look for namespace
    emit jobDone( (KMobileTools::Job*) j );
}

KMobileTools::Weaver *KMobileTools::Weaver::instance()
{
    /** The application-global Weaver instance.

    This  instance will only be created if this method is actually called
    in the lifetime of the application. */
    return s_instance;
}


#include "weaver.moc"

