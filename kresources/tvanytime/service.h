/***************************************************************************
 *   Copyright (C) 2005 by Will Stephenson   *
 *   wstephenson@kde.org   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/

#ifndef SERVICE_H
#define SERVICE_H

#include <tqdatetime.h>
#include <tqdom.h>
#include <tqmap.h>
#include <tqregexp.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <kurl.h>


class ProgramInformation {
  public:
    ProgramInformation() {}
    ProgramInformation( const TQString & title, const TQString &synopsis );
    virtual ~ProgramInformation() {}

    TQString id() const { return mId; }
    TQString title() const { return mTitle; }
    TQString synopsis() const { return mSynopsis; }
    TQStringList genres() const { return mGenres; }

    void setId ( const TQString & id ) { mId = id; }
    void setGenres( const TQStringList & genres ) { mGenres = genres; }
    bool loadXML( const TQDomElement & );
    bool loadAttribute( const TQDomElement& element );

  private:
    TQString mId;
    TQString mTitle;
    TQString mSynopsis;
    TQStringList mGenres;
};

typedef TQMap< TQString, ProgramInformation > ProgramInformationMap;

class ScheduleEvent {
  public:
    ScheduleEvent() {}
    virtual ~ScheduleEvent() {}
    
    TQString crid() const { return mCrid; }
    TQDateTime startTime() const { return mStartTime; }
    uint duration() const { return mDuration; }
    TQString programUrl() const { return mUrl; }

    bool loadXML( const TQDomElement & );
    bool loadAttribute( const TQDomElement& element );
    
  private:
    TQString mCrid;
    TQString mUrl;
    TQDateTime mStartTime;
    uint mDuration;

    static TQRegExp sRegExp;
};

class Service {
  public:
    Service();
    Service( bool active, const TQString & name, const TQString & owner, const KURL & serviceUrl, const KURL & logo, const TQStringList & genres );
    virtual ~Service() {}
  
    void setId( const TQString & id ) { mId = id; }
    void setActive( bool active );
    void setName( const TQString& name );
    void setProgramInformation( const ProgramInformationMap & map );

    TQString id() const { return mId; }
    bool active() const;
    TQString name() const;
    ProgramInformationMap programmeInformation() const;

    bool loadXML( const TQDomElement & );
    bool loadDescription( const TQDomElement & );
    bool loadAttribute( const TQDomElement& element );

  private:
    TQString mId;
    bool mActive;
    TQString mName;
    TQString mOwner;
    KURL mServiceUrl;
    KURL mLogo;
    TQStringList mGenres;
    ProgramInformationMap mProgInfo;
};

#endif
