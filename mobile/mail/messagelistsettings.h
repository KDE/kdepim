/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

#ifndef MESSAGELISTSETTINGS_H
#define MESSAGELISTSETTINGS_H

#include <QtCore/QString>

class MessageListSettings
{
  public:
    MessageListSettings();
    ~MessageListSettings();

    enum SortingOption
    {
      SortByDateTime,
      SortByDateTimeMostRecent,
      SortBySenderReceiver,
      SortBySubject,
      SortBySize,
      SortByActionItem
    };

    enum GroupingOption
    {
      GroupByNone,
      GroupByDate,
      GroupBySenderReceiver
    };

    void setSortingOption( SortingOption option );
    SortingOption sortingOption() const;

    void setSortingOrder( Qt::SortOrder order );
    Qt::SortOrder sortingOrder() const;

    void setGroupingOption( GroupingOption option );
    GroupingOption groupingOption() const;

    void setUseThreading( bool threading );
    bool useThreading() const;

    void setUseGlobalSettings( bool value );
    bool useGlobalSettings() const;

    static MessageListSettings fromConfig( qint64 collectionId );
    static void toConfig( qint64 collectionId, const MessageListSettings &settings );

    static MessageListSettings fromDefaultConfig();
    static void toDefaultConfig( const MessageListSettings &settings );

  private:
    SortingOption mSortingOption;
    bool mSortDescending;
    GroupingOption mGroupingOption;
    bool mUseThreading;
    bool mUseGlobalSettings;
};

#endif
