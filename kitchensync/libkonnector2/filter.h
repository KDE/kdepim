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

#include <klibloader.h>
#include <kgenericfactory.h>

#include <syncer.h>

class KConfig;
class QWidget;

#define K_EXPORT_KS_FILTER_CATALOG( libname, FilterClass, catalog ) \
 class KDE_NO_EXPORT localFilterFactory : public FilterFactory \
 { \
   Filter *createFilter( QObject *parent ) \
   { \
     const char *cat = catalog; \
     if ( cat ) \
       KGlobal::locale()->insertCatalogue( cat ); \
     return new FilterClass( parent ); \
   } \
 }; \
 K_EXPORT_COMPONENT_FACTORY( libname, localFilterFactory )


#define K_EXPORT_KS_FILTER( libname, FilterClass ) \
	K_EXPORT_KS_FILTER_CATALOG( libname, FilterClass, NULL )


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
class Filter : public QObject
{
  public:
    typedef QValueList<Filter*> List;
  
    Filter( QObject *parent, const char *name );
    virtual ~Filter();
  
    /**
      This method will call doLoad() which you need to implement
      if you need to read configuration data.

      @param config The KConfig from where to load configuration.
                    The group is already set.
     */
    void load( KConfig *config );

    void save( KConfig* );

    /**
      Get the translated name of the filter

      Get the name of the filter. Filters can call \sa setName
      to set the filters name.
     */
    QString name() const;
    virtual QString type() const = 0;
  
    /**
      Test if a filter can operate on the syncee

      Before requesting to convert/reconvert the syncee
      the KSync::Filter is asked to if it can operate on the
      syncee.
      Filters need to implement it.

      @param syncee Can the filter operate on this Syncee
     */
    virtual bool supports( Syncee *syncee ) = 0;

    /**
      Create a new configuration widget.

      Create a new configuration widget.
      
      @param parent The parent widget.
     */
    virtual QWidget *configWidget( QWidget *parent ) = 0;

    /**
      Called when config widget is closed

      @param widget The widget that was created by @ref configWidget().
     */
    virtual void configWidgetClosed( QWidget *widget ) = 0;

    virtual void convert( Syncee* ) = 0;
    virtual void reconvert( Syncee* ) = 0;
  
  protected:
    /**
      Returns the KConfig instance

      Get KConfig object where the configuration is stored. Do not change
      the group, and it is only valid from within the doLoad() method

      @see doLoad()
     */
    KConfig *config();

    /**
      Set the name of the filter

      Set the name returned by \sa name() const. Normally a filter
      implementation will do this from within the constructor.

      @param name Set the name of the filter
     */
    void setName( const QString& name );
  
  private:
    /**
      Do load your configuration.

      Filter implementation will load the configuration here.
      In this time calling config() is save and the right group is set.
     */
    virtual void doLoad() = 0;

    /**
      Do save your configuration

      Filter implementation will save the configuration here.
      In this time calling config() is save and the right group is set.
     */
    virtual void doSave() = 0;

    KConfig *mConfig;
    QString mName;
};

class FilterFactory : public KLibFactory
{
  public:
    virtual Filter *createFilter( QObject *parent ) = 0;

  protected:
    virtual QObject* createObject( QObject*, const char*, const char*,
                                   const QStringList & )
    {
      return 0;
    }
};

}

#endif
