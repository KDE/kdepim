/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef ABSTRACTSETTINGS_H
#define ABSTRACTSETTINGS_H

#include "abstractbase.h"
#include <KSharedConfig>
#include <QMap>

class ImportWizard;

namespace KPIMIdentities {
class Identity;
class IdentityManager;
}

namespace MailTransport {
class Transport;
}

class AbstractSettings : public AbstractBase
{
public:
    explicit AbstractSettings(ImportWizard *parent);
    ~AbstractSettings();

protected:
    void addImportInfo( const QString& log );
    void addImportError( const QString& log );

    void syncKmailConfig();

    QString uniqueIdentityName(const QString& name);

    QString createResource(const QString& resources , const QString& name, const QMap<QString, QVariant> &settings);

    KPIMIdentities::Identity* createIdentity(QString& name);

    MailTransport::Transport *createTransport();

    void storeTransport(MailTransport::Transport * mt, bool isDefault = false );

    void storeIdentity(KPIMIdentities::Identity* identity);

    void addKmailConfig( const QString& groupName, const QString& key, const QString& value);
    void addKmailConfig( const QString& groupName, const QString& key, bool value);
    void addKmailConfig( const QString& groupName, const QString& key, int value);

    void addComposerHeaderGroup( const QString& groupName, const QString& name, const QString& value );

    void addKNodeConfig(const QString& groupName, const QString& key, bool value);
    void addAkregatorConfig(const QString& groupName, const QString& key, bool value);


    void addCheckMailOnStartup(const QString& agentIdentifyName,bool loginAtStartup);
    void addToManualCheck(const QString& agentIdentifyName,bool manualCheck);
    int readKmailSettings( const QString&groupName, const QString& key);

    ImportWizard *mImportWizard;
    KPIMIdentities::IdentityManager *mManager;
    KSharedConfigPtr mKmailConfig;
};

#endif // ABSTRACTSETTINGS_H
