/*
  This file is part of the KDE Kontact Plugin Interface Library.

  Copyright (c) 2003 David Faure <faure@kde.org>

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

#ifndef KONTACTINTERFACES_UNIQUEAPPHANDLER_H
#define KONTACTINTERFACES_UNIQUEAPPHANDLER_H

#include "kontactinterfaces_export.h"
#include "plugin.h"

namespace Kontact
{

/**
 * D-Bus Object that has the name of the standalone application (e.g. "kmail")
 * and implements newInstance() so that running the separate application does
 * the right thing when kontact is running.
 * By default this means simply bringing the main window to the front,
 * but newInstance can be reimplemented.
 */
class KONTACTINTERFACES_EXPORT UniqueAppHandler : public QObject
{
  Q_OBJECT
  // We implement the KUniqueApplication interface
  Q_CLASSINFO("D-Bus Interface", "org.kde.KUniqueApplication")

  public:
    UniqueAppHandler( Plugin *plugin );
    virtual ~UniqueAppHandler();

    /// This must be reimplemented so that app-specific command line options can be parsed
    virtual void loadCommandLineOptions() = 0;

    Plugin *plugin() const;

    // for kontact
    static void setMainWidget(QWidget* widget);

  public Q_SLOTS: // DBUS methods
    int newInstance(const QByteArray &asn_id, const QByteArray &args);
    bool load();

  protected:
    virtual int newInstance();

  private:
    class Private;
    Private *const d;
};

/// Base class for UniqueAppHandler
class UniqueAppHandlerFactoryBase
{
  public:
    virtual ~UniqueAppHandlerFactoryBase(){}
    virtual UniqueAppHandler *createHandler( Plugin * ) = 0;
};

/**
 * Used by UniqueAppWatcher below, to create the above UniqueAppHandler object
 * when necessary.
 * The template argument is the UniqueAppHandler-derived class.
 * This allows to remove the need to subclass UniqueAppWatcher.
 */
template <class T> class UniqueAppHandlerFactory : public UniqueAppHandlerFactoryBase
{
  public:
    virtual UniqueAppHandler *createHandler( Plugin *plugin ) {
      plugin->registerClient();
      return new T( plugin );
    }
};

/**
 * If the standalone application is running by itself, we need to watch
 * for when the user closes it, and activate the uniqueapphandler then.
 * This prevents, on purpose, that the standalone app can be restarted.
 * Kontact takes over from there.
 *
 */
class KONTACTINTERFACES_EXPORT UniqueAppWatcher : public QObject
{
  Q_OBJECT

  public:
    /**
     * Create an instance of UniqueAppWatcher, which does everything necessary
     * for the "unique application" behavior: create the UniqueAppHandler as soon
     * as possible, i.e. either right now or when the standalone app is closed.
     *
     * @param factory templatized factory to create the handler. Example:
     * ...   Note that the watcher takes ownership of the factory.
     * @param plugin is the plugin application
     */
    UniqueAppWatcher( UniqueAppHandlerFactoryBase *factory, Plugin *plugin );

    virtual ~UniqueAppWatcher();

    bool isRunningStandalone() const;

  private Q_SLOTS:
    void slotApplicationRemoved(const QString & name, const QString & oldOwner, const QString & newOwner);

  private:
    class Private;
    Private *const d;
};

} // namespace

#endif

