/***************************************************************************
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>
   Rearranged by Marco Gulino <marco@kmobiletools.org> for the mobile protocol
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/


#ifndef MOBILEIMPL_H
#define MOBILEIMPL_H

#include <kio/global.h>
#include <kio/job.h>
#include <kurl.h>
#include <dcopobject.h>

#include <qobject.h>
#include <qstring.h>
//Added by qt3to4:
#include <Q3ValueList>

class MobileImpl : public QObject
{
    Q_OBJECT
public:
	MobileImpl();

	void createTopLevelEntry(KIO::UDSEntry& entry) const;

 bool listRoot(Q3ValueList<KIO::UDSEntry> &list);
 bool listDirectory(const QString &directory, Q3ValueList<KIO::UDSEntry> &list);


	int lastErrorCode() const { return m_lastErrorCode; }
	QString lastErrorMessage() const { return m_lastErrorMessage; }

private slots:
	void slotEntries(KIO::Job *job, const KIO::UDSEntryList &list);
	void slotResult(KIO::Job *job);

private:
	void createEntry(KIO::UDSEntry& entry, const QString &directory,
	                 const QString &file);

	bool m_lastListingEmpty;

	/// Last error code stored in class to simplify API.
	/// Note that this means almost no method can be const.
	int m_lastErrorCode;
	QString m_lastErrorMessage;
};

#endif
