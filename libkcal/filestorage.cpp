/*
    This file is part of libkcal.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <stdlib.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qptrlist.h>

#include <kdebug.h>

#include "calendar.h"
#include "vcaldrag.h"
#include "vcalformat.h"
#include "icalformat.h"

#include "filestorage.h"

using namespace KCal;

FileStorage::FileStorage( Calendar *cal, const QString &fileName,
                            CalFormat *format )
  : CalStorage( cal ),
    mFileName( fileName ),
    mSaveFormat( format )
{
}

FileStorage::~FileStorage()
{
  delete mSaveFormat;
}

void FileStorage::setFileName( const QString &fileName )
{
  mFileName = fileName;
}

QString FileStorage::fileName()const
{
  return mFileName;
}


void FileStorage::setSaveFormat( CalFormat *format )
{
  delete mSaveFormat;
  mSaveFormat = format;
}

CalFormat *FileStorage::saveFormat()const
{
  return mSaveFormat;
}


bool FileStorage::open()
{
  return true;
}

bool FileStorage::load()
{
//  kdDebug(5800) << "FileStorage::load(): '" << mFileName << "'" << endl;

  // do we want to silently accept this, or make some noise?  Dunno...
  // it is a semantical thing vs. a practical thing.
  if (mFileName.isEmpty()) return false;

  // Always try to load with iCalendar. It will detect, if it is actually a
  // vCalendar file.
  ICalFormat iCal;

  bool success = iCal.load( calendar(), mFileName);

  if ( !success ) {
    if ( iCal.exception() ) {
//      kdDebug(5800) << "---Error: " << mFormat->exception()->errorCode() << endl;
      if ( iCal.exception()->errorCode() == ErrorFormat::CalVersion1 ) {
        // Expected non vCalendar file, but detected vCalendar
        kdDebug(5800) << "FileStorage::load() Fallback to VCalFormat" << endl;
        VCalFormat vCal;
        success = vCal.load( calendar(), mFileName );
        calendar()->setLoadedProductId( vCal.productId() );
      } else {
        return false;
      }
    } else {
      kdDebug(5800) << "Warning! There should be set an exception." << endl;
      return false;
    }
  } else {
//    kdDebug(5800) << "---Success" << endl;
    calendar()->setLoadedProductId( iCal.loadedProductId() );
  }

  calendar()->setModified( false );

  return true;
}

bool FileStorage::save()
{
  if ( mFileName.isEmpty() ) return false;

  CalFormat *format = 0;
  if ( mSaveFormat ) format = mSaveFormat;
  else format = new ICalFormat;

  bool success = format->save( calendar(), mFileName );

  if ( success ) {
    calendar()->setModified( false );
  } else {
    if ( !format->exception() ) {
      kdDebug(5800) << "FileStorage::save(): Error. There should be set an expection."
                << endl;
    } else {
      kdDebug(5800) << "FileStorage::save(): " << format->exception()->message()
                << endl;
    }
  }

  if ( !mSaveFormat ) delete format;

  return success;
}

bool FileStorage::close()
{
  return true;
}
