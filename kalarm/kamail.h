/*
 *  kamail.h  -  email functions
 *  Program:  kalarm
 *  Copyright © 2002-2008 by David Jarvie <djarvie@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KAMAIL_H
#define KAMAIL_H

#include <tqstring.h>
#include <tqstringlist.h>
class KURL;
class KAEvent;
class EmailAddressList;
namespace KPIM { class IdentityManager; }
namespace KMime { namespace Types { struct Address; } }

struct KAMailData;


class KAMail
{
	public:
		static bool        send(const KAEvent&, TQStringList& errmsgs, bool allowNotify = true);
		static int         checkAddress(TQString& address);
		static int         checkAttachment(TQString& attachment, KURL* = 0);
		static bool        checkAttachment(const KURL&);
		static TQString     convertAddresses(const TQString& addresses, EmailAddressList&);
		static TQString     convertAttachments(const TQString& attachments, TQStringList& list);
		static KPIM::IdentityManager* identityManager();
		static bool        identitiesExist();
		static uint        identityUoid(const TQString& identityUoidOrName);
		static TQString     controlCentreAddress();
		static TQString     getMailBody(Q_UINT32 serialNumber);
		static TQString     i18n_NeedFromEmailAddress();
		static TQString     i18n_sent_mail();

	private:
		static KPIM::IdentityManager* mIdentityManager;     // KMail identity manager
		static TQString     sendKMail(const KAMailData&);
		static TQString     initHeaders(const KAMailData&, bool dateId);
		static TQString     appendBodyAttachments(TQString& message, const KAEvent&);
		static TQString     addToKMailFolder(const KAMailData&, const char* folder, bool checkKmailRunning);
		static bool        callKMail(const TQByteArray& callData, const TQCString& iface, const TQCString& function, const TQCString& funcType);
		static TQString     convertAddress(KMime::Types::Address, EmailAddressList&);
		static void        notifyQueued(const KAEvent&);
		static char*       base64Encode(const char* in, TQIODevice::Offset size, TQIODevice::Offset& outSize);
		static TQStringList errors(const TQString& error = TQString::null, bool sendfail = true);
};

#endif // KAMAIL_H
