/*
    This file is part of libkcal.

    Copyright (c) 2002,2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_FILESTORAGE_H
#define KCAL_FILESTORAGE_H

#include "calstorage.h"

namespace KCal {

class CalFormat;
/**
  This class provides a calendar storage as a local file.
*/
class LIBKCAL_EXPORT FileStorage : public CalStorage
{
  public:
    FileStorage( Calendar *, const QString &fileName = QString::null,
                  CalFormat *format = 0 );
    virtual ~FileStorage();

    void setFileName( const QString &mFileName );
    QString fileName() const;

    /**
      FileStorage takes ownership of format object.
    */
    void setSaveFormat( CalFormat * );
    CalFormat *saveFormat() const;

    bool open();
    bool load();
    bool save();
    bool close();

  private:
    QString mFileName;
    CalFormat *mSaveFormat;

    class Private;
    Private *d;
};

}

#endif
