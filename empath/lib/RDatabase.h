/*
  RDatabase - A persistent QAsciiDict<QByteArray>.

    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA. 
*/

#include <qdatetime.h>
#include <qdict.h>
#include <qfile.h>
#include <qstring.h>
#include <qcstring.h>
#include <qdatastream.h>

namespace RDB {

typedef QDict<Q_UINT32> Index;
typedef QDictIterator<Q_UINT32> IndexIterator;

/**
 * @short A persistent QAsciiDict<QByteArray>
 *
 * A persistent QAsciiDict<QByteArray>
 *
 * Warning: Not thread safe, no locks, and generally not safe at all.
 * Basically it works like GDBM, except it's all done with Qt, so it's
 * completely portable to wherever Qt works.
 *
 * @author Rik Hemsley <rik@kde.org>
 */
class Database
{
  public:

    /**
     * Create or operate on a database with the given name. The parameter
     * filename will have .idx and .rdb appended to create two files, an
     * index and a data file.
     */
    Database(const QString & filename);

    /**
     * The database is automatically flushed and closed when the dtor runs.
     */
    virtual ~Database();

    /**
     * @return The last error that occurred, in English.
     * Useful for debugging.
     */
    QString error() const;

    /**
     * Check this after each transaction you care about.
     * If it's false, look at error() to see what went wrong. Error
     * messages are in English.
     */
    bool ok() const;

    /**
     * @return false if the record could not be added
     */
    bool insert(const QString & key, const QByteArray & data);
    /**
     * @param ow If this comes back true, there was no such key and the
     * record has been added. Otherwise, the record was replaced.
     * @return false if the record could not be replaced
     */
    bool replace(const QString & key, const QByteArray & data, bool & ow);
    /**
     * @return false if the record could not be removed. This might
     * mean it did not exist.
     */
    bool remove(const QString & key);

    /**
     * @return the record referenced by the key. The record will be null
     * (use QByteArray::isNull() to test) if the record didn't exist.
     */
    QByteArray retrieve(const QString & key) const;

    /**
     * @return true if the key is in the index.
     */
    bool exists(const QString & key) const;

    /**
     * Write all data to disk NOW.
     */
    void sync();

    /**
     * Get rid of all unused space.
     * This creates two files with names the same as the filename you gave
     * in the ctor, with .idx_ and .rdb_ appended. These are deleted after
     * use. Make sure you have enough disk space for two copies of the
     * database !
     */
    void reorganise();

    /**
     * @return The index. This is a QAsciiDict<Q_UINT32>. The keys
     * are the keys, surprisingly. The values are the offsets in the
     * database file. This is useful for iterating through all keys
     * in the database.
     */
    const Index & index() const;

    /**
     * Clear out all entries.
     */
    void clear();

    QDateTime lastModified() const;

    void setUnread(unsigned int i) { unreadCount_ = i; } 
    void increaseUnreadCount()  { ++unreadCount_; }
    void decreaseUnreadCount()  { --unreadCount_; }
    unsigned int unreadCount()  { return unreadCount_; }

    void saveIndex() { _saveIndex(); }

  private:

    // Disabled default ctor, copy ctor and xxref.
    Database();
    Database(const Database &);
    Database & operator = (const Database &);

    void _setError(const QString & s) const;
    void _open();
    void _close();
    void _loadIndex();
    void _saveIndex();

    // Order dependency
    mutable bool ok_;
    bool indexLoaded_;
    bool indexDirty_;
    mutable QString error_;
    mutable QFile dataFile_;
    QFile indexFile_;
    Q_UINT32 offset_;
    Q_UINT32 indexFileSize_;
    Q_UINT32 dataFileSize_;
    // End order dependency

    Index index_;

    QDataStream indexStream_;
    mutable QDataStream dataStream_;

    QDateTime touched_;

    unsigned int unreadCount_;
};

} // End namespace

// vim:ts=2:sw=2:tw=78
