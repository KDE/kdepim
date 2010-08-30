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

#include <tqurl.h>
#include <tqmessagebox.h>
#include <tqapplication.h>
#include <tqeventloop.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>

#include <klocale.h>
#include <kpassdlg.h>

#include <tqdatetime.h>
#include <tqmutex.h>
#include <tqthread.h>

#ifdef KCALDAV_DEBUG
    #include <tqfile.h>
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
    , readLockout(false)
    , mAllWritesComplete(false)
    , mLock(true)
    , mPrefs(NULL)
    , mLoader(NULL)
    , mWriter(NULL)
    , mProgress(NULL)
    , mLoadingQueueReady(true)
    , mWritingQueueReady(true)
    , mWriteRetryTimer(NULL)
{
    log("ResourceCalDav(config)");
    init();

    if ( config ) {
      readConfig( config );
    }
}

ResourceCalDav::~ResourceCalDav() {
    log("jobs termination");

    if (mWriteRetryTimer != NULL) {
        mWriteRetryTimer->stop();	// Unfortunately we cannot do anything at this point; if this timer is still running something is seriously wrong
    }

    if (mLoader) {
        readLockout = true;
        mLoader->terminate();
        mLoader->wait(TERMINATION_WAITING_TIME);
        mLoadingQueueReady = true;
    }

    while ((mWriter->running() == true) || (mWritingQueue.isEmpty() == false) || !mWritingQueueReady) {
        readLockout = true;
        sleep(1);
        qApp->processEvents(TQEventLoop::ExcludeUserInput);
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

bool ResourceCalDav::isSaving() {
    doSave();
    return (((mWriteRetryTimer != NULL) ? 1 : 0) || (mWriter->running() == true) || (mWritingQueue.isEmpty() == false) || !mWritingQueueReady || readLockout);
}

/*=========================================================================
| GENERAL METHODS
 ========================================================================*/

bool ResourceCalDav::doLoad() {
    bool syncCache = true;

    if ((mLoadingQueueReady == false) || (mLoadingQueue.isEmpty() == false) || (mLoader->running() == true) || (isSaving() == true)) {
        return true;	// Silently fail; the user has obviously not responded to a dialog and we don't need to pop up more of them!
    }

    log(TQString("doLoad(%1)").arg(syncCache));

    clearCache();

    log("loading from cache");
    disableChangeNotification();
    loadCache();
    enableChangeNotification();
    clearChanges();	// TODO: Determine if this really needs to be here, as it might clear out the calendar prematurely causing user confusion while the download process is running
    emit resourceChanged(this);
    emit resourceLoaded(this);

    log("starting download job");
    startLoading(mPrefs->getFullUrl(), mPrefs->getFullTasksUrl());

    return true;
}

bool ResourceCalDav::doSave() {
    bool syncCache = true;

    log(TQString("doSave(%1)").arg(syncCache));

    if (!hasChanges()) {
        log("no changes");
        return true;
    }

    log("saving cache");
    saveCache();

    // Delete any queued read jobs
    mLoadingQueue.clear();

    // See if there is a running read thread and terminate it
    if (mLoader->running() == true) {
        mLoader->terminate();
        mLoader->wait(TERMINATION_WAITING_TIME);
        mLoadingQueueReady = true;
    }

    log("start writing job");
    if (startWriting(mPrefs->getFullUrl(), mPrefs->getFullTasksUrl()) == true) {
        log("clearing changes");
        // FIXME: Calling clearChanges() here is not the ideal way since the
        // upload might fail, but there is no other place to call it...
        clearChanges();
        if (mWriteRetryTimer != NULL) {
            if (mWriteRetryTimer->isActive() == false) {
                disconnect( mWriteRetryTimer, TQT_SIGNAL(timeout()), this, TQT_SLOT(doSave()) );
                delete mWriteRetryTimer;
                mWriteRetryTimer = NULL;
            }
        }
        return true;
    }
    else return true;	// We do not need to alert the user to this transient failure; a timer has been started to retry the save
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

    // creating reader/writer instances
    mLoader = new CalDavReader;
    mWriter = new CalDavWriter;

    // creating jobs
    // Qt4 handles this quite differently, as shown below,
    // whereas Qt3 needs events (see ::event())
//     connect(mLoader, TQT_SIGNAL(finished()), this, TQT_SLOT(loadFinished()));
//     connect(mWriter, TQT_SIGNAL(finished()), this, TQT_SLOT(writingFinished()));

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

void ResourceCalDav::updateProgressBar(int direction) {
    int current_queued_events;
    static int original_queued_events;

    // See if anything is in the queues
    current_queued_events = mWritingQueue.count() + mLoadingQueue.count();
    if ((direction == 0) && (mLoader->running() == true)) current_queued_events++;
    if ((direction == 1) && (mWriter->running() == true)) current_queued_events++;
    if (current_queued_events > original_queued_events) {
        original_queued_events = current_queued_events;
    }

    if (current_queued_events == 0) {
        if ( mProgress != NULL) {
            mProgress->setComplete();
            mProgress = NULL;
            original_queued_events = 0;
        }
    }
    else {
        if (mProgress == NULL) {
            if (direction == 0) mProgress = KPIM::ProgressManager::createProgressItem(KPIM::ProgressManager::getUniqueID(), i18n("Downloading Calendar") );
            if (direction == 1) mProgress = KPIM::ProgressManager::createProgressItem(KPIM::ProgressManager::getUniqueID(), i18n("Uploading Calendar") );
        }
        mProgress->setProgress( ((((float)original_queued_events-(float)current_queued_events)*100)/(float)original_queued_events) );
    }
}

/*=========================================================================
| READING METHODS
 ========================================================================*/

void ResourceCalDav::loadingQueuePush(const LoadingTask *task) {
   if ((mLoadingQueue.isEmpty() == true) && (mLoader->running() == false)) {
        mLoadingQueue.enqueue(task);
        updateProgressBar(0);
        loadingQueuePop();
    }
}

void ResourceCalDav::loadingQueuePop() {
    if (!mLoadingQueueReady || mLoadingQueue.isEmpty() || (isSaving() == true)) {
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
    mLoader->setTasksUrl(t->tasksUrl);
    mLoader->setParent(this);
    mLoader->setType(0);

    TQDateTime dt(TQDate::currentDate());
    mLoader->setRange(dt.addDays(-CACHE_DAYS), dt.addDays(CACHE_DAYS));
    //mLoader->setGetAll();

    mLoadingQueueReady = false;

    log("starting actual download job");
    mLoader->start(TQThread::LowestPriority);

    // if all ok, removing the task from the queue
    mLoadingQueue.dequeue();
    updateProgressBar(0);

    delete t;
}

void ResourceCalDav::startLoading(const TQString& url, const TQString& tasksUrl) {
    LoadingTask *t = new LoadingTask;
    t->url = url;
    t->tasksUrl = tasksUrl;
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
                TQCString newpass;
                if (KPasswordDialog::getPassword (newpass, TQString("<b>") + i18n("Remote authorization required") + TQString("</b><p>") + i18n("Please input the password for") + TQString(" ") + mPrefs->getusername(), NULL) != 1) {
                    log("load error: " + loader->errorString() );
                    loadError(TQString("[%1] ").arg(abs(loader->errorNumber())) + loader->errorString());
                }
                else {
                    // Set new password and try again
                    mPrefs->setPassword(TQString(newpass));
                    startLoading(mPrefs->getFullUrl(), mPrefs->getFullTasksUrl());
                }
            }
            else {
                log("load error: " + loader->errorString() );
                loadError(TQString("[%1] ").arg(abs(loader->errorNumber())) + loader->errorString());
            }
        }
        else {
            log("load error: " + loader->errorString() );
            loadError(TQString("[%1] ").arg(abs(loader->errorNumber())) + loader->errorString());
        }
    } else {
        log("successful event load");
        TQString data = loader->data();

        if (!data.isNull() && !data.isEmpty()) {
            // TODO: I don't know why, but some schedules on http://caldav-test.ioda.net/ (I used it for testing)
            // have some lines separated by single \r rather than \n or \r\n.
            // ICalFormat fails to parse that.
            data.replace("\r\n", "\n"); // to avoid \r\n becomes \n\n after the next line
            data.replace('\r', '\n');

            log("trying to parse...");
            if (parseData(data)) {
                // FIXME: The agenda view can crash when a change is
                // made on a remote server and a reload is requested!
                log("... parsing is ok");
                log("clearing changes");
                enableChangeNotification();
                clearChanges();
                emit resourceChanged(this);
                emit resourceLoaded(this);
            }
        }
    }

    if (loader->tasksError()) {
        if (loader->tasksErrorNumber() == -401) {
            if (NULL != mPrefs) {
//                 TQCString newpass;
//                 if (KPasswordDialog::getPassword (newpass, TQString("<b>") + i18n("Remote authorization required") + TQString("</b><p>") + i18n("Please input the password for") + TQString(" ") + mPrefs->getusername(), NULL) != 1) {
//                     log("load error: " + loader->tasksErrorString() );
//                     loadError(TQString("[%1] ").arg(abs(loader->tasksErrorNumber())) + loader->tasksErrorString());
//                 }
//                 else {
//                     // Set new password and try again
//                     mPrefs->setPassword(TQString(newpass));
//                     startLoading(mPrefs->getFullUrl(), mPrefs->getFullTasksUrl());
//                 }
            }
            else {
                log("load error: " + loader->tasksErrorString() );
                loadError(TQString("[%1] ").arg(abs(loader->tasksErrorNumber())) + loader->tasksErrorString());
            }
        }
        else {
            log("load error: " + loader->tasksErrorString() );
            loadError(TQString("[%1] ").arg(abs(loader->tasksErrorNumber())) + loader->tasksErrorString());
        }
    } else {
        log("successful tasks load");
        TQString tasksData = loader->tasksData();

        if (!tasksData.isNull() && !tasksData.isEmpty()) {
            // TODO: I don't know why, but some schedules on http://caldav-test.ioda.net/ (I used it for testing)
            // have some lines separated by single \r rather than \n or \r\n.
            // ICalFormat fails to parse that.
            tasksData.replace("\r\n", "\n"); // to avoid \r\n becomes \n\n after the next line
            tasksData.replace('\r', '\n');

            log("trying to parse...");
            if (parseTasksData(tasksData)) {
                // FIXME: The agenda view can crash when a change is
                // made on a remote server and a reload is requested!
                log("... parsing is ok");
                log("clearing changes");
                enableChangeNotification();
                clearChanges();
                emit resourceChanged(this);
                emit resourceLoaded(this);
            }
        }
    }

    // Loading queue and mLoadingQueueReady flag are not shared resources, i.e. only one thread has an access to them.
    // That's why no mutexes are required.
    mLoader->terminate();
    mLoader->wait(TERMINATION_WAITING_TIME);
    mLoadingQueueReady = true;
    updateProgressBar(0);
    loadingQueuePop();
}

bool ResourceCalDav::checkData(const TQString& data) {
    log("checking the data");

    ICalFormat ical;
    bool ret = true;
    if (!ical.fromString(&mCalendar, data)) {
        log("invalid ical string");
        ret = false;
    }

    return ret;
}

bool ResourceCalDav::parseData(const TQString& data) {
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

    disableChangeNotification();

    log("actually parsing the data");

    ICalFormat ical;
    if ( !ical.fromString( &mCalendar, data ) ) {
        // this should never happen, but...
        ret = false;
    }

    // debug code here -------------------------------------------------------
#ifdef KCALDAV_DEBUG
    const TQString fout_path = "/tmp/kcaldav_download_" + identifier() + ".tmp";

    TQFile fout(fout_path);
    if (fout.open(IO_WriteOnly | IO_Append)) {
        TQTextStream sout(&fout);
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

    return ret;
}

bool ResourceCalDav::parseTasksData(const TQString& data) {
    log("parseTasksData()");

    bool ret = true;

    // check if the data is OK
    // May be it's not efficient (parsing is done twice), but it should be safe
    if (!checkData(data)) {
        loadError(i18n("Parsing calendar data failed."));
        return false;
    }

    disableChangeNotification();

    log("actually parsing the data");

    ICalFormat ical;
    if ( !ical.fromString( &mCalendar, data ) ) {
        // this should never happen, but...
        ret = false;
    }

    // debug code here -------------------------------------------------------
#ifdef KCALDAV_DEBUG
    const TQString fout_path = "/tmp/kcaldav_download_" + identifier() + ".tmp";

    TQFile fout(fout_path);
    if (fout.open(IO_WriteOnly | IO_Append)) {
        TQTextStream sout(&fout);
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

    return ret;
}

/*=========================================================================
| WRITING METHODS
 ========================================================================*/

TQString ResourceCalDav::getICalString(const Incidence::List& inc) {
    if (inc.isEmpty()) {
        return "";
    }

    CalendarLocal loc(timeZoneId());
    TQString data = "";
    TQString header = "";
    TQString footer = "";
    ICalFormat ical;

    // Get the iCal header and footer
    header = ical.toString(&loc);
    int location = header.find("END:VCALENDAR", 0, true);
    footer = header.mid(location, 0xffffffff);
    header.remove("END:VCALENDAR");

    data = data + header;
    // NOTE: This is very susceptible to invalid entries in added/changed/deletedIncidences
    // Be very careful with clearChange/clearChanges, and be sure to clear after load and save...
    for(Incidence::List::ConstIterator it = inc.constBegin(); it != inc.constEnd(); ++it) {
        Incidence *i = (*it)->clone();
        data = data + ical.toString(i, &mCalendar);
    }
    data = data + footer;

    return data;
}

void ResourceCalDav::writingQueuePush(const WritingTask *task) {
//     printf("task->added: %s\n\r", task->added.ascii());
//     printf("task->deleted: %s\n\r", task->deleted.ascii());
//     printf("task->changed: %s\n\r", task->changed.ascii());
    mWritingQueue.enqueue(task);
    updateProgressBar(1);
    writingQueuePop();
}

void ResourceCalDav::writingQueuePop() {
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
    mWriter->setTasksUrl(t->tasksUrl);
    mWriter->setParent(this);
    mWriter->setType(1);

#ifdef KCALDAV_DEBUG
    const TQString fout_path = "/tmp/kcaldav_upload_" + identifier() + ".tmp";

    TQFile fout(fout_path);
    if (fout.open(IO_WriteOnly | IO_Append)) {
        TQTextStream sout(&fout);
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

    mWriter->setAddedTasksObjects(t->tasksAdded);
    mWriter->setChangedTasksObjects(t->tasksChanged);
    mWriter->setDeletedTasksObjects(t->tasksDeleted);

    mWritingQueueReady = false;

    log("starting actual write job");
    mWriter->start(TQThread::LowestPriority);

    // if all ok, remove the task from the queue
    mWritingQueue.dequeue();
    updateProgressBar(1);

    delete t;
}

bool ResourceCalDav::event ( TQEvent * e ) {
    if (e->type() == 1000) {
        // Read done
        loadFinished();
        return TRUE;
    }
    else if (e->type() == 1001) {
        // Write done
        writingFinished();
        return TRUE;
    }
    else return FALSE;
}

void ResourceCalDav::releaseReadLockout() {
    readLockout = false;
}

bool ResourceCalDav::startWriting(const TQString& url, const TQString& tasksUrl) {
    log("startWriting: url = " + url);

    // WARNING: This will segfault if a separate read or write thread
    // modifies the calendar with clearChanges() or similar
    // Before these calls are made any existing read (and maybe write) threads should be finished
    if ((mLoader->running() == true) || (mLoadingQueue.isEmpty() == false) || (mWriter->running() == true) || (mWritingQueue.isEmpty() == false)) {
        if (mWriteRetryTimer == NULL) {
            mWriteRetryTimer = new TQTimer(this);
            connect( mWriteRetryTimer, TQT_SIGNAL(timeout()), TQT_SLOT(doSave()) );
        }
        mWriteRetryTimer->start(1000, TRUE);
        return false;
    }

    // If we don't lock the read out for a few seconds, it would be possible for the old calendar to be
    // downloaded before our changes are committed, presenting a very bad image to the user as his/her appointments
    // revert to the state they were in before the write (albiet temporarily)
    readLockout = true;

    // This needs to send each event separately; i.e. if two events were added they need
    // to be extracted and pushed on the stack independently (using two calls to writingQueuePush())

    Incidence::List added = addedIncidences();
    Incidence::List changed = changedIncidences();
    Incidence::List deleted = deletedIncidences();

    Incidence::List::ConstIterator it;
    Incidence::List currentIncidence;

    for( it = added.begin(); it != added.end(); ++it ) {
        WritingTask *t = new WritingTask;

        currentIncidence.clear();
        currentIncidence.append(*it);

        t->url = url;
        t->tasksUrl = tasksUrl;
        t->added = "";
        t->changed = "";
        t->deleted = "";
        t->tasksAdded = "";
        t->tasksChanged = "";
        t->tasksDeleted = "";
        if (getICalString(currentIncidence).contains("BEGIN:VEVENT") > 0)
          t->added = getICalString(currentIncidence);
        else if (getICalString(currentIncidence).contains("BEGIN:VTODO") > 0)
          t->tasksAdded = getICalString(currentIncidence);

        writingQueuePush(t);
    }

    for( it = changed.begin(); it != changed.end(); ++it ) {
        WritingTask *t = new WritingTask;

        currentIncidence.clear();
        currentIncidence.append(*it);

        t->url = url;
        t->tasksUrl = tasksUrl;
        t->added = "";
        t->changed = "";
        t->deleted = "";
        t->tasksAdded = "";
        t->tasksChanged = "";
        t->tasksDeleted = "";

        if (getICalString(currentIncidence).contains("BEGIN:VEVENT") > 0)
          t->changed = getICalString(currentIncidence);
        else if (getICalString(currentIncidence).contains("BEGIN:VTODO") > 0)
          t->tasksChanged = getICalString(currentIncidence);

        writingQueuePush(t);
    }

    for( it = deleted.begin(); it != deleted.end(); ++it ) {
        WritingTask *t = new WritingTask;

        currentIncidence.clear();
        currentIncidence.append(*it);

        t->url = url;
        t->tasksUrl = tasksUrl;
        t->added = "";
        t->changed = "";
        t->deleted = "";
        t->tasksAdded = "";
        t->tasksChanged = "";
        t->tasksDeleted = "";

        if (getICalString(currentIncidence).contains("BEGIN:VEVENT") > 0)
          t->deleted = getICalString(currentIncidence);
        else if (getICalString(currentIncidence).contains("BEGIN:VTODO") > 0)
          t->tasksDeleted = getICalString(currentIncidence);

        writingQueuePush(t);
    }

    return true;
}

void ResourceCalDav::writingFinished() {
    log("writing finished");

    if (!mWriter) {
        log("mWriter is NULL");
        return;
    }

    if (mWriter->error() && (abs(mWriter->errorNumber()) != 207)) {
        if (mWriter->errorNumber() == -401) {
            if (NULL != mPrefs) {
                TQCString newpass;
                if (KPasswordDialog::getPassword (newpass, TQString("<b>") + i18n("Remote authorization required") + TQString("</b><p>") + i18n("Please input the password for") + TQString(" ") + mPrefs->getusername(), NULL) != 1) {
                    log("write error: " + mWriter->errorString());
                    saveError(TQString("[%1] ").arg(abs(mWriter->errorNumber())) + mWriter->errorString());
                }
                else {
                    // Set new password and try again
                    mPrefs->setPassword(TQString(newpass));
                    startWriting(mPrefs->getFullUrl(), mPrefs->getFullTasksUrl());
                }
            }
            else {
                log("write error: " + mWriter->errorString());
                saveError(TQString("[%1] ").arg(abs(mWriter->errorNumber())) + mWriter->errorString());
            }
        }
        else {
            log("write error: " + mWriter->errorString());
            saveError(TQString("[%1] ").arg(abs(mWriter->errorNumber())) + mWriter->errorString());
        }
    } else {
        log("success");
        // is there something to do here?
    }

    // Give the remote system a few seconds to process the data before we allow any read operations
    TQTimer::singleShot( 3000, this, TQT_SLOT(releaseReadLockout()) );

    // Writing queue and mWritingQueueReady flag are not shared resources, i.e. only one thread has an access to them.
    // That's why no mutexes are required.
    mWriter->terminate();
    mWriter->wait(TERMINATION_WAITING_TIME);
    mWritingQueueReady = true;
    updateProgressBar(1);
    writingQueuePop();
}

// EOF ========================================================================
