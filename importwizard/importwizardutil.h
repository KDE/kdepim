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

#ifndef IMPORTWIZARDUTIL_H
#define IMPORTWIZARDUTIL_H
#include <KUrl>
#include <QList>
#include <QColor>
#include <QString>

struct ldapStruct
{
    ldapStruct()
        : maxHint(-1),
          port(-1),
          limit(-1),
          timeout(-1),
          useSSL(false),
          useTLS(false)
    {
    }

    KUrl ldapUrl;
    QString dn;
    QString saslMech;
    QString fileName;
    QString description;
    QString password;
    int maxHint;
    int port;
    int limit;
    int timeout;
    bool useSSL;
    bool useTLS;

};

struct tagStruct {
    QString name;
    QColor color;
};


namespace ImportWizardUtil {
enum ResourceType {
    Imap,
    Pop3,
    Ldap
};

void mergeLdap(const ldapStruct &ldap);
void addNepomukTag(const QList<tagStruct> &tagList);
void storeInKWallet(const QString &name, ImportWizardUtil::ResourceType type, const QString &password);
}

#endif // IMPORTWIZARDUTIL_H
