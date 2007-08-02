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

#include "mobileimpl.h"

#include <kdebug.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kdesktopfile.h>

#include <qapplication.h>
#include <qeventloop.h>
#include <qdir.h>
//Added by qt3to4:
#include <Q3ValueList>

#include <sys/stat.h>

#include <kmobiletools_cfg.h>
#include <kmobiletools_devices.h>

MobileImpl::MobileImpl() : QObject()
{
	KGlobal::dirs()->addResourceType("mobile_entries",
		KStandardDirs::kde_default("data") + "mobileview");
}

bool MobileImpl::listRoot(Q3ValueList<KIO::UDSEntry> &list)
{
    kDebug() <<"MobileImpl::listRoot";

    QStringList names_found;
    KMobileTools::MainConfig::self()->readConfig();
    QStringList dirList = KMobileTools::MainConfig::devicelist();
    kDebug() <<"MobileImpl::" << dirList;
    QStringList::ConstIterator dirpath = dirList.begin();
    QStringList::ConstIterator end = dirList.end();
    for(; dirpath!=end; ++dirpath)
    {
        KMobileTools::DevicesConfig::prefs(*dirpath)->readConfig();
        if ( ! KMobileTools::DevicesConfig::prefs(*dirpath)->fstype() ) continue;
        KIO::UDSEntry entry;
        entry.clear();
        createEntry( entry,*dirpath,  DeviceConfigInstance(*dirpath)->devicename() );
        list.append(entry);
        names_found.append(*dirpath);
    }

    return true;
}

bool MobileImpl::listDirectory(const QString &directory, Q3ValueList<KIO::UDSEntry> &list)
{
    kDebug() <<"MobileImpl::listDirectory";

    QStringList names_found;
    QStringList dirList = QStringList("a");
    kDebug() <<"MobileImpl::" << dirList;
    QStringList::ConstIterator dirpath = dirList.begin();
    QStringList::ConstIterator end = dirList.end();
    for(; dirpath!=end; ++dirpath)
    {
        KIO::UDSEntry entry;
        entry.clear();
        QString dir_found=directory.append("/").append(*dirpath);
        if(!dir_found.find('/') ) dir_found=dir_found.mid(1);
        createEntry( entry,dir_found,  *dirpath );
        list.append(entry);
        names_found.append(dir_found);
    }

    return true;
}


static void addAtom(KIO::UDSEntry &entry, unsigned int ID, long l,
                    const QString &s = QString() )
{
	KIO::UDSAtom atom;
	atom.m_uds = ID;
	atom.m_long = l;
	atom.m_str = s;
	entry.append(atom);
}


void MobileImpl::createTopLevelEntry(KIO::UDSEntry &entry) const
{
	entry.clear();
	addAtom(entry, KIO::UDSEntry::UDS_NAME, 0, ".");
	addAtom(entry, KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
	addAtom(entry, KIO::UDSEntry::UDS_ACCESS, 0555);
	addAtom(entry, KIO::UDSEntry::UDS_MIME_TYPE, 0, "inode/system_directory");
	addAtom(entry, KIO::UDSEntry::UDS_ICON_NAME, 0, "mobile");
	addAtom(entry, KIO::UDSEntry::UDS_USER, 0, "root");
	addAtom(entry, KIO::UDSEntry::UDS_GROUP, 0, "root");
}

void MobileImpl::createEntry(KIO::UDSEntry &entry,
                             const QString &directory,
                             const QString &file)
{
    kDebug() <<"MobileImpl::createEntry:" << directory <<"<->" << file;

	KDesktopFile desktop(directory+file, true);

	kDebug() <<"path =" << directory << file;

	entry.clear();

	addAtom(entry, KIO::UDSEntry::UDS_NAME, 0, file);
	addAtom(entry, KIO::UDSEntry::UDS_URL, 0, "mobile:/"+directory);

	addAtom(entry, KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
	addAtom(entry, KIO::UDSEntry::UDS_MIME_TYPE, 0, "inode/directory");

	QString icon = desktop.readIcon();
	QString empty_icon = desktop.readEntry("EmptyIcon");

	if (!empty_icon.isEmpty())
	{
		KUrl url = desktop.readURL();

		m_lastListingEmpty = true;

		KIO::ListJob *job = KIO::listDir(url, false, false);
		connect( job, SIGNAL( entries(KIO::Job *,
		                      const KIO::UDSEntryList &) ),
		         this, SLOT( slotEntries(KIO::Job *,
			             const KIO::UDSEntryList &) ) );
		connect( job, SIGNAL( result(KIO::Job *) ),
		         this, SLOT( slotResult(KIO::Job *) ) );
		qApp->eventLoop()->enterLoop();

		if (m_lastListingEmpty) icon = empty_icon;
	}

	addAtom(entry, KIO::UDSEntry::UDS_ICON_NAME, 0, icon);
}

void MobileImpl::slotEntries(KIO::Job *job, const KIO::UDSEntryList &list)
{
	if (list.size()>0)
	{
		job->kill(true);
		m_lastListingEmpty = false;
		qApp->eventLoop()->exitLoop();
	}
}

void MobileImpl::slotResult(KIO::Job *)
{
	qApp->eventLoop()->exitLoop();
}


#include "mobileimpl.moc"
