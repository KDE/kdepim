/*
    This file is part of libqopensync.

    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

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

#ifndef QSYNC_PLUGINADVANCEDOPTIONS_H
#define QSYNC_PLUGINADVANCEDOPTIONS_H

#include <libqopensync/qopensync_export.h>

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>

class OSyncPluginAdvancedOption;
class OSyncPluginAdvancedOptionParameter;

namespace QSync {

class QSYNC_EXPORT PluginAdvancedOptionParameter
{
    friend class PluginAdvancedOption;

  public:
    typedef QList<PluginAdvancedOptionParameter> List;

    enum OptionType
    {
      NoneOption = 0,
      BoolOption,
      CharOption,
      DoubleOption,
      IntOption,
      LongOption,
      LongLongOption,
      UIntOption,
      ULongOption,
      ULongLongOption,
      StringOption
    };

    PluginAdvancedOptionParameter();
    ~PluginAdvancedOptionParameter();

    static PluginAdvancedOptionParameter create();

    /**
      Returns whether the object is a valid plugin advanced option parameter.
     */
    bool isValid() const;

    /**
      Sets the display name of the parameter.
     */
    void setDisplayName( const QString &displayName );

    /**
      Returns the display name of the parameter.
     */
    QString displayName() const;

    /**
      Sets the name of the parameter.
     */
    void setName( const QString &name );

    /**
      Returns the name of the parameter.
     */
    QString name() const;

    /**
      Sets the type of the parameter.
     */
    void setType( OptionType type );

    /**
      Returns the type of the parameter.
     */
    OptionType type() const;

    /**
      Returns the type string of the parameter.
     */
    QString typeString() const;

    /**
      Returns the value enums of the parameter.
     */
    QStringList enumValues() const;

    /**
      Adds a value enum to the parameter.
     */
    void addEnumValue( const QString &value );

    /**
      Removes a value enum from the parameter.
     */
    void removeEnumValue( const QString &value );

    /**
      Sets the value of the parameter.
     */
    void setValue( const QString &value );

    /**
      Returns the value of the parameter.
     */
    QString value() const;

  private:
    OSyncPluginAdvancedOptionParameter *mPluginAdvancedOptionParameter;
};

class QSYNC_EXPORT PluginAdvancedOption
{
    friend class PluginConfig;

  public:
    typedef QList<PluginAdvancedOption> List;

    enum OptionType
    {
      NoneOption = 0,
      BoolOption,
      CharOption,
      DoubleOption,
      IntOption,
      LongOption,
      LongLongOption,
      UIntOption,
      ULongOption,
      ULongLongOption,
      StringOption
    };

    PluginAdvancedOption();
    ~PluginAdvancedOption();

    /**
      Returns whether the object is a valid plugin advanced option.
     */
    bool isValid() const;

    /**
      Returns the list of parameters of the option.
     */
    PluginAdvancedOptionParameter::List parameters() const;

    /**
      Adds a parameter to the option.
     */
    void addParameter( const PluginAdvancedOptionParameter &parameter );

    /**
      Removes a parameter from the option.
     */
    void removeParameter( const PluginAdvancedOptionParameter &parameter );

    /**
      Sets the minimum size of the option.
     */
    void setMinimumSize( unsigned int minSize );

    /**
      Returns the minimum sizeof the option.
     */
    unsigned int minimumSize() const;

    /**
      Sets the maximum size of the option.
     */
    void setMaximumSize( unsigned int maxSize );

    /**
      Returns the maximum sizeof the option.
     */
    unsigned int maximumSize() const;

    /**
      Sets the maximum occurrence of the option.
     */
    void setMaximumOccurrence( unsigned int occurrence );

    /**
      Returns the maximum occurrence the option.
     */
    unsigned int maximumOccurrence() const;

    /**
      Sets the display name of the option.
     */
    void setDisplayName( const QString &displayName );

    /**
      Returns the display name the option.
     */
    QString displayName() const;

    /**
      Sets the name of the option.
     */
    void setName( const QString &name );

    /**
      Returns the name the option.
     */
    QString name() const;

    /**
      Sets the type of the option.
     */
    void setType( OptionType type );

    /**
      Returns the type of the option.
     */
    OptionType type() const;

    /**
      Returns the type string of the option.
     */
    QString typeString() const;

    /**
      Returns the value enums of the option.
     */
    QStringList enumValues() const;

    /**
      Adds a value enum to the option.
     */
    void addEnumValue( const QString &value );

    /**
      Removes a value enum from the option.
     */
    void removeEnumValue( const QString &value );

    /**
      Sets the value of the option.
     */
    void setValue( const QString &value );

    /**
      Returns the value of the option.
     */
    QString value() const;

  private:
    OSyncPluginAdvancedOption *mPluginAdvancedOption;
};

}

#endif
