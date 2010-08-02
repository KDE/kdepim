/*  -*- c++ -*-
    This file is part of libkdepim.

    Copyright (c) 2002,2004 Marc Mutz <mutz@kde.org>

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

#ifndef __LIBKDEPIM_PLUGINLOADERBASE_H__
#define __LIBKDEPIM_PLUGINLOADERBASE_H__

#include <tqstring.h>
#include <tqmap.h>

#include <kdepimmacros.h>

class KLibrary;
class TQStringList;

namespace KPIM {

  class KDE_EXPORT PluginMetaData {
  public:
    PluginMetaData() {}
    PluginMetaData( const TQString & lib, const TQString & name,
		    const TQString & comment )
      : library( lib ), nameLabel( name ),
	descriptionLabel( comment ), loaded( false ) {}
    TQString library;
    TQString nameLabel;
    TQString descriptionLabel;
    mutable bool loaded;
  };

  class KDE_EXPORT PluginLoaderBase {
  protected:
    PluginLoaderBase();
    virtual ~PluginLoaderBase();

  public:
    /** Returns a list of all available plugin objects (of kind @p T) */
    TQStringList types() const;

    /** Returns the @ref PluginMetaData structure for a given type */
    const PluginMetaData * infoForName( const TQString & type ) const;

    /** Overload this method in subclasses to call @ref doScan with
        the right @p path argument */
    virtual void scan() = 0;

  protected:
    /** Rescans the plugin directory to find any newly installed
	plugins. Extend this method in subclasses to add any
	builtins. Subclasses must call this explicitely. It's not
	called for them in the constructor.
    **/
    void doScan( const char * path );

    /** Returns a pointer to symbol @p main_func in the library that
        implements the plugin of type @p type */
    void * mainFunc( const TQString & type, const char * main_func ) const;

  private:
    const KLibrary * openLibrary( const TQString & libName ) const;
    TQMap< TQString, PluginMetaData > mPluginMap;

    class Private;
    Private * d;
  };

} // namespace KMime

#endif // __LIBKDEPIM_PLUGINLOADERBASE_H__
