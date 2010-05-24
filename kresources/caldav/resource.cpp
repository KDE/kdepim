/*=========================================================================
| KCalDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
| (c) 2009  Kumaran Santhanam (initial KDE4 version)
|
| This project is released under the GNU General Public License.
| Please see the file COPYING for more details.
|--------------------------------------------------------------------------
| Main interface to the KResource system.
 ========================================================================*/

/*=========================================================================
| INCLUDES
 ========================================================================*/

#include <string.h>

#include <qurl.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qeventloop.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>

#include <klocale.h>
#include <kpassdlg.h>

#include <qdatetime.h>
#include <qmutex.h>
#include <qthread.h>

#ifdef KCALDAV_DEBUG
    #include <qfile.h>
#endif

#include "resource.h"
#include "reader.h"
#include "writer.h"

/*=========================================================================
| NAMESPACE
 ========================================================================*/

using namespace KCal;

/*=========================================================================
| CONSTANTS
 ========================================================================*/

const unsigned long ResourceCalDav::TERMINATION_WAITING_TIME = 3 * 1000; // 3 seconds
const int ResourceCalDav::CACHE_DAYS = 90;

const int ResourceCalDav::DEFAULT_RELOAD_INTERVAL   = 10;
const int ResourceCalDav::DEFAULT_SAVE_INTERVAL     = 10;
const int ResourceCalDav::DEFAULT_RELOAD_POLICY     = ResourceCached::ReloadInterval;
const int ResourceCalDav::DEFAULT_SAVE_POLICY       = ResourceCached::SaveDelayed;

/*=========================================================================
| UTILITY
 ========================================================================*/

#define log(s)  kdDebug() << identifier() << ": " << (s);

/*=========================================================================
| CONSTRUCTOR / DESTRUCTOR
 ========================================================================*/

ResourceCalDav::ResourceCalDav( const KConfig *config ) :
    ResourceCached(config)
    , mLock(true)
    , mPrefs(NULL)
    , mLoader(NULL)
    , mWriter(NULL)
    , mLoadingQueueReady(true)
    , mWritingQueueReady(true)
{
    log("ResourceCalDav(config)");
    init();

    if ( config ) {
      readConfig( config );
    }
}

ResourceCalDav::~ResourceCalDav() {
    log("jobs termination");

    // TODO: do we need termination here?

    if (mLoader) {
        mLoader->terminate();
    }
    if (mWriter) {
        mWriter->terminate();
    }

    log("waiting for jobs terminated");

    if (mLoader) {
        mLoader->wait(TERMINATION_WAITING_TIME);
    }
    if (mWriter) {
        mWriter->wait(TERMINATION_WAITING_TIME);
    }

    log("deleting jobs");

    delete mLoader;
    delete mWriter;

    log("deleting preferences");

    delete mPrefs;
}

/*=========================================================================
| GENERAL METHODS
 ========================================================================*/

bool ResourceCalDav::doLoad() {
    bool syncCache = true;

    log(QString("doLoad(%1)").arg(syncCache));

    clearCache();

    log("loading from cache");
    disableChangeNotification();
    loadCache();
    enableChangeNotification();

    log("starting download job");
    startLoading(mPrefs->getFullUrl());

    return true;
}

bool ResourceCalDav::doSave() {
    bool syncCache = true;

    log(QString("doSave(%1)").arg(syncCache));

    if (!hasChanges()) {
        log("no changes");
        return true;
    }

    log("saving cache");
    saveCache();

    log("start writing job");
    startWriting(mPrefs->getFullUrl());

    log("clearing changes");
    // FIXME: Calling clearChanges() here is not the ideal way since the
    // upload might fail, but there is no other place to call it...
    clearChanges();

    return true;
}


bool ResourceCalDav::doSave( bool syncCache, Incidence *incidence ) {
    // To suppress warning about doSave(bool) method hides ResourceCached::doSave(bool, Incidence)
    return ResourceCached::doSave();
}

KABC::Lock* ResourceCalDav::lock() {
    log("lock()");
    return &mLock;
}

void ResourceCalDav::readConfig( const KConfig *config ) {
    log("readConfig");
    mPrefs->readConfig();
    ResourceCached::readConfig(config);
}

void ResourceCalDav::writeConfig( KConfig *config ) {
    log("writeConfig()");
    ResourceCalendar::writeConfig(config);
    mPrefs->writeConfig();
    ResourceCached::writeConfig(config);
}

CalDavPrefs* ResourceCalDav::createPrefs() const {
    log("createPrefs()");
    CalDavPrefs* p = new CalDavPrefs(identifier());
    return p;
}

void ResourceCalDav::init() {
    // default settings
    setReloadInterval(DEFAULT_RELOAD_INTERVAL);
    setReloadPolicy(DEFAULT_RELOAD_POLICY);
    setSaveInterval(DEFAULT_SAVE_INTERVAL);
    setSavePolicy(DEFAULT_SAVE_POLICY);

    // creating preferences
    mPrefs = createPrefs();

    // creating jobs
    // FIXME: Qt4 handles this quite differently, as shown below...
//     mLoader = new CalDavReader;
//     connect(mLoader, SIGNAL(finished()), this, SLOT(loadFinished()));
//     mWriter = new CalDavWriter;
//     connect(mWriter, SIGNAL(finished()), this, SLOT(writingFinished()));

    // ...whereas Qt3 needs events like so:
    mLoader = new CalDavReader;
    //connect(mLoader, SIGNAL(finished()), this, SLOT(loadFinished()));
    mWriter = new CalDavWriter;
    //connect(mWriter, SIGNAL(finished()), this, SLOT(writingFinished()));

    setType("ResourceCalDav");
}

void ResourceCalDav::setIncidencesReadOnly(Incidence::List& inc, bool readOnly) {
    Incidence::List::Iterator it;
    for ( it = inc.begin(); it != inc.end(); ++it ) {
        (*it)->setReadOnly( readOnly );
    }
}

void ResourceCalDav::ensureReadOnlyFlagHonored() {
    //disableChangeNotification();

    Incidence::List inc( rawIncidences() );
    setIncidencesReadOnly(inc, readOnly());

    //enableChangeNotification();

    emit resourceChanged(this);
}

void ResourceCalDav::setReadOnly(bool v) {
    KRES::Resource::setReadOnly(v);
    log("ensuring read only flag honored");
    ensureReadOnlyFlagHonored();
}

/*=========================================================================
| READING METHODS
 ========================================================================*/

void ResourceCalDav::loadingQueuePush(const LoadingTask *task) {
    mLoadingQueue.enqueue(task);
    loadingQueuePop();
}

void ResourceCalDav::loadingQueuePop() {
    if (!mLoadingQueueReady || mLoadingQueue.isEmpty()) {
        return;
    }

    if (!mLoader) {
        log("loader == NULL");
        return;
    }

    // Loading queue and mLoadingQueueReady flag are not shared resources, i.e. only one thread has an access to them.
    // That's why no mutexes are required.
    LoadingTask *t = mLoadingQueue.head();

    mLoader->setUrl(t->url);

    QDateTime dt(QDate::currentDate());
    mLoader->setRange(dt.addDays(-CACHE_DAYS), dt.addDays(CACHE_DAYS));
    //mLoader->setGetAll();

    mLoadingQueueReady = false;

    log("starting actual download job");
    mLoader->start(QThread::LowestPriority);

    // if all ok, removing the task from the queue
    mLoadingQueue.dequeue();

    delete t;

    // FIXME
    // Qt3 needs to wait here for the download to finish, as I am too
    // lazy to set up the correct event mechanisms at this time
    // The correct mechanism would be to have the thread call
    // QApplication::postEvent(), have the GUI trap the event,
    // and then call loadFinished()
    while (mLoader->running() == true)
        qApp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
    loadFinished();
}

void ResourceCalDav::startLoading(const QString& url) {
    LoadingTask *t = new LoadingTask;
    t->url = url;
    loadingQueuePush(t);
}

void ResourceCalDav::loadFinished() {
    CalDavReader* loader = mLoader;

    log("load finished");

    if (!loader) {
        log("loader is NULL");
        return;
    }

    if (loader->error()) {
        if (loader->errorNumber() == -401) {
            if (NULL != mPrefs) {
                QCString newpass;
                if (KPasswordDialog::getPassword (newpass, QString("<b>") + i18n("Remote authorization required") + QString("</b><p>") + i18n("Please input the password for") + QString(" ") + mPrefs->getusername(), NULL) != 1) {
                    log("load error: " + loader->errorString() );
                    loadError(QString("[%1] ").arg(abs(loader->errorNumber())) + loader->errorString());
                }
                else {
                    // Set new password and try again
                    mPrefs->setPassword(QString(newpass));
                    startLoading(mPrefs->getFullUrl());
                }
            }
            else {
                log("load error: " + loader->errorString() );
                loadError(QString("[%1] ").arg(abs(loader->errorNumber())) + loader->errorString());
            }
        }
        else {
            log("load error: " + loader->errorString() );
            loadError(QString("[%1] ").arg(abs(loader->errorNumber())) + loader->errorString());
        }
    } else {
        log("successful load");
        QString data = loader->data();

        if (!data.isNull() && !data.isEmpty()) {
            // TODO: I don't know why, but some schedules on http://caldav-test.ioda.net/ (I used it for testing)
            // have some lines separated by single \r rather than \n or \r\n.
            // ICalFormat fails to parse that.
            data.replace("\r\n", "\n"); // to avoid \r\n becomes \n\n after the next line
            data.replace('\r', '\n');

            log("trying to parse...");
            if (parseData(data)) {
                log("... parsing is ok");
                log("clearing changes");
                clearChanges();
                emit resourceLoaded(this);
            }
        }
    }

    // Loading queue and mLoadingQueueReady flag are not shared resources, i.e. only one thread has an access to them.
    // That's why no mutexes are required.
    mLoadingQueueReady = true;
    loadingQueuePop();
}

bool ResourceCalDav::checkData(const QString& data) {
    log("checking the data");

    ICalFormat ical;
    bool ret = true;
    if (!ical.fromString(&mCalendar, data)) {
        log("invalid ical string");
        ret = false;
    }

    return ret;
}

bool ResourceCalDav::parseData(const QString& data) {
    log("parseData()");

    bool ret = true;

    // check if the data is OK
    // May be it's not efficient (parsing is done twice), but it should be safe
    if (!checkData(data)) {
        loadError(i18n("Parsing calendar data failed."));
        return false;
    }

    log("clearing cache");
    clearCache();
    emit resourceChanged(this);

    disableChangeNotification();

    log("actually parsing the data");

    ICalFormat ical;
    if ( !ical.fromString( &mCalendar, data ) ) {
        // this should never happen, but...
        ret = false;
    }

    // debug code here -------------------------------------------------------
#ifdef KCALDAV_DEBUG
    const QString fout_path = "/tmp/kcaldav_download_" + identifier() + ".tmp";

    QFile fout(fout_path);
    if (fout.open(IO_WriteOnly | IO_Append)) {
        QTextStream sout(&fout);
        sout << "---------- " << resourceName() << ": --------------------------------\n";
        sout << data << "\n";
        fout.close();
    } else {
        loadError(i18n("can't open file"));
    }
#endif // KCALDAV_DEBUG
    // end of debug code ----------------------------------------------------

    enableChangeNotification();

    if (ret) {
        log("parsing is ok");
        //if ( !noReadOnlyOnLoad() && readOnly() ) {
        if ( readOnly() ) {
            log("ensuring read only flag honored");
            ensureReadOnlyFlagHonored();
        }
        log("saving to cache");
        saveCache();
    }

    emit resourceChanged(this);

    return ret;
}

/*=========================================================================
| WRITING METHODS
 ========================================================================*/

QString ResourceCalDav::getICalString(const Incidence::List& inc) {
    if (inc.isEmpty()) {
        return "";
    }

    CalendarLocal loc(timeZoneId());
    QString data = "";
    ICalFormat ical;

    // NOTE: This is very susceptible to invalid entries in added/changed/deletedIncidences
    // Be very careful with clearChange/clearChanges, and be sure to clear after load and save...
    for(Incidence::List::ConstIterator it = inc.constBegin(); it != inc.constEnd(); ++it) {
        Incidence *i = (*it)->clone();
        loc.addIncidence(i);
    }

    data = ical.toString(&loc);

    return data;
}

void ResourceCalDav::writingQueuePush(const WritingTask *task) {
//     printf("task->added: %s\n\r", task->added.ascii());
//     printf("task->deleted: %s\n\r", task->deleted.ascii());
//     printf("task->changed: %s\n\r", task->changed.ascii());
    mWritingQueue.enqueue(task);
    writingQueuePop();
}

void ResourceCalDav::writingQueuePop() {
    // FIXME This crashes...

    if (!mWritingQueueReady || mWritingQueue.isEmpty()) {
        return;
    }

    if (!mWriter) {
        log("writer == NULL");
        return;
    }

    // Writing queue and mWritingQueueReady flag are not shared resources, i.e. only one thread has an access to them.
    // That's why no mutexes are required.
    WritingTask *t = mWritingQueue.head();

    log("writingQueuePop: url = " + t->url);

    mWriter->setUrl(t->url);

#ifdef KCALDAV_DEBUG
    const QString fout_path = "/tmp/kcaldav_upload_" + identifier() + ".tmp";

    QFile fout(fout_path);
    if (fout.open(IO_WriteOnly | IO_Append)) {
        QTextStream sout(&fout);
        sout << "---------- " << resourceName() << ": --------------------------------\n";
        sout << "================== Added:\n" << t->added << "\n";
        sout << "================== Changed:\n" << t->changed << "\n";
        sout << "================== Deleted:\n" << t->deleted << "\n";
        fout.close();
    } else {
        loadError(i18n("can't open file"));
    }
#endif // debug

    mWriter->setAddedObjects(t->added);
    mWriter->setChangedObjects(t->changed);
    mWriter->setDeletedObjects(t->deleted);

    mWritingQueueReady = false;

    log("starting actual write job");
    mWriter->start(QThread::LowestPriority);

    // if all ok, remove the task from the queue
    mWritingQueue.dequeue();

    delete t;

    // FIXME
    // Qt3 needs to wait here for the download to finish, as I am too
    // lazy to set up the correct event mechanisms at this time
    // The correct mechanism would be to have the thread call
    // QApplication::postEvent(), have the GUI trap the event,
    // and then call writingFinished()
    while (mWriter->running() == true)
        qApp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
    writingFinished();
}

void ResourceCalDav::startWriting(const QString& url) {
    log("startWriting: url = " + url);

    WritingTask *t = new WritingTask;

    Incidence::List added = addedIncidences();
    Incidence::List changed = changedIncidences();
    Incidence::List deleted = deletedIncidences();

    t->url = url;
    t->added = getICalString(added);		// This crashes when an event is added from the remote server and save() is subsequently called
    t->changed = getICalString(changed);
    t->deleted = getICalString(deleted);

    writingQueuePush(t);
}

void ResourceCalDav::writingFinished() {
    log("writing finished");

    if (!mWriter) {
        log("mWriter is NULL");
        return;
    }

    if (mWriter->error()) {
        log("error: " + mWriter->errorString());
        saveError(QString("[%1] ").arg(abs(mWriter->errorNumber())) + mWriter->errorString());
    } else {
        log("success");
        // is there something to do here?
    }

    // Writing queue and mWritingQueueReady flag are not shared resources, i.e. only one thread has an access to them.
    // That's why no mutexes are required.
    mWritingQueueReady = true;
    writingQueuePop();
}

// EOF ========================================================================
