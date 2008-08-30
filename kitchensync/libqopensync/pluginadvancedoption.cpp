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

#include <opensync/opensync.h>
#include <opensync/opensync-plugin.h>

#include "pluginadvancedoption.h"

using namespace QSync;

PluginAdvancedOptionParameter::PluginAdvancedOptionParameter()
  : mPluginAdvancedOptionParameter( 0 )
{
}

PluginAdvancedOptionParameter::~PluginAdvancedOptionParameter()
{
}

PluginAdvancedOptionParameter PluginAdvancedOptionParameter::create()
{
  PluginAdvancedOptionParameter param;

  OSyncError *error;
  param.mPluginAdvancedOptionParameter = osync_plugin_advancedoption_param_new( &error );

  return param;
}

bool PluginAdvancedOptionParameter::isValid() const
{
  return (mPluginAdvancedOptionParameter != 0);
}

void PluginAdvancedOptionParameter::setDisplayName( const QString &displayName )
{
  Q_ASSERT( mPluginAdvancedOptionParameter );

  osync_plugin_advancedoption_param_set_displayname( mPluginAdvancedOptionParameter, displayName.toUtf8().data() );
}

QString PluginAdvancedOptionParameter::displayName() const
{
  Q_ASSERT( mPluginAdvancedOptionParameter );

  return QString::fromUtf8( osync_plugin_advancedoption_param_get_displayname( mPluginAdvancedOptionParameter ) );
}

void PluginAdvancedOptionParameter::setName( const QString &name )
{
  Q_ASSERT( mPluginAdvancedOptionParameter );

  osync_plugin_advancedoption_param_set_name( mPluginAdvancedOptionParameter, name.toUtf8() );
}

QString PluginAdvancedOptionParameter::name() const
{
  Q_ASSERT( mPluginAdvancedOptionParameter );

  return QString::fromUtf8( osync_plugin_advancedoption_param_get_name( mPluginAdvancedOptionParameter ) );
}

void PluginAdvancedOptionParameter::setType( OptionType type )
{
  Q_ASSERT( mPluginAdvancedOptionParameter );

  OSyncPluginAdvancedOptionType osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_NONE;

  switch ( type ) {
    case NoneOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_NONE; break;
    case BoolOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_BOOL; break;
    case CharOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_CHAR; break;
    case DoubleOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_DOUBLE; break;
    case IntOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_INT; break;
    case LongOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_LONG; break;
    case LongLongOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_LONGLONG; break;
    case UIntOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_UINT; break;
    case ULongOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_ULONG; break;
    case ULongLongOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_ULONGLONG; break;
    case StringOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_STRING; break;
  }

  osync_plugin_advancedoption_param_set_type( mPluginAdvancedOptionParameter, osyncType );
}

PluginAdvancedOptionParameter::OptionType PluginAdvancedOptionParameter::type() const
{
  Q_ASSERT( mPluginAdvancedOptionParameter );

  OSyncPluginAdvancedOptionType osyncType = osync_plugin_advancedoption_param_get_type( mPluginAdvancedOptionParameter );

  OptionType type = NoneOption;
  switch ( osyncType ) {
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_NONE: type = NoneOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_BOOL: type = BoolOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_CHAR: type = CharOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_DOUBLE: type = DoubleOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_INT: type = IntOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_LONG: type = LongOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_LONGLONG: type = LongLongOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_UINT: type = UIntOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_ULONG: type = ULongOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_ULONGLONG: type = ULongLongOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_STRING: type = StringOption; break;
  }

  return type;
}

QString PluginAdvancedOptionParameter::typeString() const
{
  Q_ASSERT( mPluginAdvancedOptionParameter );

  return QString::fromUtf8( osync_plugin_advancedoption_param_get_type_string( mPluginAdvancedOptionParameter ) );
}

QStringList PluginAdvancedOptionParameter::enumValues() const
{
  Q_ASSERT( mPluginAdvancedOptionParameter );

  QStringList values;

  OSyncList *list = osync_plugin_advancedoption_param_get_valenums( mPluginAdvancedOptionParameter );
  for ( ; list; list = list->next )
    values.append( QString::fromUtf8( (char *)list->data ) );

  return values;
}

void PluginAdvancedOptionParameter::addEnumValue( const QString &value )
{
  Q_ASSERT( mPluginAdvancedOptionParameter );

  osync_plugin_advancedoption_param_add_valenum( mPluginAdvancedOptionParameter, value.toUtf8().data() );
}

void PluginAdvancedOptionParameter::removeEnumValue( const QString &value )
{
  Q_ASSERT( mPluginAdvancedOptionParameter );

  osync_plugin_advancedoption_param_remove_valenum( mPluginAdvancedOptionParameter, value.toUtf8().data() );
}

void PluginAdvancedOptionParameter::setValue( const QString &value )
{
  Q_ASSERT( mPluginAdvancedOptionParameter );

  osync_plugin_advancedoption_param_set_value( mPluginAdvancedOptionParameter, value.toUtf8().data() );
}

QString PluginAdvancedOptionParameter::value() const
{
  Q_ASSERT( mPluginAdvancedOptionParameter );

  return QString::fromUtf8( osync_plugin_advancedoption_param_get_value( mPluginAdvancedOptionParameter ) );
}


PluginAdvancedOption::PluginAdvancedOption()
  : mPluginAdvancedOption( 0 )
{
}

PluginAdvancedOption::~PluginAdvancedOption()
{
}

bool PluginAdvancedOption::isValid() const
{
  return (mPluginAdvancedOption != 0);
}

PluginAdvancedOptionParameter::List PluginAdvancedOption::parameters() const
{
  Q_ASSERT( mPluginAdvancedOption );

  PluginAdvancedOptionParameter::List parameters;

  OSyncList *list = osync_plugin_advancedoption_get_parameters( mPluginAdvancedOption );
  for ( ; list; list = list->next ) {
    PluginAdvancedOptionParameter parameter;
    parameter.mPluginAdvancedOptionParameter = (OSyncPluginAdvancedOptionParameter*)list->data;
    parameters.append( parameter );
  }

  return parameters;
}

void PluginAdvancedOption::addParameter( const PluginAdvancedOptionParameter &parameter )
{
  Q_ASSERT( mPluginAdvancedOption );

  osync_plugin_advancedoption_add_parameter( mPluginAdvancedOption, parameter.mPluginAdvancedOptionParameter );
}

void PluginAdvancedOption::removeParameter( const PluginAdvancedOptionParameter &parameter )
{
  Q_ASSERT( mPluginAdvancedOption );

  osync_plugin_advancedoption_remove_parameter( mPluginAdvancedOption, parameter.mPluginAdvancedOptionParameter );
}

void PluginAdvancedOption::setMinimumSize( unsigned int minSize )
{
  Q_ASSERT( mPluginAdvancedOption );

  osync_plugin_advancedoption_set_minsize( mPluginAdvancedOption, minSize );
}

unsigned int PluginAdvancedOption::minimumSize() const
{
  Q_ASSERT( mPluginAdvancedOption );

  return osync_plugin_advancedoption_get_minsize( mPluginAdvancedOption );
}

void PluginAdvancedOption::setMaximumSize( unsigned int maxSize )
{
  Q_ASSERT( mPluginAdvancedOption );

  osync_plugin_advancedoption_set_maxsize( mPluginAdvancedOption, maxSize );
}

unsigned int PluginAdvancedOption::maximumSize() const
{
  Q_ASSERT( mPluginAdvancedOption );

  return osync_plugin_advancedoption_get_maxsize( mPluginAdvancedOption );
}

void PluginAdvancedOption::setMaximumOccurrence( unsigned int occurrence )
{
  Q_ASSERT( mPluginAdvancedOption );

  osync_plugin_advancedoption_set_maxoccurs( mPluginAdvancedOption, occurrence );
}

unsigned int PluginAdvancedOption::maximumOccurrence() const
{
  Q_ASSERT( mPluginAdvancedOption );

  return osync_plugin_advancedoption_get_maxoccurs( mPluginAdvancedOption );
}

void PluginAdvancedOption::setDisplayName( const QString &displayName )
{
  Q_ASSERT( mPluginAdvancedOption );

  osync_plugin_advancedoption_set_displayname( mPluginAdvancedOption, displayName.toUtf8().data() );
}

QString PluginAdvancedOption::displayName() const
{
  Q_ASSERT( mPluginAdvancedOption );

  return QString::fromUtf8( osync_plugin_advancedoption_get_displayname( mPluginAdvancedOption ) );
}

void PluginAdvancedOption::setName( const QString &name )
{
  Q_ASSERT( mPluginAdvancedOption );

  osync_plugin_advancedoption_set_name( mPluginAdvancedOption, name.toUtf8() );
}

QString PluginAdvancedOption::name() const
{
  Q_ASSERT( mPluginAdvancedOption );

  return QString::fromUtf8( osync_plugin_advancedoption_get_name( mPluginAdvancedOption ) );
}

void PluginAdvancedOption::setType( OptionType type )
{
  Q_ASSERT( mPluginAdvancedOption );

  OSyncPluginAdvancedOptionType osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_NONE;

  switch ( type ) {
    case NoneOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_NONE; break;
    case BoolOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_BOOL; break;
    case CharOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_CHAR; break;
    case DoubleOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_DOUBLE; break;
    case IntOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_INT; break;
    case LongOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_LONG; break;
    case LongLongOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_LONGLONG; break;
    case UIntOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_UINT; break;
    case ULongOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_ULONG; break;
    case ULongLongOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_ULONGLONG; break;
    case StringOption: osyncType = OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_STRING; break;
  }

  osync_plugin_advancedoption_set_type( mPluginAdvancedOption, osyncType );
}

PluginAdvancedOption::OptionType PluginAdvancedOption::type() const
{
  Q_ASSERT( mPluginAdvancedOption );

  OSyncPluginAdvancedOptionType osyncType = osync_plugin_advancedoption_get_type( mPluginAdvancedOption );

  OptionType type = NoneOption;
  switch ( osyncType ) {
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_NONE: type = NoneOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_BOOL: type = BoolOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_CHAR: type = CharOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_DOUBLE: type = DoubleOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_INT: type = IntOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_LONG: type = LongOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_LONGLONG: type = LongLongOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_UINT: type = UIntOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_ULONG: type = ULongOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_ULONGLONG: type = ULongLongOption; break;
    case OSYNC_PLUGIN_ADVANCEDOPTION_TYPE_STRING: type = StringOption; break;
  }

  return type;
}

QString PluginAdvancedOption::typeString() const
{
  Q_ASSERT( mPluginAdvancedOption );

  return QString::fromUtf8( osync_plugin_advancedoption_get_type_string( mPluginAdvancedOption ) );
}

QStringList PluginAdvancedOption::enumValues() const
{
  Q_ASSERT( mPluginAdvancedOption );

  QStringList values;

  OSyncList *list = osync_plugin_advancedoption_get_valenums( mPluginAdvancedOption );
  for ( ; list; list = list->next )
    values.append( QString::fromUtf8( (char *)list->data ) );

  return values;
}

void PluginAdvancedOption::addEnumValue( const QString &value )
{
  Q_ASSERT( mPluginAdvancedOption );

  osync_plugin_advancedoption_add_valenum( mPluginAdvancedOption, value.toUtf8().data() );
}

void PluginAdvancedOption::removeEnumValue( const QString &value )
{
  Q_ASSERT( mPluginAdvancedOption );

  osync_plugin_advancedoption_remove_valenum( mPluginAdvancedOption, value.toUtf8().data() );
}

void PluginAdvancedOption::setValue( const QString &value )
{
  Q_ASSERT( mPluginAdvancedOption );

  osync_plugin_advancedoption_set_value( mPluginAdvancedOption, value.toUtf8().data() );
}

QString PluginAdvancedOption::value() const
{
  Q_ASSERT( mPluginAdvancedOption );

  return QString::fromUtf8( osync_plugin_advancedoption_get_value( mPluginAdvancedOption ) );
}
