#ifndef STOREDTRANSFERJOB_H
#define STOREDTRANSFERJOB_H

#include <kio/job.h>

// To be moved to KIO?
namespace KIOext {

/**
 * StoredTransferJob is a TransferJob (for downloading or uploading data) that
 * also stores a QByteArray with the data, making it simpler to use than the
 * standard TransferJob.
 *
 * For KIO::get it puts the data into the member QByteArray, so the user
 * of this class can get hold of the whole data at once by calling data()
 * when the result signal is emitted.
 * You should only use StoredTransferJob to download data if you cannot
 * process the data by chunks while it's being downloaded, since storing
 * everything in a QByteArray can potentially require a lot of memory.
 *
 * For KIO::put the user of this class simply provides the bytearray from
 * the start, and the job takes care of uploading it.
 * You should only use StoredTransferJob to upload data if you cannot
 * provide the in chunks while it's being uploaded, since storing
 * everything in a QByteArray can potentially require a lot of memory.
 *
 */
class StoredTransferJob : public KIO::TransferJob {
    Q_OBJECT

public:
       /**
	* Do not create a StoredTransferJob. Use storedGet() or storedPut()
	* instead.
	* @param url the url to get or put
	* @param command the command to issue
	* @param packedArgs the arguments
	* @param _staticData additional data to transmit (e.g. in a HTTP Post)
	* @param showProgressInfo true to show progress information to the user
	*/
    StoredTransferJob(const KURL& url, int command,
                      const QByteArray &packedArgs,
                      const QByteArray &_staticData,
                      bool showProgressInfo);

    /**
     * Set data to be uploaded. This is for put jobs.
     * Automatically called by KIOext::put(const QByteArray &, ...), do not call this yourself.
     */
    void setData( const QByteArray& arr );

    /**
     * Get hold of the downloaded data. This is for get jobs.
     * You're supposed to call this only from the slot connected to the result() signal.
     */
    QByteArray data() const { return m_data; }

private slots:
    void slotData( KIO::Job *job, const QByteArray &data );
    void slotDataReq( KIO::Job *job, QByteArray &data );
private:
    QByteArray m_data;
    int m_uploadOffset;
};

    /**
     * Get (a.k.a. read), into a single QByteArray.
     * @see StoredTransferJob
     *
     * @param url the URL of the file
     * @param reload true to reload the file, false if it can be taken from the cache
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation.
     */
    StoredTransferJob *storedGet( const KURL& url, bool reload=false, bool showProgressInfo = true );

    /**
     * Put (a.k.a. write) data from a single QByteArray.
     * @see StoredTransferJob
     *
     * @param url Where to write data.
     * @param permissions May be -1. In this case no special permission mode is set.
     * @param overwrite If true, any existing file will be overwritten.
     * @param resume true to resume, false otherwise
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation.
     * @see multi_get()
     */
    StoredTransferJob *put( const QByteArray& arr, const KURL& url, int permissions,
                            bool overwrite, bool resume, bool showProgressInfo = true );

} // namespace

#endif
