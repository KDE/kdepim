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

#ifndef KCAL_RESOURCECALDAV_H
#define KCAL_RESOURCECALDAV_H

#include "preferences.h"
#include <tqthread.h>
#include <tqptrqueue.h>

#include <libkcal/resourcecached.h>
#include <libkdepim/progressmanager.h>

#include <kabc/locknull.h>

#include <kdepimmacros.h>
#include <kconfig.h>

namespace KCal {

class CalDavReader;
class CalDavWriter;

/*=========================================================================
| CLASS
 ========================================================================*/

/**
 * This class provides a resource for accessing calendars via CalDAV protocol.
 */
class KDE_EXPORT ResourceCalDav : public ResourceCached
{
    Q_OBJECT

public:

    explicit ResourceCalDav( const KConfig *config );
    virtual ~ResourceCalDav();

    void readConfig( const KConfig *config );
    void writeConfig( KConfig *config );

    /**
     * @return This resource preferences.
     */
    CalDavPrefs* prefs() {
        return mPrefs;
    }

    /**
     * @return This resource preferences.
     */
    const CalDavPrefs* prefs() const {
        return mPrefs;
    }

    virtual void setReadOnly(bool v);

    bool isSaving();

protected slots:

    void loadFinished();

    virtual bool doSave();

    void writingFinished();

    void releaseReadLockout();

protected:

    struct LoadingTask {
        TQString url;
        TQString tasksUrl;
        TQString journalsUrl;
    };

    struct WritingTask {
        TQString url;
        TQString tasksUrl;
        TQString journalsUrl;

        TQString added;
        TQString changed;
        TQString deleted;

        TQString tasksAdded;
        TQString tasksChanged;
        TQString tasksDeleted;

        TQString journalsAdded;
        TQString journalsChanged;
        TQString journalsDeleted;
    };


    virtual bool doLoad();
//    virtual bool doSave();

    virtual KABC::Lock* lock();

    /**
     * Creates prefs and configures them.
     * @return a newly created preferences object. It should be removed by the caller.
     */
    CalDavPrefs* createPrefs() const;

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
     * @param tasksUrl URL to load task data from.
     * @param journalsUrl URL to load journal data from.
     */
    void startLoading(const TQString& url, const TQString& tasksUrl, const TQString& journalsUrl);

    /**
     * Checks if the data is correct and can be parsed.
     * @param data ical string to check.
     * @return true if the data is correct, false otherwise.
     */
    bool checkData(const TQString& data);

    /**
     * Parses the data and adds events to the calendar.
     * @param data calendar data.
     * @return true on success, false on fail.
     */
    bool parseData(const TQString& data);

    /**
     * Parses the data and adds tasks to the calendar.
     * @param data calendar data.
     * @return true on success, false on fail.
     */
    bool parseTasksData(const TQString& data);

    /**
     * Parses the data and adds journals to the calendar.
     * @param data calendar data.
     * @return true on success, false on fail.
     */
    bool parseJournalsData(const TQString& data);

    /**
     * Initiates calendar writing process.
     * @param url URL to save event data to.
     * @param tasksUrl URL to save task data to.
     * @param journalsUrl URL to save journal data to.
     * @return true if write was queued successfully, false if not
     */
    bool startWriting(const TQString& url, const TQString& tasksUrl, const TQString& journalsUrl);

    /**
     * Returns a list of incidences as a valid iCalendar string.
     * @param inc list of incidences.
     * @return a string in iCalendar format which describes the given incidences.
     */
    TQString getICalString(const Incidence::List& inc);

    /**
     * Changes read-only status of incidences from a given list.
     * @param inc list of incidences.
     * @param readOnly read-only status that all the incidences should have after the method finishes.
     */
    static void setIncidencesReadOnly(Incidence::List& inc, bool readOnly);

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

    virtual bool event ( TQEvent * e );

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
    CalDavPrefs* mPrefs;
    CalDavReader* mLoader;
    CalDavWriter* mWriter;
    KPIM::ProgressItem *mProgress;

    bool mLoadingQueueReady;
    TQPtrQueue<LoadingTask> mLoadingQueue;

    bool mWritingQueueReady;
    TQPtrQueue<WritingTask> mWritingQueue;

    TQTimer *mWriteRetryTimer;

};



} // namespace KCal

#endif // KCAL_RESOURCECALDAV_H

