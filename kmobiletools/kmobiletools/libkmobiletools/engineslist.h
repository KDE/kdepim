/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#ifndef ENGINESLIST_H
#define ENGINESLIST_H

#include <libkmobiletools/kmobiletools_export.h>

#include <qstringlist.h>
#include <qobject.h>
#include <kplugininfo.h>
/// @TODO move this class and engineLoader in a single class (EngineManager?)

/**
	@author Marco Gulino <marco@kmobiletools.org>
*/
class KPluginInfo;
class EnginesListPrivate;
class KConfigSkeleton;
namespace KMobileTools {
class Engine;
class KMOBILETOOLS_EXPORT EnginesList : public QObject, public QList<KMobileTools::Engine*>
{
    Q_OBJECT
public:
    EnginesList();
    ~EnginesList();

    static EnginesList *instance();
    const QStringList namesList(bool friendlyNames=false);
    KMobileTools::Engine *find( const QString &name, bool friendlyName=false);
    void queryClose();
    void append ( KMobileTools::Engine* item );
    void remove ( KMobileTools::Engine* item );
    bool closing() const;
    bool locked(const QString &device) const;
    const QStringList locklist();
    bool lock(const QString &device);
    void unlock(const QString &device);
    KPluginInfo::List availEngines();
    KPluginInfo engineInfo(const QString &s, bool searchByLibrary=true);

    /**
     * A convenience method that allows to access a global engine object to be used in the wizard.
     * @return a pointer to a KMobileTools::Engine object, or NULL if there is not an engine loaded.
     */
    Engine *wizardEngine();
    /**
     * A configuration object with all the settings collected by the wizard.
     * @return a pointer to a KConfigSkeleton object, or NULL if no configuration is loaded yet.
     */
    KConfigSkeleton *wizardConfig();
    /**
     * Sets the configuration object for the running Wizard.
     * @param wizardcfg a pointer to a KConfigSkeleton object.
     */
    void setWizardConfig(KConfigSkeleton *wizardcfg);

    /**
     * Sets a new engine to be used in the wizard.
     * Calling without parameters or a NULL pointer will delete current engine, and sets the pointer to NULL
     * @param newEngine a pointer to a new KMobileTools::Engine object.
     */
    void setWizardEngine(KMobileTools::Engine* newEngine=NULL);

    private:
        EnginesListPrivate *d;

    public slots:
        void dump();
//         void emitPhonebookUpdated();
    signals:
        void phonebookUpdated(); /// Emitted when one of the engines has updated internal phonebook;
        void engineAdded(const KMobileTools::Engine *);
        void engineRemoved(const KMobileTools::Engine *);

};
}

#endif
