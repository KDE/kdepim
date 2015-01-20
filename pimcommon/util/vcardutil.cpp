/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "vcardutil.h"
#include <QByteArray>

using namespace PimCommon;

VCardUtil::VCardUtil()
{

}

void VCardUtil::adaptVcard(QByteArray &data)
{
    data.replace("X-messaging/aim-All",("X-AIM"));
    data.replace("X-messaging/icq-All",("X-ICQ"));
    data.replace("X-messaging/xmpp-All",("X-JABBER"));
    data.replace("X-messaging/msn-All",("X-MSN"));
    data.replace("X-messaging/yahoo-All",("X-YAHOO"));
    data.replace("X-messaging/gadu-All",("X-GADUGADU"));
    data.replace("X-messaging/skype-All",("X-SKYPE"));
    data.replace("X-messaging/groupwise-All",("X-GROUPWISE"));
    data.replace(("X-messaging/sms-All"),("X-SMS"));
    data.replace(("X-messaging/meanwhile-All"),("X-MEANWHILE"));
    data.replace(("X-messaging/irc-All"),("X-IRC"));
    data.replace(("X-messaging/googletalk-All"),("X-GTALK"));
}

