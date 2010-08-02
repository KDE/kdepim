/*
 * loader.cpp
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "loader.h"
#include "document.h"
#include "feeddetector.h"

#include <kio/job.h>
#include <kprocess.h>
#include <kstaticdeleter.h>
#include <kurl.h>
#include <kdebug.h>

#include <tqdom.h>
#include <tqbuffer.h>
#include <tqregexp.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqtimer.h>

using namespace RSS;

DataRetriever::DataRetriever()
{
}

DataRetriever::~DataRetriever()
{
}

class FileRetriever::Private
{
    public:
        
   Private()
      : buffer(NULL),
        lastError(0), job(NULL)
   {
   }

   ~Private()
   {
      delete buffer;
   }

   TQBuffer *buffer;
   int lastError;
   KIO::Job *job;
   static KStaticDeleter<TQString> userAgentsd;
   static TQString* userAgent;
};

KStaticDeleter<TQString> FileRetriever::Private::userAgentsd;
TQString* FileRetriever::Private::userAgent = 0L;
FileRetriever::FileRetriever()
   : d(new Private)
{
}

FileRetriever::~FileRetriever()
{
   delete d;
}

bool FileRetriever::m_useCache = true;

TQString FileRetriever::userAgent()
{
    if (Private::userAgent == 0L)
        FileRetriever::Private::userAgentsd.setObject(Private::userAgent, new TQString);
    return *Private::userAgent;
}

void FileRetriever::setUserAgent(const TQString &ua)
{
    if (Private::userAgent == 0L)
        FileRetriever::Private::userAgentsd.setObject(Private::userAgent, new TQString);
    (*Private::userAgent) = ua;
}

void FileRetriever::setUseCache(bool enabled)
{
    m_useCache = enabled;
}

void FileRetriever::retrieveData(const KURL &url)
{
   if (d->buffer)
      return;

   d->buffer = new TQBuffer;
   d->buffer->open(IO_WriteOnly);

   KURL u=url;

   if (u.protocol()=="feed")
       u.setProtocol("http");

   d->job = KIO::get(u, false, false);
   d->job->addMetaData("cache", m_useCache ? "refresh" : "reload");

   TQString ua = userAgent();
   if (!ua.isEmpty())
      d->job->addMetaData("UserAgent", ua);


   TQTimer::singleShot(1000*90, this, TQT_SLOT(slotTimeout()));

   connect(d->job, TQT_SIGNAL(data(KIO::Job *, const TQByteArray &)),
                TQT_SLOT(slotData(KIO::Job *, const TQByteArray &)));
   connect(d->job, TQT_SIGNAL(result(KIO::Job *)), TQT_SLOT(slotResult(KIO::Job *)));
   connect(d->job, TQT_SIGNAL(permanentRedirection(KIO::Job *, const KURL &, const KURL &)),
                TQT_SLOT(slotPermanentRedirection(KIO::Job *, const KURL &, const KURL &)));
}

void FileRetriever::slotTimeout()
{
    abort();

    delete d->buffer;
    d->buffer = NULL;

    d->lastError = KIO::ERR_SERVER_TIMEOUT;

    emit dataRetrieved(TQByteArray(), false);
}

int FileRetriever::errorCode() const
{
   return d->lastError;
}

void FileRetriever::slotData(KIO::Job *, const TQByteArray &data)
{
   d->buffer->writeBlock(data.data(), data.size());
}

void FileRetriever::slotResult(KIO::Job *job)
{
   TQByteArray data = d->buffer->buffer();
   data.detach();

   delete d->buffer;
   d->buffer = NULL;

   d->lastError = job->error();
   emit dataRetrieved(data, d->lastError == 0);
}

void FileRetriever::slotPermanentRedirection(KIO::Job *, const KURL &, const KURL &newUrl)
{
   emit permanentRedirection(newUrl);
}

void FileRetriever::abort()
{
	if (d->job)
	{
		d->job->kill(true);
		d->job = NULL;
	}
}

struct OutputRetriever::Private
{
   Private() : process(NULL),
      buffer(NULL),
      lastError(0)
   {
   }

   ~Private()
   {
      delete process;
      delete buffer;
   }

   KShellProcess *process;
   TQBuffer *buffer;
   int lastError;
};

OutputRetriever::OutputRetriever() :
   d(new Private)
{
}

OutputRetriever::~OutputRetriever()
{
   delete d;
}

void OutputRetriever::retrieveData(const KURL &url)
{
   // Ignore subsequent calls if we didn't finish the previous job yet.
   if (d->buffer || d->process)
      return;

   d->buffer = new TQBuffer;
   d->buffer->open(IO_WriteOnly);

   d->process = new KShellProcess();
   connect(d->process, TQT_SIGNAL(processExited(KProcess *)),
                       TQT_SLOT(slotExited(KProcess *)));
   connect(d->process, TQT_SIGNAL(receivedStdout(KProcess *, char *, int)),
                       TQT_SLOT(slotOutput(KProcess *, char *, int)));
   *d->process << url.path();
   d->process->start(KProcess::NotifyOnExit, KProcess::Stdout);
}

int OutputRetriever::errorCode() const
{
   return d->lastError;
}

void OutputRetriever::slotOutput(KProcess *, char *data, int length)
{
   d->buffer->writeBlock(data, length);
}

void OutputRetriever::slotExited(KProcess *p)
{
   if (!p->normalExit())
      d->lastError = p->exitStatus();

   TQByteArray data = d->buffer->buffer();
   data.detach();

   delete d->buffer;
   d->buffer = NULL;

   delete d->process;
   d->process = NULL;

   emit dataRetrieved(data, p->normalExit() && p->exitStatus() == 0);
}

struct Loader::Private
{
   Private() : retriever(NULL),
      lastError(0)
   {
   }

   ~Private()
   {
      delete retriever;
   }

   DataRetriever *retriever;
   int lastError;
   KURL discoveredFeedURL;
   KURL url;
};

Loader *Loader::create()
{
   return new Loader;
}

Loader *Loader::create(TQObject *object, const char *slot)
{
   Loader *loader = create();
   connect(loader, TQT_SIGNAL(loadingComplete(Loader *, Document, Status)),
           object, slot);
   return loader;
}

Loader::Loader() : d(new Private)
{
}

Loader::~Loader()
{
    delete d;
}

void Loader::loadFrom(const KURL &url, DataRetriever *retriever)
{
   if (d->retriever != NULL)
      return;

   d->url=url;
   d->retriever = retriever;

   connect(d->retriever, TQT_SIGNAL(dataRetrieved(const TQByteArray &, bool)),
           this, TQT_SLOT(slotRetrieverDone(const TQByteArray &, bool)));

   d->retriever->retrieveData(url);
}

int Loader::errorCode() const
{
   return d->lastError;
}

void Loader::abort()
{
	if (d && d->retriever)
	{
		d->retriever->abort();
        delete d->retriever;
		d->retriever=NULL;
	}
    emit loadingComplete(this, TQDomDocument(), Aborted);
    delete this;
}

const KURL &Loader::discoveredFeedURL() const
{
   return d->discoveredFeedURL;
}

void Loader::slotRetrieverDone(const TQByteArray &data, bool success)
{
   d->lastError = d->retriever->errorCode();

   delete d->retriever;
   d->retriever = NULL;

   Document rssDoc;
   Status status = Success;

   if (success) {
      TQDomDocument doc;

      /* Some servers insert whitespace before the <?xml...?> declaration.
       * QDom doesn't tolerate that (and it's right, that's invalid XML),
       * so we strip that.
       */

      const char *charData = data.data();
      int len = data.count();

      while (len && TQChar(*charData).isSpace()) {
         --len;
         ++charData;
      }

      if ( len > 3 && TQChar(*charData) == TQChar(0357) ) { // 0357 0273 0277
			  len -= 3;
			  charData += 3;
	  }
      TQByteArray tmpData;
      tmpData.setRawData(charData, len);

      if (doc.setContent(tmpData))
      {
         rssDoc = Document(doc);
         if (!rssDoc.isValid())
         {
            discoverFeeds(tmpData);
            status = ParseError;
         }
      }
      else
      {
         discoverFeeds(tmpData);
         status = ParseError;
      }

      tmpData.resetRawData(charData, len);
   } else
      status = RetrieveError;

   emit loadingComplete(this, rssDoc, status);

   delete this;
}

void Loader::discoverFeeds(const TQByteArray &data)
{
    TQString str = TQString(data).simplifyWhiteSpace();
    
    TQStringList feeds; 
    
    FeedDetectorEntryList list = FeedDetector::extractFromLinkTags(str); 
    
    for (FeedDetectorEntryList::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        feeds += (*it).url();
    }  
    
    if (list.isEmpty())
        feeds = FeedDetector::extractBruteForce(str);
        
    TQString feed = feeds.first();
    TQString host = d->url.host();
    KURL testURL;
    // loop through, prefer feeds on same host
    TQStringList::Iterator end( feeds.end() );
    for ( TQStringList::Iterator it = feeds.begin(); it != end; ++it) 
    {
        testURL=*it;
        if (testURL.host() == host)
        {
            feed = *it;
            break;
        }
    }

    d->discoveredFeedURL = feed.isNull() ? TQString() : FeedDetector::fixRelativeURL(feed, d->url); 
}

#include "loader.moc"
// vim:noet:ts=4
