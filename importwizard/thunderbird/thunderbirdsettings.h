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

#ifndef THUNDERBIRDSETTINGS_H
#define THUNDERBIRDSETTINGS_H

#include "abstractsettings.h"
#include "importwizardutil.h"
#include <QHash>
#include <QColor>
#include <QStringList>
#include <KUrl>

class ImportWizard;

class ThunderbirdSettings : public AbstractSettings
{
public:
    explicit ThunderbirdSettings(const QString& filename, ImportWizard *parent );
    ~ThunderbirdSettings();
private:
    void readAccount();
    void readIdentity( const QString& account );
    void readTransport();
    void readGlobalSettings();
    void readLdapSettings();
    void readTagSettings();
    void readExtensionsSettings();
    int adaptAutoResizeResolution(int index, const QString &configStrList);

    void insertIntoMap( const QString& line );

    void addAuth(QMap<QString, QVariant>& settings, const QString & argument, const QString &accountName );

    QHash<QString, QVariant> mHashConfig;
    QHash<QString, QString> mHashSmtp;
    QStringList mAccountList;
    QStringList mLdapAccountList;

    QHash<QString, tagStruct> mHashTag;
};

#endif /* THUNDERBIRDSETTINGS_H */
