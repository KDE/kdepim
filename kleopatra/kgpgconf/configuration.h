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

#include <QUrl>

#include <QHash>
#include <QString>
#include <QVariant>

class ConfigComponent;
class ConfigGroup;
class ConfigEntry;

class Config
{
public:
    Config();
    ~Config();

    QStringList componentList() const;
    ConfigComponent* component( const QString& name ) const;
    void addComponent( ConfigComponent* component );

private:
    Config( const Config& );
    Config& operator=( const Config& );

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
    ConfigEntry* entry( const QString& name ) const;

private:
    ConfigComponent();
    ConfigComponent( const ConfigComponent& );
    ConfigComponent& operator=( const ConfigComponent& );

private:
    QString m_name;
    QString m_description;
    QHash<QString, ConfigGroup*> m_groups;
    mutable QHash<QString, ConfigEntry*> m_entries;
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

    bool isEmpty() const;

    QStringList entryList() const;
    ConfigEntry* entry( const QString& name ) const;
    void addEntry( ConfigEntry* entry );

private:
    ConfigGroup();
    ConfigGroup( const ConfigGroup& );
    ConfigGroup& operator=( const ConfigGroup& );

private:
    QString m_name;
    QString m_description;
    QHash<QString, ConfigEntry*> m_entries;
};

class ConfigEntry
{
public:

    enum Mutability {
        UnspecifiedMutability=0,
        NoChange,
        Change
    };

    enum ArgType {
        None=0,
        String,
        Int,
        UInt,
        Path,
        Url,
        LdapUrl,
        DirPath
    };

    enum ListType {
        NoList=0,
        List
    };

    explicit ConfigEntry( const QString& name );

    QString name() const;
    void setName( const QString& name );

    QString description() const;
    void setDescription( const QString& description );

    void setMutability( Mutability mutability );
    Mutability mutability() const;

    bool isDirty() const;
    void unsetDirty();

    void setUseBuiltInDefault( bool useDefault );
    bool useBuiltInDefault() const;

    void setValueFromRawString( const QString& str );
    void setValueFromUiString( const QString& str );

    void setArgType( ArgType type, ListType listType );
    ArgType argType() const;

    /** Human-readable (i.e. translated) description of the entry's type */
    QString typeDescription() const;

    bool boolValue() const;
    QString stringValue() const;
    int intValue() const;
    unsigned int uintValue() const;
    QUrl urlValue() const;
    QStringList stringValueList() const;
    QList<int> intValueList() const;
    QList<unsigned int> uintValueList() const;
    QList<QUrl> urlValueList() const;
    unsigned int numberOfTimesSet() const;

    QString outputString() const;

    void setBoolValue( bool );
    void setStringValue( const QString& );
    void setIntValue( int );
    void setUIntValue( unsigned int );
    void setURLValue( const QUrl& );
    void setNumberOfTimesSet( unsigned int );
    void setStringValueList( const QStringList& );
    void setIntValueList( const QList<int>& );
    void setUIntValueList( const QList<unsigned int>& );
    void setURLValueList( const QList<QUrl>& );

private:
    bool isStringType() const;
    bool isList() const;

    enum EscapeMode {
        NoEscape=0,
        Escape=1,
        Quote=2,
        EscapeAndQuote=3
    };

    QString toString( EscapeMode mode ) const;

    ConfigEntry();
    ConfigEntry( const ConfigEntry& );
    ConfigEntry& operator=( const ConfigEntry& );
    
    enum UnescapeMode {
        DoNotUnescape=0,
        Unescape=1
    };

    QVariant stringToValue( const QString& str, UnescapeMode mode ) const;

private:
    bool m_dirty;
    QString m_name;
    QString m_description;
    QVariant m_value;
    Mutability m_mutability;
    bool m_useDefault;
    ArgType m_argType;
    bool m_isList;
};

#endif // KGPGCONF_CONFIGURATION_H

