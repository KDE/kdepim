/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <freyther@kde.org>

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
#ifndef KSYNC_FILTER_H
#define KSYNC_FILTER_H

#include <qobject.h>
#include <qstringlist.h>
#include <qptrlist.h>

#include <syncer.h>

class KConfig;
class QWidget;
namespace KSync {

/**
 * A Filter is a Dynamically Shared Object
 * that is called to operate on a Syncee before
 * and after syncing is done.
 * For example this feature is used to filter
 * out Records from specific Categories for
 * specefic Syncees.
 * The same feature could be used to have any file
 * downloaded by  a Konnector or KIO Resource and then
 * convert an UnknownSyncee to something else before
 * sync is taking place and it gets converted to a
 * UnknownSyncee before writing back to the origin
 *
 * You need to supply implementation for a configure
 * dialog, a method to determine if you're able
 * and want to filter a Syncee and then methods to
 * convert and convert back (reconvert) this Syncee.
 *
 * A Filter can have multiple instances with different
 * Config Option set
 */
struct Filter : public QObject {
  typedef QValueList<Filter*> List;
  
  Filter( QObject* obj, const char* name);
  virtual ~Filter();
  
  void load( KConfig* );
  QString name()const;
  
  virtual bool wantToFilter( Syncee* ) = 0;
  virtual QWidget *configWidget( QWidget* wid , KConfig* cfg) = 0;
  
  
  /*
   * both methods may return 0 if they're
   * not able to convert!
   */
  virtual Syncee* reconvert( Syncee* ) = 0;
  virtual Syncee* convert( Syncee* ) = 0;
  
  protected:
  KConfig *config();
  void setName( const QString& name );
  
  private:
  virtual void doLoad() = 0;
  KConfig *m_cfg;
  QString m_name;
};
}

/**
 * \fn void KSync::Filter::load( KConfig *cfg)
 *
 * This method will call doLoad() which you need to implement
 * if you need to read configuration data.
 *
 * @param cfg The KConfig from where to load Configuration. The group is already set
 */


/**
 * \fn QString KSync::Filter::name()const
 * \brief Get the translated name of the Filter
 *
 * Get the Name of the Filter. Filters can call \sa setName
 * to set the Filters Name.
 *
 */

/**
 * \fn bool KSync::Filter::wantToFilter(Syncee* syncee)
 * \brief Test if a Filter can operate on the Syncee
 *
 * Before requesting to convert/reconvert the Syncee
 * the KSync::Filter is asked to if it can operate on the
 * Syncee.
 * Filters need to implement it
 *
 * @param syncee Can the filter operate on this Syncee
 */


/**
 * \fn Syncee* KSync::Filter::reconvert(Syncee* syn)
 * \brief Convert the Syncee before writing back
 *
 * Before the Syncees gets written back to the
 * Konnector you can filter it. If you return a different
 * Syncee the old one will be replaced.
 * The old one will be cleaned up and removed by the
 * KonnectorManager
 *
 * @param syn The Syncee to filter
 * @return The filtered Syncee or a new one
 */

/**
 * \fn Syncee* KSync::Filter::convert(Syncee* syn)
 * \brief Convert the Syncee before emitting the Read signal
 *
 * Before the Syncee emits the read signal you can filter the
 * Syncee.
 * If you return a different Syncee the old one will be replaced.
 * The old one will be cleaned up and removed by the
 * KonnectorManager
 *
 * @param syn The Syncee to filter
 * @return The filtered Syncee or a new one
 */

/**
 * \fn KConfig* KSync::Filter::config()
 * \brief Return the KConfig Instance
 *
 * Get KConfig object where the Configuration is stored. Do not change
 * the group, and it is only valid from within the doLoad() method
 *
 * @see doLoad()
 */

/**
 * \fn void KSync::Filter::setName(const QString& name)
 * \brief Set the Name of the Filter
 *
 * Set the Name returned by \sa name()const. Normally a Filter
 * Implementation will do this from within the Constructor.
 *
 * @param name Set the Name of the Filter
 */

/**
 * \fn void KSync::Filter::doLoad()
 * \brief Do load your Configuration
 *
 * Filter Implementation will load the Configuration here.
 * In this Time calling config() is save and the
 * right group is set
 *
 */


/**
 * \fn QWidget* KSync::Filter::configWidget(QWidget* parent, KConfig *cfg )
 * \brief Create a new Configure Widget
 *
 * Create a new KConfig widget. Use @param parent as the parent
 * @param cfg as the destination for the config data.
 *
 * Use the current values for your Config Widget.
 *
 * On d'tor you may save to the KConfig Instance, the right
 * group is set.
 *
 *
 */
#endif
