/*=========================================================================
| KCardDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
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

#include <kabc/addressee.h>
#include <kabc/vcardconverter.h>

#include <klocale.h>
#include <kpassdlg.h>

#include <qdatetime.h>
#include <qmutex.h>
#include <qthread.h>
#include <qtimer.h>

#ifdef KCARDDAV_DEBUG
    #include <qfile.h>
#endif

#include "resource.h"
#include "reader.h"
#include "writer.h"

/*=========================================================================
| NAMESPACE
 ========================================================================*/

using namespace KABC;

/*=========================================================================
| CONSTANTS
 ========================================================================*/

const unsigned long ResourceCardDav::TERMINATION_WAITING_TIME = 3 * 1000; // 3 seconds
const int ResourceCardDav::CACHE_DAYS = 90;

const int ResourceCardDav::DEFAULT_RELOAD_INTERVAL   = 10;
const int ResourceCardDav::DEFAULT_SAVE_INTERVAL     = 10;
//const int ResourceCardDav::DEFAULT_RELOAD_POLICY     = ResourceCached::ReloadInterval;
//const int ResourceCardDav::DEFAULT_SAVE_POLICY       = ResourceCached::SaveDelayed;

/*=========================================================================
| UTILITY
 ========================================================================*/

#define log(s)  kdDebug() << identifier() << ": " << (s);

/*=========================================================================
| CONSTRUCTOR / DESTRUCTOR
 ========================================================================*/

ResourceCardDav::ResourceCardDav( const KConfig *config ) :
    ResourceCached(config)
    , mLock(true)
    , readLockout(false)
    , mAllWritesComplete(false)
    , mPrefs(NULL)
    , mLoader(NULL)
    , mWriter(NULL)
    , mProgress(NULL)
    , mLoadingQueueReady(true)
    , mWritingQueueReady(true)
    , mWriteRetryTimer(NULL)
{
    log("ResourceCardDav(config)");
    init();

    if ( config ) {
      readConfig( config );
    }
}

ResourceCardDav::~ResourceCardDav() {
    log("jobs termination");

    if (mWriteRetryTimer != NULL) {
        mWriteRetryTimer->stop();	// Unfortunately we cannot do anything at this point; if this timer is still running something is seriously wrong
    }

    while ((mWriter->running() == true) || (mWritingQueue.isEmpty() == false) || !mWritingQueueReady) {
        readLockout = true;
        sleep(1);
        qApp->processEvents(QEventLoop::ExcludeUserInput);
    }

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

bool ResourceCardDav::isSaving() {
    return (((mWriteRetryTimer != NULL) ? mWriteRetryTimer->isActive() : 0) || (mWriter->running() == true) || (mWritingQueue.isEmpty() == false) || !mWritingQueueReady);
}

/*=========================================================================
| GENERAL METHODS
 ========================================================================*/

bool ResourceCardDav::load() {
    bool syncCache = true;

    if ((mLoadingQueueReady == false) || (mLoadingQueue.isEmpty() == false) || (mLoader->running() == true) || (readLockout == true)) {
        return true;	// Silently fail; the user has obviously not responded to a dialog and we don't need to pop up more of them!
    }

    log(QString("doLoad(%1)").arg(syncCache));

    //clearCache();

    log("loading from cache");
    //disableChangeNotification();
    loadCache();
    //enableChangeNotification();
    clearChanges();
    addressBook()->emitAddressBookChanged();
    emit loadingFinished( this );

    log("starting download job");
    startLoading(mPrefs->getFullUrl());

    return true;
}

bool ResourceCardDav::doSave() {
    bool syncCache = true;

    log(QString("doSave(%1)").arg(syncCache));

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
    if (startWriting(mPrefs->getFullUrl()) == true) {
        log("clearing changes");
        // FIXME: Calling clearChanges() here is not the ideal way since the
        // upload might fail, but there is no other place to call it...
        clearChanges();
        return true;
    }
    else return true;	// We do not need to alert the user to this transient failure; a timer has been started to retry the save
}


bool ResourceCardDav::save( Ticket* ticket ) {
    // To suppress warning about doSave() method hides ResourceCached::doSave(Ticket)
    //return ResourceCached::doSave();
    return doSave();
}

KABC::Lock* ResourceCardDav::lock() {
    log("lock()");
    return &mLock;
}

void ResourceCardDav::readConfig( const KConfig *config ) {
    log("readConfig");
    mPrefs->readConfig();
    // FIXME KABC
    //ResourceCached::readConfig(config);
}

void ResourceCardDav::writeConfig( KConfig *config ) {
    log("writeConfig()");
    Resource::writeConfig(config);
    mPrefs->writeConfig();
    ResourceCached::writeConfig(config);
}

CardDavPrefs* ResourceCardDav::createPrefs() const {
    log("createPrefs()");
    CardDavPrefs* p = new CardDavPrefs(identifier());
    return p;
}

void ResourceCardDav::init() {
    // default settings
//     setReloadInterval(DEFAULT_RELOAD_INTERVAL);
//     setReloadPolicy(DEFAULT_RELOAD_POLICY);
//     setSaveInterval(DEFAULT_SAVE_INTERVAL);
//     setSavePolicy(DEFAULT_SAVE_POLICY);

    // creating preferences
    mPrefs = createPrefs();

    // creating reader/writer instances
    mLoader = new CardDavReader;
    mWriter = new CardDavWriter;

    // creating jobs
    // Qt4 handles this quite differently, as shown below,
    // whereas Qt3 needs events (see ::event())
//     connect(mLoader, SIGNAL(finished()), this, SLOT(loadFinished()));
//     connect(mWriter, SIGNAL(finished()), this, SLOT(writingFinished()));

    setType("ResourceCardDav");
}

void ResourceCardDav::ensureReadOnlyFlagHonored() {
    //disableChangeNotification();

    // FIXME KABC
    //Incidence::List inc( rawIncidences() );
    //setIncidencesReadOnly(inc, readOnly());

    //enableChangeNotification();

    addressBook()->emitAddressBookChanged();
}

void ResourceCardDav::setReadOnly(bool v) {
    KRES::Resource::setReadOnly(v);
    log("ensuring read only flag honored");
    ensureReadOnlyFlagHonored();
}

/*=========================================================================
| READING METHODS
 ========================================================================*/

void ResourceCardDav::loadingQueuePush(const LoadingTask *task) {
   if ((mLoadingQueue.isEmpty() == true) && (mLoader->running() == false)) {
        mLoadingQueue.enqueue(task);
        loadingQueuePop();
        if (mProgress == NULL) {
            mProgress = KPIM::ProgressManager::createProgressItem(KPIM::ProgressManager::getUniqueID(), i18n("Downloading Calendar") );
            mProgress->setProgress( 0 );
        }
    }
}

void ResourceCardDav::loadingQueuePop() {
    if (!mLoadingQueueReady || mLoadingQueue.isEmpty() || (mWritingQueue.isEmpty() == false) || (mWriter->running() == true) || !mWritingQueueReady) {
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
    mLoader->setParent(this);
    mLoader->setType(0);

    //QDateTime dt(QDate::currentDate());
    //mLoader->setRange(dt.addDays(-CACHE_DAYS), dt.addDays(CACHE_DAYS));
    //mLoader->setGetAll();

    mLoadingQueueReady = false;

    log("starting actual download job");
    mLoader->start(QThread::LowestPriority);

    // if all ok, removing the task from the queue
    mLoadingQueue.dequeue();

    delete t;
}

void ResourceCardDav::startLoading(const QString& url) {
    LoadingTask *t = new LoadingTask;
    t->url = url;
    loadingQueuePush(t);
}

void ResourceCardDav::loadFinished() {
    CardDavReader* loader = mLoader;

    log("load finished");

    if ( mProgress != NULL) {
        mProgress->setComplete();
        mProgress = NULL;
    }

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
                    addressBook()->error(QString("[%1] ").arg(abs(loader->errorNumber())) + loader->errorString());
                }
                else {
                    // Set new password and try again
                    mPrefs->setPassword(QString(newpass));
                    startLoading(mPrefs->getFullUrl());
                }
            }
            else {
                log("load error: " + loader->errorString() );
                addressBook()->error(QString("[%1] ").arg(abs(loader->errorNumber())) + loader->errorString());
            }
        }
        else {
            log("load error: " + loader->errorString() );
            addressBook()->error(QString("[%1] ").arg(abs(loader->errorNumber())) + loader->errorString());
        }
    } else {
        log("successful load");
        QString data = loader->data();

        if (!data.isNull() && !data.isEmpty()) {
            // TODO: I don't know why, but some schedules on http://carddav-test.ioda.net/ (I used it for testing)
            // have some lines separated by single \r rather than \n or \r\n.
            // ICalFormat fails to parse that.
            data.replace("\r\n", "\n"); // to avoid \r\n becomes \n\n after the next line
            data.replace('\r', '\n');

            log("trying to parse...");
            //printf("PARSING:\n\r%s\n\r", data.ascii());
            if (parseData(data)) {
                // FIXME: The agenda view can crash when a change is 
                // made on a remote server and a reload is requested!
                log("... parsing is ok");
                log("clearing changes");
                //enableChangeNotification();
                clearChanges();
                addressBook()->emitAddressBookChanged();
                emit loadingFinished( this );
            }
        }
    }

    // Loading queue and mLoadingQueueReady flag are not shared resources, i.e. only one thread has an access to them.
    // That's why no mutexes are required.
    mLoadingQueueReady = true;
    loadingQueuePop();
}

bool ResourceCardDav::checkData(const QString& data) {
    log("checking the data");

    KABC::VCardConverter converter;
    bool ret = true;
    KABC::VCardConverter conv;
    Addressee::List addressees = conv.parseVCards( data );
    if (addressees.isEmpty() == true) {
        ret = false;
    }

    return ret;
}

bool ResourceCardDav::parseData(const QString& data) {
    log("parseData()");

    bool ret = true;

    // check if the data is OK
    // May be it's not efficient (parsing is done twice), but it should be safe
    if (!checkData(data)) {
        addressBook()->error(i18n("Parsing calendar data failed."));
        return false;
    }

    // FIXME KABC
    //log("clearing cache");
    //clearCache();

    //disableChangeNotification();

    log("actually parsing the data");

    KABC::VCardConverter conv;
    Addressee::List addressees = conv.parseVCards( data );
    Addressee::List::ConstIterator it;
    for( it = addressees.begin(); it != addressees.end(); ++it ) {
      KABC::Addressee addr = *it;
      if ( !addr.isEmpty() ) {
        addr.setResource( this );

        insertAddressee( addr );
        clearChange( addr );
      }
    }

    // debug code here -------------------------------------------------------
#ifdef KCARDDAV_DEBUG
    const QString fout_path = "/tmp/kcarddav_download_" + identifier() + ".tmp";

    QFile fout(fout_path);
    if (fout.open(IO_WriteOnly | IO_Append)) {
        QTextStream sout(&fout);
        sout << "---------- " << resourceName() << ": --------------------------------\n";
        sout << data << "\n";
        fout.close();
    } else {
        addressBook()->error(i18n("can't open file"));
    }
#endif // KCARDDAV_DEBUG
    // end of debug code ----------------------------------------------------

    //enableChangeNotification();

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

Ticket *ResourceCardDav::requestSaveTicket()
{
  if ( !addressBook() ) {
    kdDebug(5700) << "no addressbook" << endl;
    return 0;
  }

  return createTicket( this );
}

void ResourceCardDav::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;
}

void ResourceCardDav::writingQueuePush(const WritingTask *task) {
//     printf("task->added: %s\n\r", task->added.ascii());
//     printf("task->deleted: %s\n\r", task->deleted.ascii());
//     printf("task->changed: %s\n\r", task->changed.ascii());
    mWritingQueue.enqueue(task);
    writingQueuePop();
    if (mProgress == NULL) {
        mProgress = KPIM::ProgressManager::createProgressItem(KPIM::ProgressManager::getUniqueID(), i18n("Saving Calendar") );
        mProgress->setProgress( 0 );
    }
}

void ResourceCardDav::writingQueuePop() {
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
    mWriter->setParent(this);
    mWriter->setType(1);

#ifdef KCARDDAV_DEBUG
    const QString fout_path = "/tmp/kcarddav_upload_" + identifier() + ".tmp";

    QFile fout(fout_path);
    if (fout.open(IO_WriteOnly | IO_Append)) {
        QTextStream sout(&fout);
        sout << "---------- " << resourceName() << ": --------------------------------\n";
        sout << "================== Added:\n" << t->added << "\n";
        sout << "================== Changed:\n" << t->changed << "\n";
        sout << "================== Deleted:\n" << t->deleted << "\n";
        fout.close();
    } else {
        addressBook()->error(i18n("can't open file"));
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
}

bool ResourceCardDav::event ( QEvent * e ) {
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

bool ResourceCardDav::startWriting(const QString& url) {
    log("startWriting: url = " + url);

    WritingTask *t = new WritingTask;
    KABC::VCardConverter converter;

    // WARNING: This will segfault if a separate read or write thread
    // modifies the calendar with clearChanges() or similar
    // Before these calls are made any existing read (and maybe write) threads should be finished
    if ((mLoader->running() == true) || (mLoadingQueue.isEmpty() == false) || (mWriter->running() == true) || (mWritingQueue.isEmpty() == false)) {
        if (mWriteRetryTimer == NULL) {
            mWriteRetryTimer = new QTimer(this);
            connect( mWriteRetryTimer, SIGNAL(timeout()), SLOT(doSave()) );
        }
        mWriteRetryTimer->start(1000, TRUE);
        return false;
    }

    KABC::Addressee::List added = addedAddressees();
    KABC::Addressee::List changed = changedAddressees();
    KABC::Addressee::List deleted = deletedAddressees();

    t->url = url;
    // FIXME KABC
    t->added = converter.createVCards(added);		// This crashes when an event is added from the remote server and save() is subsequently called
    t->changed = converter.createVCards(changed);
    t->deleted = converter.createVCards(deleted);

    writingQueuePush(t);

    return true;
}

void ResourceCardDav::writingFinished() {
    log("writing finished");

    if ( mProgress != NULL) {
        mProgress->setComplete();
        mProgress = NULL;
    }

    if (!mWriter) {
        log("mWriter is NULL");
        return;
    }

    if (mWriter->error() && (abs(mWriter->errorNumber()) != 207)) {
        if (mWriter->errorNumber() == -401) {
            if (NULL != mPrefs) {
                QCString newpass;
                if (KPasswordDialog::getPassword (newpass, QString("<b>") + i18n("Remote authorization required") + QString("</b><p>") + i18n("Please input the password for") + QString(" ") + mPrefs->getusername(), NULL) != 1) {
                    log("write error: " + mWriter->errorString());
                    addressBook()->error(QString("[%1] ").arg(abs(mWriter->errorNumber())) + mWriter->errorString());
                }
                else {
                    // Set new password and try again
                    mPrefs->setPassword(QString(newpass));
                    startWriting(mPrefs->getFullUrl());
                }
            }
            else {
                log("write error: " + mWriter->errorString());
                addressBook()->error(QString("[%1] ").arg(abs(mWriter->errorNumber())) + mWriter->errorString());
            }
        }
        else {
            log("write error: " + mWriter->errorString());
            addressBook()->error(QString("[%1] ").arg(abs(mWriter->errorNumber())) + mWriter->errorString());
        }
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
