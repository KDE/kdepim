/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>

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

#include <config.h>

#include <kstandarddirs.h>

#include "helper.h"

using namespace OpieHelper;

Base::Base( CategoryEdit* edit,
            KSync::KonnectorUIDHelper* helper,
            const QString &tz,
            Device* dev )
{
    m_edit = edit;
    m_helper = helper;
    m_tz = tz;
    m_device = dev;
}
Base::~Base()
{

}
QDateTime Base::fromUTC( time_t time )
{
   struct tm *lt;

   /* getenv can be NULL */
   char* ptrTz = getenv( "TZ");
   QString real_TZ = ptrTz ? QString::fromLocal8Bit( ptrTz ) : QString::null;

   if (!m_tz.isEmpty() )
       setenv( "TZ", m_tz.local8Bit(), true );

#if defined(_OS_WIN32) || defined (Q_OS_WIN32) || defined (Q_OS_WIN64)
    _tzset();
#else
    tzset();
#endif
    lt = localtime( &time );
    QDateTime dt;
    dt.setDate( QDate( lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday ) );
    dt.setTime( QTime( lt->tm_hour, lt->tm_min, lt->tm_sec ) );

    if (!m_tz.isEmpty() ) {
        unsetenv("TZ");
        if (!real_TZ.isEmpty() )
            setenv("TZ",  real_TZ.local8Bit(), true );
    }
    // done
    return dt;
}
time_t Base::toUTC( const QDateTime& dt )
{
    time_t tmp;
    struct tm *lt;

    /* getenv can be NULL */
    char* ptrTz = getenv( "TZ");
    QString real_TZ = ptrTz ? QString::fromLocal8Bit( getenv("TZ") ) : QString::null;

    if ( !m_tz.isEmpty() )
        setenv( "TZ", m_tz.local8Bit(), true );

#if defined(_OS_WIN32) || defined (Q_OS_WIN32) || defined (Q_OS_WIN64)
    _tzset();
#else
    tzset();
#endif

    // get a tm structure from the system to get the correct tz_name
    tmp = time( 0 );
    lt = localtime( &tmp );

    lt->tm_sec = dt.time().second();
    lt->tm_min = dt.time().minute();
    lt->tm_hour = dt.time().hour();
    lt->tm_mday = dt.date().day();
    lt->tm_mon = dt.date().month() - 1; // 0-11 instead of 1-12
    lt->tm_year = dt.date().year() - 1900; // year - 1900
    //lt->tm_wday = dt.date().dayOfWeek(); ignored anyway
    //lt->tm_yday = dt.date().dayOfYear(); ignored anyway
    lt->tm_wday = -1;
    lt->tm_yday = -1;
    // tm_isdst negative -> mktime will find out about DST
    lt->tm_isdst = -1;
    // keep tm_zone and tm_gmtoff
    tmp = mktime( lt );

    if (!m_tz.isEmpty() ) {
        unsetenv("TZ");
        if (!real_TZ.isEmpty() )
            setenv("TZ",  real_TZ.local8Bit(), true );
    }
    return tmp;
}


KTempFile* Base::file() {
    KTempFile* fi = new KTempFile( locateLocal("tmp",  "opie-konnector"),  "new");
    return fi;
}
QString Base::categoriesToNumber( const QStringList &list, const QString &app )
{
 startover:
    QStringList dummy;
    QValueList<OpieCategories>::ConstIterator catIt;
    QValueList<OpieCategories> categories = m_edit->categories();
    bool found = false;
    for ( QStringList::ConstIterator listIt = list.begin(); listIt != list.end(); ++listIt ) {
        /* skip empty category name */
        if ( (*listIt).isEmpty() ) continue;

        found  = false;
        for ( catIt = categories.begin(); catIt != categories.end(); ++catIt ) {
	    /*
	     * We currently do not take app into account
	     * if name matches and the id isn't already in dummy we'll add it
	     */
            if ( (*catIt).name() == (*listIt) && !dummy.contains(( *catIt).id() )  ) { // the same name
                found= true;
                dummy << (*catIt).id();
            }
        }
        /* if not found and the category is not empty
         *
         * generate a new category and start over again
         * ugly goto to reiterate
         */

        if ( !found && !(*listIt).isEmpty() ){
            m_edit->addCategory( app, (*listIt) );  // generate a new category
            goto startover;
	}
    }

    return dummy.join(";");
}
QString Base::konnectorId( const QString &appName,  const QString &uid )
{
    QString id;
    QString id2;
    // Konnector-.length() ==  10
    if ( uid.startsWith( "Konnector-" ) ) { // not converted
        id2 =  uid.mid( 10 );
    }else if ( m_helper) {
        id =  m_helper->konnectorId( appName,  uid );
        //                        konnector kde
        if (id.isEmpty() ) { // generate new id
            id2 = QString::number( newId() );
            id = QString::fromLatin1("Konnector-") + id2;
        }else if ( id.startsWith( "Konnector-" ) ) { // not converted
            id2 =  id.mid( 10 );
        }
        m_kde2opie.append( Kontainer( id,     uid ) );
    }
    return id2;
}
/*
 * IntelliSync(tm) is completely broken in regards to assigning UID's
 * it's always assigning the 0. So for us to work properly we need to rely
 * on uids!
 * We'll see if it equals '0' and then prolly assign a new uid
 */
QString Base::kdeId( const QString &appName,  const QString &_uid )
{
    QString uid = _uid;
    if (_uid.stripWhiteSpace() == QString::fromLatin1("0") ) {
        uid = QString::number( newId() );
    }

    QString ret;
    if ( !m_helper )
        ret = QString::fromLatin1("Konnector-")  + uid;

    else // only if meta
        ret = m_helper->kdeId( appName, "Konnector-"+uid,  "Konnector-"+uid);

    return ret;
}
// code copyrighted by tt FIXME
// GPL from Qtopia
int Base::newId()
{
    static QMap<int,  bool> ids;
    int id = -1 * (int) ::time(NULL );
    while ( ids.contains( id ) ){
        id += -1;
        if ( id > 0 )
            id = -1;
    }
    ids.insert( id, true );
    return id;
}
const Device* Base::device() {
    return m_device;
}

/**
 * Append Text if text is not the default. This way we don't have
 * empty attributes in the XML file, this makes the file smaller
 * and transfer over slower connections faster
 */
QString Base::appendText( const QString& append, const QString& def )
{
  if ( append != def )
    return append;

  return QString::null;
}


// FROM TT QStyleSheet and StringUtil it's GPLed
// we also need to escape '\"' for our xml files
QString OpieHelper::escape( const QString& plain )
{
  QString rich;

  for ( int i = 0; i < int(plain.length()); ++i ) {
    if ( plain[i] == '<' )
      rich +="&lt;";
    else if ( plain[i] == '>' )
      rich +="&gt;";
    else if ( plain[i] == '&' )
      rich +="&amp;";
    else if ( plain[i] == '\"' )
      rich += "&quot;";
    else
      rich += plain[i];
  }

  return rich;
}
