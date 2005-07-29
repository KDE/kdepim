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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef SERVICE_H
#define SERVICE_H

#include <qdatetime.h>
#include <qdom.h>
#include <qmap.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <kurl.h>


class ProgramInformation {
  public:
    ProgramInformation() {}
    ProgramInformation( const QString & title, const QString &synopsis );
    virtual ~ProgramInformation() {}

    QString id() const { return mId; }
    QString title() const { return mTitle; }
    QString synopsis() const { return mSynopsis; }
    QStringList genres() const { return mGenres; }

    void setId ( const QString & id ) { mId = id; }
    void setGenres( const QStringList & genres ) { mGenres = genres; }
    bool loadXML( const QDomElement & );
    bool loadAttribute( const QDomElement& element );

  private:
    QString mId;
    QString mTitle;
    QString mSynopsis;
    QStringList mGenres;
};

typedef QMap< QString, ProgramInformation > ProgramInformationMap;

class ScheduleEvent {
  public:
    ScheduleEvent() {}
    virtual ~ScheduleEvent() {}
    
    QString crid() const { return mCrid; }
    QDateTime startTime() const { return mStartTime; }
    uint duration() const { return mDuration; }
    QString programUrl() const { return mUrl; }

    bool loadXML( const QDomElement & );
    bool loadAttribute( const QDomElement& element );
    
  private:
    QString mCrid;
    QString mUrl;
    QDateTime mStartTime;
    uint mDuration;

    static QRegExp sRegExp;
};

class Service {
  public:
    Service();
    Service( bool active, const QString & name, const QString & owner, const KURL & serviceUrl, const KURL & logo, const QStringList & genres );
    virtual ~Service() {}
  
    void setId( const QString & id ) { mId = id; }
    void setActive( bool active );
    void setName( const QString& name );
    void setProgramInformation( const ProgramInformationMap & map );

    QString id() const { return mId; }
    bool active() const;
    QString name() const;
    ProgramInformationMap programmeInformation() const;

    bool loadXML( const QDomElement & );
    bool loadDescription( const QDomElement & );
    bool loadAttribute( const QDomElement& element );

  private:
    QString mId;
    bool mActive;
    QString mName;
    QString mOwner;
    KURL mServiceUrl;
    KURL mLogo;
    QStringList mGenres;
    ProgramInformationMap mProgInfo;
};

#endif
