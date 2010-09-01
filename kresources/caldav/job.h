/*=========================================================================
| KCalDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
| (c) 2009  Kumaran Santhanam (initial KDE4 version)
|
| This project is released under the GNU General Public License.
| Please see the file COPYING for more details.
|--------------------------------------------------------------------------
| Job class for accessing remote calendars.
 ========================================================================*/

/*=========================================================================
| INCLUDES
 ========================================================================*/

#ifndef KCALDAV_JOB_H
#define KCALDAV_JOB_H

#include <tqthread.h>
#include <tqstring.h>
#include <tqdatetime.h>
#include <tqapplication.h>

extern "C" {
    #include <libcaldav/caldav.h>
}

namespace KCal {

/*=========================================================================
| CLASS
 ========================================================================*/

/**
 * Calendar job.
 */
class CalDavJob : public TQThread {

public:

    /**
     * @param url URL to load.
     */
    CalDavJob(const TQString& url = TQString());

    virtual ~CalDavJob();

    /**
     * Sets a new URL to load.
     */
    virtual void setUrl(const TQString& s) {
        mUrl = s;
    }

    /**
     * Sets a new Tasks URL to load.
     */
    virtual void setTasksUrl(const TQString& s) {
        mTasksUrl = s;
    }

    /**
     * Sets a new Journals URL to load.
     */
    virtual void setJournalsUrl(const TQString& s) {
        mJournalsUrl = s;
    }

    /**
     * Sets the parent qobject.
     */
    virtual void setParent(TQObject *s) {
        mParent = s;
    }

    /**
     * Sets the type (0==read, 1==write)
     */
    virtual void setType(int s) {
        mType = s;
    }

    /**
     * @return URL to load.
     */
    virtual TQString url() const {
        return mUrl;
    }

    /**
     * @return Tasks URL to load.
     */
    virtual TQString tasksUrl() const {
        return mTasksUrl;
    }

    /**
     * @return Journals URL to load.
     */
    virtual TQString journalsUrl() const {
        return mJournalsUrl;
    }

    /**
     * @return parent object
     */
    virtual TQObject *parent() {
        return mParent;
    }

    /**
     * @return type
     */
    virtual int type() {
        return mType;
    }

    /**
     * @return true if events downloading process failed.
     */
    virtual bool error() const {
        return mError;
    }

    /**
     * @return true if tasks downloading process failed.
     */
    virtual bool tasksError() const {
        return mTasksError;
    }

    /**
     * @return true if journals downloading process failed.
     */
    virtual bool journalsError() const {
        return mJournalsError;
    }

    /**
     * @return an event error string.
     */
    virtual TQString errorString() const {
        return mErrorString;
    }

    /**
     * @return a task error string.
     */
    virtual TQString tasksErrorString() const {
        return mTasksErrorString;
    }

    /**
     * @return a journal error string.
     */
    virtual TQString journalsErrorString() const {
        return mJournalsErrorString;
    }

    /**
     * @return an event error number.
     */
    virtual long errorNumber() const {
        return mErrorNumber;
    }

    /**
     * @return a task error number.
     */
    virtual long tasksErrorNumber() const {
        return mTasksErrorNumber;
    }

    /**
     * @return a journal error number.
     */
    virtual long journalsErrorNumber() const {
        return mJournalsErrorNumber;
    }

protected:

    virtual void run();

    /**
     * Main run method for event jobs. Jobs should not override run() method.
     * Instead of this they should override this one.
     * @param caldavRuntime specific libcaldav runtime information. This pointer should not be saved for the usage
     * outside of runJob.
     * @return libcaldav response code (see CALDAV_RESPONSE)
     */
    virtual int runJob(runtime_info* caldavRuntime) = 0;

    /**
     * Main run method for task jobs. Jobs should not override run() method.
     * Instead of this they should override this one.
     * @param caldavRuntime specific libcaldav runtime information. This pointer should not be saved for the usage
     * outside of runJob.
     * @return libcaldav response code (see CALDAV_RESPONSE)
     */
    virtual int runTasksJob(runtime_info* caldavRuntime) = 0;

    /**
     * Main run method for journal jobs. Jobs should not override run() method.
     * Instead of this they should override this one.
     * @param caldavRuntime specific libcaldav runtime information. This pointer should not be saved for the usage
     * outside of runJob.
     * @return libcaldav response code (see CALDAV_RESPONSE)
     */
    virtual int runJournalsJob(runtime_info* caldavRuntime) = 0;

    /**
     * Some cleaning. Jobs may (and usually should) override this method.
     */
    virtual void cleanJob() {
        mError = false;
        mErrorString = "";
        mErrorNumber = 0;
        mTasksError = false;
        mTasksErrorString = "";
        mTasksErrorNumber = 0;
        mJournalsError = false;
        mJournalsErrorString = "";
        mJournalsErrorNumber = 0;
    }

    /**
     * Sets an event error string to @p err. Also sets an error flag.
     */
    void setErrorString(const TQString& str, const long number);

    /**
     * Sets a task error string to @p err. Also sets an error flag.
     */
    void setTasksErrorString(const TQString& str, const long number);

    /**
     * Sets a journal error string to @p err. Also sets an error flag.
     */
    void setJournalsErrorString(const TQString& str, const long number);

    /**
     * Process an event error.
     * Subclasses can overwrite this method, if some special error message handling
     * should be done. Call setErrorString() to set the error after processing is done.
     * @param err error structure.
     */
    virtual void processError(const caldav_error* err);

    /**
     * Process a task error.
     * Subclasses can overwrite this method, if some special error message handling
     * should be done. Call setErrorString() to set the error after processing is done.
     * @param err error structure.
     */
    virtual void processTasksError(const caldav_error* err);

    /**
     * Process a journal error.
     * Subclasses can overwrite this method, if some special error message handling
     * should be done. Call setErrorString() to set the error after processing is done.
     * @param err error structure.
     */
    virtual void processJournalsError(const caldav_error* err);

private:

    TQString mUrl;
    TQString mTasksUrl;
    TQString mJournalsUrl;
    bool mError;
    bool mTasksError;
    bool mJournalsError;
    TQString mErrorString;
    TQString mTasksErrorString;
    TQString mJournalsErrorString;
    long mErrorNumber;
    long mTasksErrorNumber;
    long mJournalsErrorNumber;
    TQObject *mParent;
    int mType;

    void enableCaldavDebug(runtime_info*);
};

} // namespace KCal

#endif // KCALDAV_JOB_H

