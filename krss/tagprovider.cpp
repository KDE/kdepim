/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "tagprovider.h"
#include "config-nepomuk.h"

#include <KLocale>
#include <KDebug>
#include <KGlobal>
#include <boost/shared_ptr.hpp>

#ifdef HAVE_NEPOMUK
#include "nepomuktagprovider/nepomuktagprovider.h"
typedef class KRss::NepomukTagProviderLoadJob TagProviderLoadJobImpl;
#else
#include "defaulttagprovider/defaulttagprovider.h"
typedef class KRss::DefaultTagProviderLoadJob TagProviderLoadJobImpl;
#endif

using namespace KRss;
using boost::shared_ptr;

// caches the loaded instance of TagProvider
K_GLOBAL_STATIC( shared_ptr<TagProvider>, s_tagProvider )

namespace KRss {

class TagProviderRetrieveJobPrivate
{
public:
    explicit TagProviderRetrieveJobPrivate( TagProviderRetrieveJob* const qq )
        : q( qq )
    {
    }

    void tagProviderLoaded( KJob *job );

public:
    TagProviderRetrieveJob* const q;
};

} // namespace KRss

TagProvider::TagProvider( QObject *parent )
    : QObject( parent )
{
}

TagProvider::~TagProvider()
{
}

void TagProviderRetrieveJobPrivate::tagProviderLoaded( KJob *job )
{
    if ( job->error() ) {
        q->setError( TagProviderRetrieveJob::CouldNotRetrieveTagProvider );
        q->setErrorText( job->errorString() );
        kWarning() << job->errorString();
        q->emitResult();
        return;
    }

    s_tagProvider->reset( static_cast<TagProviderLoadJob*>( job )->tagProvider() );
    q->emitResult();
}

TagProviderRetrieveJob::TagProviderRetrieveJob( QObject *parent )
    : KJob( parent), d( new TagProviderRetrieveJobPrivate( this ) )
{
}

TagProviderRetrieveJob::~TagProviderRetrieveJob()
{
    delete d;
}

shared_ptr<TagProvider> TagProviderRetrieveJob::tagProvider() const
{
    return *s_tagProvider;
}

void TagProviderRetrieveJob::start()
{
    if ( s_tagProvider->get() ) {
        // loaded before, so return the instance
        kDebug() << "Already loaded, returning the instance";
        emitResult();
    }
    else {
        // first request, load it
        TagProviderLoadJob *tjob = new TagProviderLoadJobImpl();
        connect( tjob, SIGNAL( result( KJob* ) ), this, SLOT( tagProviderLoaded( KJob* ) ) );
        tjob->start();
    }
}

QString TagProviderRetrieveJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case TagProviderRetrieveJob::CouldNotRetrieveTagProvider:
            result = i18n( "Could not retrieve the tag provider.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

#include "tagprovider.moc"
