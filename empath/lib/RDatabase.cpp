/*
  RDatabase - A persistent QAsciiDict<QByteArray>.
  
  Copyright (C) 1999 Rik Hemsley rik@kde.org
 
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

#include <qdir.h>

#include "RDatabase.h"

namespace RDB
{

Database::Database(const QString & filename)
  : ok_(false),
    indexLoaded_(false),
    indexDirty_(false),
    error_("No error"),
    offset_(0),
    indexFileSize_(0),
    dataFileSize_(0)
{
  indexFile_.setName(filename + ".idx");
  dataFile_.setName(filename + ".rdb");

  index_.setAutoDelete(true);
  _open();
  touched_ = QDateTime::currentDateTime();
}

Database::~Database()
{
  // Must save the index before we die.
  _saveIndex();
  _close();
}

  void
Database::_setError(const QString & s)
{
  ok_ = false;
  error_ = s;
}

  QString
Database::error() const
{
  return error_;
}
    
  bool
Database::ok() const
{
  return ok_;
}

  const Index &
Database::index() const
{
  return index_;
}

  void
Database::_open()
{
  ok_ = true;

  if (!indexFile_.open(IO_ReadWrite)) {
    _setError("Cannot open index file");
    return;
  }
 
  if (!dataFile_.open(IO_ReadWrite)) {
    _setError("Cannot open data file");
    return;
  }

  // Remember where the end of the data file is.
  dataFileSize_   = dataFile_.size();

  indexStream_  .setDevice(&indexFile_);
  dataStream_   .setDevice(&dataFile_);
  
  _loadIndex();
}

  void
Database::_loadIndex()
{
  if (indexLoaded_)
    return;

  index_.clear();
  
  indexFile_.reset();
  indexStream_ >> offset_;

  QCString key;
  Q_UINT32 ofs;
  
  while (!indexStream_.eof()) {
    indexStream_ >> key >> ofs;
    index_.insert(key, new Q_UINT32(ofs));
  }

  touched_ = QDateTime::currentDateTime();
  indexDirty_ = false;
  indexLoaded_ = true;
}

  void
Database::_saveIndex()
{
  if (!indexDirty_)
    return;

  indexFile_.reset();
  indexStream_ << offset_;

  IndexIterator it(index_);
  
  for (; it.current(); ++it)
    indexStream_ << it.currentKey() << *(it.current());

  indexDirty_ = false;
}

  void
Database::_close()
{
  if (indexFile_.isOpen()) {
    indexFile_.flush();
    indexFile_.close();

    if (indexFile_.status() != IO_Ok) {
      _setError("Cannot close index file");
      return;
    }
  }
  
  if (dataFile_.isOpen()) {
    dataFile_.flush();
    dataFile_.close();
    
    if (dataFile_.status() != IO_Ok) {
      _setError("Cannot close data file");
      return;
    }
  }
}

  bool
Database::insert(const QCString & key, const QByteArray & data)
{
  ok_ = true;

  if (index_[key] != 0) {
    _setError("Record `" + key + "' exists");
    return false;
  }
  
  index_.insert(key, new Q_UINT32(dataFileSize_));
  indexDirty_ = true;
  
  dataFile_.at(dataFileSize_);
  dataStream_ << data;

  dataFileSize_ = dataFile_.at();

  touched_ = QDateTime::currentDateTime();
  return true;
}

  bool
Database::exists(const QCString & key)
{
  return (index_[key] != 0);
}

  QByteArray
Database::retrieve(const QCString & key)
{
  ok_ = true;

  QByteArray data;

  Q_UINT32 * ofs = index_[key];

  if (ofs == 0) {
    _setError("Record `" + key + "' not found in index file");
    return data;
  }

  if (*ofs > dataFileSize_) {
    _setError("Attempt to seek past end of file");
    return data;
  }

  dataFile_.at(*ofs);

  dataStream_ >> data;

  if (data.isNull()) {
    _setError("Record `" + key + "' not found in data file");
    return data;
  }

  return data;
}

  bool
Database::remove(const QCString & key)
{
  ok_ = true;
  bool removed = index_.remove(key);
  indexDirty_ = removed;
  if (removed)
    touched_ = QDateTime::currentDateTime();
  return removed;
}

  bool
Database::replace(const QCString & key, const QByteArray & data, bool & ow)
{
  ok_ = true;

  Q_UINT32 * ofs = index_[key];
  
  if (ofs == 0) {

    ow = false;

    index_.insert(key, new Q_UINT32(dataFileSize_));
    indexDirty_ = true;
  
    dataFile_.at(dataFileSize_);
    dataStream_ << data;

    dataFileSize_ = dataFile_.at();

    touched_ = QDateTime::currentDateTime();
    return true;
  }

  ow = true;

  QByteArray originalRecord;
  
  dataFile_.at(*ofs);

  Q_UINT32 originalRecordLength;

#ifdef NAUGHTY_NAUGHTY

  // Cheat - we know the serialisation format of QByteArray.
  dataStream_ >> originalRecordLength;

#else
  
  dataStream_ >> originalRecord;

  if (originalRecord.isNull()) {
    _setError("Original record for `" + key + "' not found in data file");
    return false;
  }

  originalRecordLength = originalRecord.size();

#endif

  Q_UINT32 newRecordLength = data.size();
  

  // If the replacement is larger than the original, we remove
  // the key for original, and add the new record at the end of the
  // data file.

  if (newRecordLength > originalRecordLength) {
  
    index_.replace(key, new Q_UINT32(dataFileSize_));
    indexDirty_ = true;
    
    dataFile_.at(dataFileSize_);
    dataStream_ << data;
  
    dataFileSize_ = dataFile_.at();
   
    return true;
  }

  // The replacement is the same size as, or smaller than, the original.
  // We may overwrite the original.

  dataFile_.at(*ofs);
  dataStream_ << data;

  dataFileSize_ = dataFile_.at();
  
  return true;
}

  void
Database::sync()
{
  _saveIndex();
  indexFile_.flush();
  dataFile_.flush();
  touched_ = QDateTime::currentDateTime();
}

  void
Database::reorganise()
{
  // Create a new index and a new data file.
  // Copy all records one by one from the original data file to the
  // new one. Next write the index to the new index file.
  // Lastly, remove the old files and rename the new ones with
  // the original names.
  
  // TODO: Sort the index by offset, to reduce seeks.
  
  QString indexFilename = indexFile_.name();
  QString dataFilename = dataFile_.name();
  
  QFile indexf(indexFilename + "_");
  QFile dataf(dataFilename + "_");

  if (!indexf.open(IO_WriteOnly | IO_Truncate)) {
    _setError("Could not open file `" + indexf.name() + "'");
    return;
  }

  if (!dataf.open(IO_WriteOnly | IO_Truncate)) {
    _setError("Could not open file `" + dataf.name() + "'");
    return;
  }

  QDataStream istr(&indexf);
  istr << offset_;

  QDataStream dstr(&dataf);

  unsigned int cursor(0);

  IndexIterator it(index_);

  for (; it.current(); ++it) {

    QByteArray data;

    dataFile_.at(*(it.current()));

    dataStream_ >> data;

    if (data.isNull()) {
      _setError
        ("Record `" + QCString(it.currentKey()) + "' not found in data file");
      continue;
    }

    cursor = dataf.at();
    
    dstr << data;
    istr << it.currentKey() << cursor;
  }

  indexf.flush();
  dataf.flush();

  indexf.close();
  dataf.close();

  if (indexf.status() != IO_Ok) {
    _setError("Could not flush and close temporary index file");
    return;
  }
  
  if (dataf.status() != IO_Ok) {
    _setError("Could not flush and close temporary data file");
    return;
  }

  indexFile_.close();
  dataFile_.close();

  indexFile_.remove();
  dataFile_.remove();

  QDir d;
  
  bool renamed = d.rename(indexFilename + "_", indexFilename, true);
  
  if (!renamed) {
    _setError
      ("Could not rename `" + indexFilename + "_' to `" + indexFilename + "'");
    return;
  }

  renamed = d.rename(dataFilename + "_", dataFilename, true);
  
  if (!renamed) {
    _setError
      ("Could not rename `" + dataFilename + "_' to `" + dataFilename + "'");
    return;
  }

  if (!indexFile_.open(IO_ReadWrite)) {
    _setError("Could not reopen index file");
    return;
  }

  indexLoaded_ = false;
  _loadIndex();
  
  if (!dataFile_.open(IO_ReadWrite)) {
    _setError("Could not reopen data file");
    return;
  }
}

  QDateTime
Database::lastModified() const
{
  return touched_;
}

  void
Database::clear()
{
  index_.clear();
 
  indexFile_.remove();
  dataFile_.remove();

  if (!indexFile_.open(IO_ReadWrite)) {
    _setError("Could not reopen index file");
    return;
  }

  indexLoaded_ = false;
  _loadIndex();
  
  if (!dataFile_.open(IO_ReadWrite)) {
    _setError("Could not reopen data file");
    return;
  }
 
  touched_ = QDateTime::currentDateTime();
}

} // End namespace

// vim:ts=2:sw=2:tw=78
