/*
    Copyright (c) 2010-2016 Laurent Montel <montel@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include "setupobject.h"
#include <QVector>
class KConfig;

struct Config {
    QString group;
    QString key;
    QString value;
    bool obscure;
};

class ConfigFile : public SetupObject
{
    Q_OBJECT
public:
    explicit ConfigFile(const QString &configName, QObject *parent = Q_NULLPTR);
    ~ConfigFile();
    void create() Q_DECL_OVERRIDE;
    void destroy() Q_DECL_OVERRIDE;
    void edit();
public Q_SLOTS:
    Q_SCRIPTABLE void write();
    Q_SCRIPTABLE void setName(const QString &name);
    Q_SCRIPTABLE void setConfig(const QString &group, const QString &key, const QString &value);
    Q_SCRIPTABLE void setPassword(const QString &group, const QString &key, const QString &value);
    Q_SCRIPTABLE void setEditMode(const bool editMode);
    Q_SCRIPTABLE void setEditName(const QString &name);
private:
    QVector<Config> m_configData;
    QString m_name;
    KConfig *m_config;
    QString m_editName;
    bool m_editMode;
};

#endif
