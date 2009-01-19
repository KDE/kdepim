/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

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

#include "jobxp.h"

#include <QtCore/QMetaMethod>
#include <KDE/KDebug>

namespace KMobileTools {

class JobXPPrivate {
public:
    JobXP::Type m_jobType;

    bool m_canBeAborted;
    bool m_canBeAbortedIsCached;

    int m_progress;
};

JobXP::JobXP( JobXP::Type jobType, QObject* parent )
 : ThreadWeaver::Job( parent ), d( new JobXPPrivate )
{
    d->m_jobType = jobType;
    d->m_canBeAborted = false;
    d->m_canBeAbortedIsCached = false;
    d->m_progress = 0;
}


JobXP::~JobXP()
{
    delete d; 
}

JobXP::Type JobXP::jobType() const {
    return d->m_jobType;
}

bool JobXP::canBeAborted() const {
    if( !d->m_canBeAbortedIsCached ) {
        for( int i=0; i<metaObject()->methodCount(); i++ ) {
            // look if the current method is the abort method
            QMetaMethod abortMethod = metaObject()->method( i );
            QString signature( abortMethod.signature() );
            kDebug() << signature;
            int pos = signature.indexOf( '(' );

            if( signature.left( pos ) == "requestAbort" ) {
                d->m_canBeAborted = true;
                break;
            }
        }

        d->m_canBeAbortedIsCached = true;
    }

    return d->m_canBeAborted;
}

void JobXP::setProgress( int percent ) {
    if( percent > d->m_progress && percent <= 100 ) {
        d->m_progress = percent;
        emit progressChanged( percent );
    }
}

int JobXP::progress() const
{
    return d->m_progress;
}

}

#include "jobxp.moc"
