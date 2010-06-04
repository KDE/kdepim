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

#ifndef KABC_RESOURCECARDDAV_H
#define KABC_RESOURCECARDDAV_H

#include "preferences.h"
#include <qthread.h>
#include <qptrqueue.h>

#include <kabcresourcecached.h>
#include <libkdepim/progressmanager.h>

#include <kabc/locknull.h>

#include <kdepimmacros.h>
#include <kconfig.h>

namespace KABC {

class CardDavReader;
class CardDavWriter;

/*=========================================================================
| CLASS
 ========================================================================*/

/**
 * This class provides a resource for accessing calendars via CardDAV protocol.
 */
class KDE_EXPORT ResourceCardDav : public ResourceCached
{
    Q_OBJECT

public:

    explicit ResourceCardDav( const KConfig *config );
    virtual ~ResourceCardDav();

    void readConfig( const KConfig *config );
    void writeConfig( KConfig *config );

    virtual Ticket *requestSaveTicket();
    virtual void releaseSaveTicket( Ticket* );

    /**
     * @return This resource preferences.
     */
    CardDavPrefs* prefs() {
        return mPrefs;
    }

    /**
     * @return This resource preferences.
     */
    const CardDavPrefs* prefs() const {
        return mPrefs;
    }

    virtual void setReadOnly(bool v);

    bool isSaving();

protected slots:

    void loadFinished();

    virtual bool doSave();

    void writingFinished();

protected:

    struct LoadingTask {
        QString url;
    };

    struct WritingTask {
        QString url;
        QString added;
        QString changed;
        QString deleted;
    };


//     virtual bool doLoad( bool syncCache );
//     virtual bool doSave( bool syncCache );

    virtual bool load();

    virtual bool save( Ticket* ticket );

    virtual KABC::Lock* lock();

    /**
     * Creates prefs and configures them.
     * @return a newly created preferences object. It should be removed by the caller.
     */
    CardDavPrefs* createPrefs() const;

    /**
     * Initializes internal state.
     * Particulary, sets save and reload policies to default values,
     * creates writing and reading jobs and preferences objects.
     */
    void init();

    /**
     * Updates the progress bar
     */
    void updateProgressBar(int direction);

    /**
     * Initiates calendar loading process.
     * @param url URL to load calendar data from.
     */
    void startLoading(const QString& url);

    /**
     * Checks if the data is correct and can be parsed.
     * @param data ical string to check.
     * @return true if the data is correct, false otherwise.
     */
    bool checkData(const QString& data);

    /**
     * Parses the data and adds events to the calendar.
     * @param data calendar data.
     * @return true on success, false on fail.
     */
    bool parseData(const QString& data);

    /**
     * Initiates calendar writing process.
     * @param url URL to save calendar data to.
     * @return true if write was queued successfully, false if not
     */
    bool startWriting(const QString& url);

    /**
     * Ensures incidences' read-only states are the same as the calendar's read-only state.
     */
    void ensureReadOnlyFlagHonored();

    /**
     * If the loading queue is empty or the loader is not ready, does nothing.
     * Otherwise, pops a head element and starts a loading process for it.
     */
    void loadingQueuePop();

    /**
     * Pushes the given loading task to the loading queue.
     * Then calls loadingQueuePop.
     */
    void loadingQueuePush(const LoadingTask *task);

    /**
     * If the writing queue is empty or the writer is not ready, does nothing.
     * Otherwise, pops a head element and starts a writing process for it.
     */
    void writingQueuePop();

    /**
     * Pushes the given writing task to the writing queue.
     * Then calls writingQueuePop.
     */
    void writingQueuePush(const WritingTask *task);

    virtual bool event ( QEvent * e );

private:

    // constants: =============================================================

    /// Termination waiting time in milliseconds. Used to terminate job threads.
    static const unsigned long TERMINATION_WAITING_TIME;

    /**
     * Resource caches only events which are from the interval [-CACHE_DAYS, CACHE_DAYS].
     */
    static const int CACHE_DAYS;

    static const int DEFAULT_RELOAD_INTERVAL;
    static const int DEFAULT_SAVE_INTERVAL;
    static const int DEFAULT_RELOAD_POLICY;
    static const int DEFAULT_SAVE_POLICY;

    bool readLockout;
    bool mAllWritesComplete;

    // members: ===============================================================

    KABC::LockNull mLock;
    CardDavPrefs* mPrefs;
    CardDavReader* mLoader;
    CardDavWriter* mWriter;
    KPIM::ProgressItem *mProgress;

    bool mLoadingQueueReady;
    QPtrQueue<LoadingTask> mLoadingQueue;

    bool mWritingQueueReady;
    QPtrQueue<WritingTask> mWritingQueue;

    QTimer *mWriteRetryTimer;

};



} // namespace KABC

#endif // KABC_RESOURCECARDDAV_H

