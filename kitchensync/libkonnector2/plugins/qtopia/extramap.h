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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KSYNC_OPIE_EXTRA_TAGS_MAP_H
#define KSYNC_OPIE_EXTRA_TAGS_MAP_H

#include <qmap.h>
#include <qstring.h>


class QDomNamedNodeMap;
class QStringList;
namespace OpieHelper {
/**
 * Base class for every Custom Item
 * that you may want to save to store
 * Information during a sync
 */
struct CustomExtraItem 
{
  virtual ~CustomExtraItem();
};

/**
 * Whenever we do not handle a XML attribute we need
 * to save it somewhere and when we write we need to flush
 * the tags to file
 * This way we won't loose changes
 */

/**
 *used to save on a Key Value basis
 */
typedef QString CUID; // ComposedUID
typedef QMap<QString, QString> KeyValue;
typedef QMap<CUID, KeyValue> ExtraMapBase;

struct ExtraMap : public ExtraMapBase 
{
  ~ExtraMap();
  
  /* not virtual but works here */
  /**
   * Clear and free internal structures. On destruction clar()
   * is called.
   */
  void clear();
  
  /**
   * Converts the KeyValue
   */
  QString toString( const CUID& );
  
  /**
   * assembles 'app-uid' and converts the stuff to additional attributes
   */
  QString toString( const QString& app, const QString& uid );
  
  /**
   * add a CUID with keyValue
   * @param app the Application
   * @param uid The uid
   * @param map The AttributeMap
   * @param lst The list of handled attributes
   */
  void add(const QString& app, const QString& uid, const QDomNamedNodeMap& map, const QStringList& lst);
  
  /**
   * Add custom information for a UID + App + Type
   *
   * @param app  The Application, like Todo, Calendar, Foo
   * @param uid  The UID of the Record
   * @param type The type of the ExtraData like Alarm, CompletedInfo...
   * @param item The pointer to the CustomExtraItem. Ownership of the Instance is transfered
   *
   */
  void add(const QString& app, const QString& type, const QString& uid, CustomExtraItem* item );
  
  /**
   * \brief Get the CustomExtraItem for a Composed Unique Identifier
   *
   *  Get the custom Item by the Composed Unique Identifier, it consists out of the
   *  Application-Name, the type of the CustomExtraItem and the actual UID
   *  of the record. This way you can have more than one CustomExtraItem for each
   *  UID.
   *
   * @param app  The Application-Name
   * @param type The CustomExtraItem Type
   * @param uid  The UID of the Records
   *
   * @return 0 or the CustomExtraItem for the CUID
   *
   * @see add
   */
  CustomExtraItem* item( const QString& app, const QString& type, const QString& uid );
  
  protected:
  QString escape( const QString& str );
  private:
  QMap<CUID, CustomExtraItem*> m_custom;
};

}


#endif
