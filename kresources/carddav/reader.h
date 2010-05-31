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

#include <qstring.h>
#include <qdatetime.h>

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
    CardDavReader(const QString& url = QString()) :
        CardDavJob(url)
        , mGetAll(true)
    {
    }

    /**
     * Sets a time range. Only event between @p start and @p end will be loaded.
     * This method call disables the effect of setGetAll() call.
     * setGetAll() call disables the effect of this method.
     */
    void setRange(const QDateTime& start, const QDateTime& end) {
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
    QString data() const {
        return mData;
    }

protected:

    virtual int runJob(runtime_info* caldavRuntime);

    virtual void cleanJob();

private:

    QString mData;
    bool mGetAll;
    QDateTime mTimeStart;
    QDateTime mTimeEnd;

};

} // namespace KABC

#endif // KABCDAV_LOADER_H

