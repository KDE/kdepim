/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "feed.h"
#include "fetchtransaction.h"
#include "feedscollection.h"

#include <kurl.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <kapplication.h>

#include <qtimer.h>
#include <qdatetime.h>
#include <qlistview.h>
#include <qdom.h>

using namespace Akregator;
using namespace RSS;

Feed::Feed(QListViewItem *i, FeedsCollection *coll)
    : FeedGroup(i, coll)
    , m_useCustomExpiry(false) 
    , m_expiryAge(0) 
    , m_fetchError(false)
    , m_fetchTries(0)
    , m_loader(0)
    , m_merged(false)
    , m_transaction(0)
    , m_unread(0)
    , m_articles()
{
}

Feed::~Feed()
{}

QDomElement Feed::toXml( QDomElement parent, QDomDocument document ) const
{
    QDomElement el = document.createElement( "outline" );
    el.setAttribute( "text", title() );
    el.setAttribute( "title", title() );
    el.setAttribute( "xmlUrl", m_xmlUrl );
    el.setAttribute( "htmlUrl", m_htmlUrl );
    el.setAttribute( "description", m_description );
    el.setAttribute( "autoFetch", (autoFetch() ? "true" : "false") );
    el.setAttribute( "fetchInterval", QString::number(fetchInterval()) );
    el.setAttribute( "useCustomExpiry", (useCustomExpiry() ? "true" : "false") );
    el.setAttribute( "expiryAge", m_expiryAge );
    
    el.setAttribute( "type", "rss" ); // despite some additional fields, its still "rss" OPML
    el.setAttribute( "version", "RSS" );
    parent.appendChild( el );
    return el;
}

void Feed::dumpXmlData( QDomElement parent, QDomDocument doc )
{
    QDomElement channode = doc.createElement( "channel" );
    parent.appendChild(channode);
    QDomElement tnode = doc.createElement( "title" );
    QDomText t=doc.createTextNode( title() );
    tnode.appendChild(t);
    channode.appendChild(tnode);

    if (!m_htmlUrl.isEmpty())
    {
        QDomElement lnode = doc.createElement( "link" );
        QDomText ht=doc.createTextNode( m_htmlUrl );
        lnode.appendChild(ht);
        channode.appendChild(lnode);
    }

    // rss 2.0 requires channel description
    QDomElement dnode = doc.createElement( "description" );
    QDomText dt=doc.createTextNode( m_htmlUrl );
    dnode.appendChild(dt);
    channode.appendChild(dnode);

    ArticleSequence::ConstIterator it;
    ArticleSequence::ConstIterator en=m_articles.end();
    for (it = m_articles.begin(); it != en; ++it)
    {
        QDomElement enode = doc.createElement( "item" );
        (*it).dumpXmlData(enode, doc);
        channode.appendChild(enode);
    }

}

void Feed::markAllRead()
{
    ArticleSequence::Iterator it;
    ArticleSequence::Iterator en=m_articles.end();
    for (it = m_articles.begin(); it != en; ++it)
    {
        (*it).setStatus(MyArticle::Read);
    }
    m_unread=0;
}

int Feed::expiryAge() const
{
    if (useCustomExpiry())
        return m_expiryAge;
    else 
    {
        if (Settings::useExpiry())
            return Settings::expiryAge();
        else
            return 0;
    }                     
    return 0; // never reached
}

void Feed::appendArticles(const Document &d, bool findDups)
{
    //kdDebug() << "appendArticles findDups=="<<findDups<< " isMerged=="<< m_merged<<endl;
    findDups=true;
    m_articles.enableSorting(false);
    Article::List::ConstIterator it;
    Article::List::ConstIterator en = d.articles().end();
    //kdDebug() << "m_unread before appending articles=="<<m_unread<<endl;
    
    int nudge=0;

    for (it = d.articles().begin(); it != en; ++it)
    {
        MyArticle mya(*it);
        
	if (findDups)
        {
            ArticleSequence::ConstIterator oo=m_articles.find(mya);
            if (oo == m_articles.end() )
            {
                if (m_merged)
                    mya.setStatus(MyArticle::New);
                else
                {
                    if (mya.status() == MyArticle::New)
                        mya.setStatus(MyArticle::Unread);
                }

		mya.offsetFetchTime(nudge);
                appendArticle(mya);
		nudge--;
            }
            //else{
            //kdDebug() << "got dup!!"<<mya.title()<<endl;
            //}
        }
        else
        {
            MyArticle mya(*it);
            if (!m_merged)
            {
                if (mya.status()==MyArticle::New)
                    mya.setStatus(MyArticle::Unread);
            }
	
            mya.offsetFetchTime(nudge);
            appendArticle(mya);
            nudge++;
        }
    }
    m_articles.enableSorting(true);
    m_articles.sort();
    
}

void Feed::appendArticle(const MyArticle &a)
{
    QDateTime now = QDateTime::currentDateTime();
    if (expiryAge() == 0 || a.pubDate().secsTo(now) <= expiryAge() * 3600 * 24) // if not expired
    {
        if (a.status()!=MyArticle::Read)
            m_unread++;
            
        ArticleSequence::Iterator it;
        ArticleSequence::Iterator end = m_articles.end();
        bool inserted = false;    
        it = m_articles.begin();
        
        while ( !inserted && it != end )
            if ( a > (*it) )
                ++it;       
            else
                inserted = true;
        if ( inserted )
            m_articles.insert(it, a);
        else
            m_articles.append(a);    
    }    
}


void Feed::fetch(bool followDiscovery, FetchTransaction *trans)
{
    m_followDiscovery=followDiscovery;
    m_transaction=trans;
    m_fetchTries=0;

    // mark all new as unread
    ArticleSequence::Iterator it;
    ArticleSequence::Iterator en=m_articles.end();
    for (it = m_articles.begin(); it != en; ++it)
    {
        if ((*it).status()==MyArticle::New)
        {
            (*it).setStatus(MyArticle::Unread);
        }
    }

   // Disable icon to show it is fetching.
    if (!m_favicon.isNull())
    {
        KIconEffect iconEffect;
        QPixmap tempIcon = iconEffect.apply(m_favicon, KIcon::Small, KIcon::DisabledState);
        item()->setPixmap(0, tempIcon);
    }

    tryFetch();
}

void Feed::abortFetch()
{
    if (m_loader)
        m_loader->abort();
}

void Feed::tryFetch()
{
    if (item() && m_fetchError)
        item()->setPixmap(0, KGlobal::iconLoader()->loadIcon("txt", KIcon::Small));

    m_fetchError=false;

    m_loader = Loader::create( this, SLOT(fetchCompleted(Loader *, Document, Status)) );
    m_loader->loadFrom( m_xmlUrl, new FileRetriever );
}

void Feed::fetchCompleted(Loader *l, Document doc, Status status)
{
    // Note that Loader::~Loader() is private, so you cannot delete Loader instances.
    // You don't need to do that anyway since Loader instances delete themselves.

    if (status!= Success)
    {
        if (m_followDiscovery && (status==ParseError) && (m_fetchTries < 3) && 			(l->discoveredFeedURL().isValid()))
        {
            m_fetchTries++;
            m_xmlUrl=l->discoveredFeedURL().url();
            emit fetchDiscovery(this);
            tryFetch();
            return;
        }
        else
        {
            m_fetchError=true;
            emit fetchError(this);
            return;
        }
    }

    // Restore favicon.
    if (!m_favicon.isNull())
    {
	item()->setPixmap(0, m_favicon);
    }
    else
    {
	loadFavicon();
    }

    m_fetchError=false;
    m_document=doc;
    //kdDebug() << "Feed fetched successfully [" << m_document.title() << "]" << endl;


    if (m_image.isNull())
    {
        QString u=m_xmlUrl;
        QString imageFileName=KGlobal::dirs()->saveLocation("cache", "akregator/Media/")+u.replace("/", "_").replace(":", "_")+".png";
        m_image=QPixmap(imageFileName, "PNG");

        if (m_image.isNull())
        {
            if (m_document.image()) // if we aint got teh image
                                    // and the feed provides one, get it....
            {
		if (m_transaction)
            	    m_transaction->loadImage(this, m_document.image());
	    }
        }
    }

    if (title().isEmpty()) 
        setTitle( m_document.title() );

    m_description = m_document.description();
    m_htmlUrl = m_document.link().url();
    
    //kdDebug() << "ismerged reprots:::"<<isMerged()<<endl;

    bool findDups=isMerged();
    appendArticles(m_document, findDups);

    m_loader=0;
    m_transaction=0;
    emit fetched(this);
}

void Feed::loadFavicon()
{
    if (!m_transaction)
	return;
    m_transaction->loadIcon(this);
}

void Feed::deleteExpiredArticles()
{
    if (expiryAge() != 0)
    {
        long expiryInSec;
        if (m_useCustomExpiry)
            expiryInSec = m_expiryAge * 3600 * 24;
        else     
            expiryInSec = Settings::expiryAge() * 3600 *24;
            
        QDateTime now = QDateTime::currentDateTime();
        ArticleSequence::ConstIterator it;
        ArticleSequence::ConstIterator tmp;
        ArticleSequence::ConstIterator end = m_articles.end();
        // when we found an article which is not yet expired, we can stop, since articles are sorted by date 
        bool foundNotYetExpired = false;
        
        it = m_articles.begin();
        while ( !foundNotYetExpired && it != end )
        {
            if ((*it).pubDate().secsTo(now) > expiryInSec)
            {
                tmp = it;
                ++it;       
                m_articles.remove(*tmp);
         
            }
            else 
                foundNotYetExpired = true;
        }    
    }    
}

void Feed::setFavicon(const QPixmap &p)
{
    if (p.isNull())
	return;
    if (!m_fetchError && item())
            item()->setPixmap(0, p);
    m_favicon=p;
}

void Feed::setImage(const QPixmap &p)
{
    if (p.isNull())
        return;
    m_image=p;
    QString u=m_xmlUrl;
    m_image.save(KGlobal::dirs()->saveLocation("cache", "akregator/Media/")+u.replace("/", "_").replace(":", "_")+".png","PNG");
    emit(imageLoaded(this));
}


// = ArticleSequence ===================================================== //

struct ArticleSequence::Private
{
   int dummy;
   bool doSort :1;
};

ArticleSequence::ArticleSequence()
   : MyArticle::List()
   , d(new Private)
{
}

ArticleSequence::ArticleSequence(const ArticleSequence &other)
   : MyArticle::List(other)
   , d(new Private)
{
}

ArticleSequence::~ArticleSequence()
{
   delete d;
}

/*
    The reason to include insert/append/prepend code here is to:
    a) check if there's another record with the exactly same pubDate() present,
    b) if so, adjust this inserted item's clock off by one second to keep sorting sane,
    c) re-sort added items (if enabled).
    d) use MyArticle::fetchDate for sorting! ( defined by MyArticle::operator <() )
 */
ArticleSequence::iterator ArticleSequence::insert( iterator it, const MyArticle &x )
{
    return MyArticle::List::insert( it, x );
}

void ArticleSequence::insert( iterator it, size_type n, const MyArticle &x )
{
    MyArticle::List::insert( it, n, x );
}

ArticleSequence::iterator ArticleSequence::append( const MyArticle &x )
{
    return MyArticle::List::append( x );
}

ArticleSequence::iterator ArticleSequence::prepend( const MyArticle &x )
{
    return MyArticle::List::prepend( x );
}


void ArticleSequence::enableSorting(bool b)
{
    d->doSort = b;
}

void ArticleSequence::sort()
{
    if (d->doSort)
    {
        qHeapSort( *this );
    }
}


#include "feed.moc"
