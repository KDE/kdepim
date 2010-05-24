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

#include <qthread.h>
#include <qstring.h>
#include <qdatetime.h>

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
class CalDavJob : public QThread {

public:

    /**
     * @param url URL to load.
     */
    CalDavJob(const QString& url = QString());

    virtual ~CalDavJob();

    /**
     * Sets a new URL to load.
     */
    virtual void setUrl(const QString& s) {
        mUrl = s;
    }

    /**
     * @return URL to load.
     */
    virtual QString url() const {
        return mUrl;
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
    virtual QString errorString() const {
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
    void setErrorString(const QString& str, const long number);

    /**
     * Process an error.
     * Subclasses can overwrite this method, if some special error message handling
     * should be done. Call setErrorString() to set the error after processing is done.
     * @param err error structure.
     */
    virtual void processError(const caldav_error* err);

private:

    QString mUrl;
    bool mError;
    QString mErrorString;
    long mErrorNumber;

    void enableCaldavDebug(runtime_info*);
};

} // namespace KCal

#endif // KCALDAV_JOB_H

