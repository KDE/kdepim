/*
    Copyright (C) 2011 Tobias Koenig <tokoe@kde.org>

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

#ifndef MAILCOMMON_FILTERMANAGER_H
#define MAILCOMMON_FILTERMANAGER_H

#include "mailcommon_export.h"

#include "mailfilteragentinterface.h"

#include <akonadi/item.h>

#include <QtCore/QObject>

namespace MailCommon {

/**
 * @short A wrapper class that allows easy access to the mail filters
 *
 * This class communicates with the mailfilter agent via DBus.
 */
class MAILCOMMON_EXPORT FilterManager : public QObject
{
  public:
    /**
     * Describes the list of filters.
     */
    enum FilterSet
    {
      NoSet = 0x0,
      Inbound = 0x1,
      Outbound = 0x2,
      Explicit = 0x4,
      BeforeOutbound = 0x8,
      All = Inbound|BeforeOutbound|Outbound|Explicit
    };

    /**
     * Returns the global filter manager object.
     */
    static FilterManager* instance();

    /**
     * Returns whether the filter manager is in a usable state.
     */
    bool isValid() const;

    /**
     * Checks for existing filters with the @p name and extend the
     * "name" to "name (i)" until no match is found for i=1..n
     */
    QString createUniqueFilterName( const QString &name ) const;

    /**
     * Applies filter with the given @p identifier on the message @p item.
     * @return @c true on success, @c false otherwise.
     */
    void filter( const Akonadi::Item &item, const QString &identifier ) const;

    /**
     * Process given message item by applying the filter rules one by
     * one. You can select which set of filters (incoming or outgoing)
     * should be used.
     *
     * @param item The message item to process.
     * @param set Select the filter set to use.
     * @param account @c true if an account id is specified else @c false
     * @param accountId The id of the resource that the message was retrieved from
     */
    void filter( const Akonadi::Item &item, FilterSet set = Inbound,
                 bool account = false, const QString &resourceId = QString() ) const;

    /**
     * Process given @p messages by applying the filter rules one by
     * one. You can select which set of filters (incoming or outgoing)
     * should be used.
     *
     * @param item The message item to process.
     * @param set Select the filter set to use.
     */
    void filter( const Akonadi::Item::List &messages, FilterSet set = Explicit ) const;

  private:
    FilterManager();

    static FilterManager *mInstance;

    org::freedesktop::Akonadi::MailFilterAgent *mMailFilterAgentInterface;
};

}

#endif
