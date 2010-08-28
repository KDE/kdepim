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
     * @return true if downloading process failed.
     */
    virtual bool error() const {
        return mError;
    }

    /**
     * @return an error string.
     */
    virtual TQString errorString() const {
        return mErrorString;
    }

    /**
     * @return an error number.
     */
    virtual long errorNumber() const {
        return mErrorNumber;
    }

protected:

    virtual void run();

    /**
     * Main run method for jobs. Jobs should not override run() method.
     * Instead of this they should override this one.
     * @param caldavRuntime specific libcaldav runtime information. This pointer should not be saved for the usage
     * outside of runJob.
     * @return libcaldav response code (see CALDAV_RESPONSE)
     */
    virtual int runJob(runtime_info* caldavRuntime) = 0;

    /**
     * Some cleaning. Jobs may (and usually should) override this method.
     */
    virtual void cleanJob() {
        mError = false;
        mErrorString = "";
        mErrorNumber = 0;
    }

    /**
     * Sets an error string to @p err. Also sets an error flag.
     */
    void setErrorString(const TQString& str, const long number);

    /**
     * Process an error.
     * Subclasses can overwrite this method, if some special error message handling
     * should be done. Call setErrorString() to set the error after processing is done.
     * @param err error structure.
     */
    virtual void processError(const caldav_error* err);

private:

    TQString mUrl;
    TQString mTasksUrl;
    bool mError;
    TQString mErrorString;
    long mErrorNumber;
    TQObject *mParent;
    int mType;

    void enableCaldavDebug(runtime_info*);
};

} // namespace KCal

#endif // KCALDAV_JOB_H

