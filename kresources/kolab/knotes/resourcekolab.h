/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@klaralvdalens-datakonsult.se>

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

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef RESOURCEKOLAB_H
#define RESOURCEKOLAB_H

#include <resourcenotes.h>
#include <libkcal/incidencebase.h>
#include <libkcal/calendarlocal.h>
#include <resourcekolabbase.h>
#include <subresource.h>


namespace Kolab {

/**
 * This class implements a KNotes resource that keeps its
 * addresses in an IMAP folder in KMail (or other conforming email
 * clients).
 */
class ResourceKolab : public ResourceNotes,
                      public KCal::IncidenceBase::Observer,
                      public ResourceKolabBase
{
  Q_OBJECT

public:
  ResourceKolab( const KConfig* );
  virtual ~ResourceKolab();

  /** Load resource data. */
  bool load();

  /** Save resource data. */
  bool save();

  /** Open the notes resource. */
  bool doOpen();
  /** Close the notes resource. */
  void doClose();

  bool addNote( KCal::Journal* );

  bool deleteNote( KCal::Journal* );

  void incidenceUpdated( KCal::IncidenceBase* );

  // The IMAPBase methods called by KMail
  bool fromKMailAddIncidence( const QString& type, const QString& resource,
                              const QString& note );
  void fromKMailDelIncidence( const QString& type, const QString& resource,
                              const QString& note );
  void slotRefresh( const QString& type, const QString& resource );

  // Listen to KMail changes in the amount of sub resources
  void fromKMailAddSubresource( const QString& type, const QString& resource,
                                bool writable );
  void fromKMailDelSubresource( const QString& type, const QString& resource );

  /** Return the list of subresources. */
  QStringList subresources() const;

  /** Is this subresource active? */
  bool subresourceActive( const QString& ) const;

signals:
  void signalSubresourceAdded( Resource*, const QString&, const QString& );
  void signalSubresourceRemoved( Resource*, const QString&, const QString& );

private:
  bool addNote( KCal::Journal* journal, const QString& resource );

  bool loadResource( const QString& resource );

  QString configFile() const {
    return ResourceKolabBase::configFile( "knotes" );
  }

  // Parse a journal from a string
  KCal::Journal* parseJournal( const QString& str );
  KCal::CalendarLocal mCalendar;

  // The list of subresources
  Kolab::ResourceMap mResources;
  // Mapping from uid to resource
  QMap<QString, QString> mUidmap;
};

}

#endif // RESOURCEKOLAB_H
