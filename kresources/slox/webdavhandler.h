/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef WEBDAVHANDLER_H
#define WEBDAVHANDLER_H

#include <qvaluelist.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qdom.h>

#include <kabc/addressee.h>

namespace KCal {
class Incidence;
}

class SloxItem
{
  public:
    enum Status { Invalid, Delete, Create };
    
    SloxItem();
    
    QDomNode domNode;
    QString sloxId;
    Status status;
};

class WebdavHandler
{
  public:
    WebdavHandler();

    void setUserId( const QString & );
    QString userId() const;

    void log( const QString & );

    static QDomElement addElement( QDomDocument &, QDomNode &,
                                   const QString &tag );
    static QDomElement addDavElement( QDomDocument &, QDomNode &,
                                      const QString &tag );
    static QDomElement addSloxElement( QDomDocument &, QDomNode &,
                                       const QString &tag,
                                       const QString &text = QString::null );

    static QDateTime sloxToQDateTime( const QString &str );
    static QDateTime sloxToQDateTime( const QString &str,
                                      const QString &timeZoneId );
    static QString qDateTimeToSlox( const QDateTime &dt );
    static QString qDateTimeToSlox( const QDateTime &dt,
                                    const QString &timeZoneId );

    static QValueList<SloxItem> getSloxItems( const QDomDocument &doc );

    void clearSloxAttributeStatus();
    void parseSloxAttribute( const QDomElement & );
    void setSloxAttributes( KCal::Incidence * );
    void setSloxAttributes( KABC::Addressee & );

  private:
    QString mLogFile;
    int mLogCount;

    QString mUserId;
    
    bool mWritable;
};

#endif
