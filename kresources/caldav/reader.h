/*=========================================================================
| KCalDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
| (c) 2009  Kumaran Santhanam (initial KDE4 version)
|
| This project is released under the GNU General Public License.
| Please see the file COPYING for more details.
|--------------------------------------------------------------------------
| Remote calendar loading class.
 ========================================================================*/

/*=========================================================================
| INCLUDES
 ========================================================================*/

#ifndef KCALDAV_LOADER_H
#define KCALDAV_LOADER_H

#include "job.h"

#include <tqstring.h>
#include <tqdatetime.h>

namespace KCal {

/*=========================================================================
| CLASS
 ========================================================================*/

/**
 * Calendar Reader.
 */
class CalDavReader : public CalDavJob {

public:

    /**
     * @param url URL to load.
     */
    CalDavReader(const TQString& url = TQString()) :
        CalDavJob(url)
        , mGetAll(true)
    {
    }

    /**
     * Sets a time range. Only event between @p start and @p end will be loaded.
     * This method call disables the effect of setGetAll() call.
     * setGetAll() call disables the effect of this method.
     */
    void setRange(const TQDateTime& start, const TQDateTime& end) {
        mGetAll = false;
        mTimeStart = start;
        mTimeEnd = end;
    }

    /**
     * Sets the flag to load all events from the remote calendar.
     * This method call disables the effect of setRange() call.
     * setGetAll() call disables the effect of this method.
     */
    void setGetAll() {
        mGetAll = true;
    }

    /**
     * @return downloaded calendar data in iCal format.
     */
    TQString data() const {
        return mData;
    }

    /**
     * @return downloaded task data in iCal format.
     */
    TQString tasksData() const {
        return mTasksData;
    }

protected:

    virtual int runJob(runtime_info* caldavRuntime);
    virtual int runTasksJob(runtime_info* caldavRuntime);

    virtual void cleanJob();
    virtual void cleanTasksJob();

private:

    TQString mData;
    TQString mTasksData;
    bool mGetAll;
    TQDateTime mTimeStart;
    TQDateTime mTimeEnd;

};

} // namespace KCal

#endif // KCALDAV_LOADER_H

