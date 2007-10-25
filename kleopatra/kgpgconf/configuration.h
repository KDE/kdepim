/* -*- mode: c++; c-basic-offset:4 -*-
    configuration.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef KGPGCONF_CONFIGURATION_H
#define KGPGCONF_CONFIGURATION_H

#include <QHash>
#include <QString>
#include <QVariant>

class ConfigComponent;
class ConfigGroup;
class ConfigEntry;

class Config
{
public:
    ~Config();

    QStringList componentList() const;
    ConfigComponent* component( const QString& name ) const;
    void addComponent( ConfigComponent* component );

private:

    QHash<QString,ConfigComponent*> m_components;
};

class ConfigComponent
{
public:
    explicit ConfigComponent( const QString& name );
    ~ConfigComponent();

    QString name() const;
    void setName( const QString& name );

    void setDescription( const QString& description );
    QString description() const;

    QStringList groupList() const;
    ConfigGroup* group( const QString& name ) const;

    void addGroup( ConfigGroup* group );

private:
    ConfigComponent();

private:
    QString m_name;
    QString m_description;
    QHash<QString, ConfigGroup*> m_groups; 
};

class ConfigGroup
{
public:
    explicit ConfigGroup( const QString& name );
    ~ConfigGroup();

    QString name() const;
    void setName( const QString& name );

    QString description() const;
    void setDescription( const QString& description );

    QStringList entryList() const;
    ConfigEntry* entry( const QString& name ) const;
    void addEntry( ConfigEntry* entry );

private:
    ConfigGroup();

private:
    QString m_name;
    QString m_description;
    QHash<QString, ConfigEntry*> m_entries;
};

class ConfigEntry
{
public:
    explicit ConfigEntry( const QString& name );

    QString name() const;
    void setName( const QString& name );

    QString description() const;
    void setDescription( const QString& description );

    void setReadOnly( bool readOnly );
    bool isReadOnly() const;

private:
    ConfigEntry();

private:
    QString m_name;
    QString m_description;
    QVariant m_value;
    bool m_readOnly;
};

#endif // KGPGCONF_CONFIGURATION_H

