/* Copyright 2009 Thomas McGuire <mcguire@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef CONFIGPROVIDER_H
#define CONFIGPROVIDER_H

#include "messagelist_export.h"

#include <KSharedConfig>

namespace MessageList
{

namespace Core
{

/**
 * Little helper class used to get and set the config file object used by the message list.
 */
class MESSAGELIST_EXPORT ConfigProvider
{
  public:

    KSharedConfig::Ptr config() const;

    /**
     * Sets the config object used by the message list. You have to call that before any other
     * class of the message list is used, so call this before creating the message list widget.
     */
    void setConfig( KSharedConfig::Ptr config );

    static ConfigProvider *self();

  private:

    mutable KSharedConfig::Ptr mConfig;
};

}

}

#endif
