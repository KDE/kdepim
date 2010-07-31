/*=========================================================================
| KABCDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
|
| This project is released under the GNU General Public License.
| Please see the file COPYING for more details.
|--------------------------------------------------------------------------
| Remote address book loading class.
 ========================================================================*/

/*=========================================================================
| INCLUDES
 ========================================================================*/

#ifndef KABCDAV_LOADER_H
#define KABCDAV_LOADER_H

#include "job.h"

#include <tqstring.h>
#include <tqdatetime.h>

namespace KABC {

/*=========================================================================
| CLASS
 ========================================================================*/

/**
 * Calendar Reader.
 */
class CardDavReader : public CardDavJob {

public:

    /**
     * @param url URL to load.
     */
    CardDavReader(const TQString& url = TQString()) :
        CardDavJob(url)
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

protected:

    virtual int runJob(runtime_info* caldavRuntime);

    virtual void cleanJob();

private:

    TQString mData;
    bool mGetAll;
    TQDateTime mTimeStart;
    TQDateTime mTimeEnd;

};

} // namespace KABC

#endif // KABCDAV_LOADER_H

