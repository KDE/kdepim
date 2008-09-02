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

#ifndef QSYNC_PLUGINLOCALIZATION_H
#define QSYNC_PLUGINLOCALIZATION_H

#include <libqopensync/qopensync_export.h>

#include <QtCore/QString>

class OSyncPluginLocalization;

namespace QSync {

class QSYNC_EXPORT PluginLocalization
{
    friend class PluginConfig;

  public:
    enum ConfigOption
    {
      EncodingOption,
      TimeZoneOption,
      LanguageOption
    };

    PluginLocalization();
    ~PluginLocalization();

    /**
      Returns whether the object is a valid plugin localization.
     */
    bool isValid() const;

    /**
      Returns whether the given option is supported by the plugin.
     */
    bool isOptionSupported( ConfigOption option ) const;

    /**
      Sets the encoding.
     */
    void setEncoding( const QString &encoding );

    /**
      Returns the encoding.
     */
    QString encoding() const;

    /**
      Sets the timezone.
     */
    void setTimeZone( const QString &timezone );

    /**
      Returns the timezone.
     */
    QString timeZone() const;

    /**
      Sets the language.
     */
    void setLanguage( const QString &language );

    /**
      Returns the language.
     */
    QString language() const;

  private:
    OSyncPluginLocalization *mPluginLocalization;
};

}

#endif
