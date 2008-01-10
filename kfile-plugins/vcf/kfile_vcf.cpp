/* This file is part of the KDE project
 * Copyright (C) 2002 Shane Wright <me@shanewright.co.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <kdebug.h>
#include <config.h>
#include "kfile_vcf.h"

#include <kprocess.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kabc/vcardconverter.h>

#include <qdict.h>
#include <qfile.h>

typedef KGenericFactory<KVcfPlugin> VcfFactory;

K_EXPORT_COMPONENT_FACTORY(kfile_vcf, VcfFactory( "kfile_vcf" ))

KVcfPlugin::KVcfPlugin(QObject *parent, const char *name,
                       const QStringList &args)

    : KFilePlugin(parent, name, args)
{
    KFileMimeTypeInfo* info = addMimeTypeInfo( "text/x-vcard" );

    KFileMimeTypeInfo::GroupInfo* group = 0L;

    group = addGroupInfo(info, "Technical", i18n("Technical Details"));

    KFileMimeTypeInfo::ItemInfo* item;

    item = addItemInfo(group, "Name", i18n("Name"), QVariant::String);
    item = addItemInfo(group, "Email", i18n("Email"), QVariant::String);
    item = addItemInfo(group, "Telephone", i18n("Telephone"), QVariant::String);
}


bool KVcfPlugin::readInfo( KFileMetaInfo& info, uint /*what*/ )
{
    QFile file(info.path());

    if (!file.open(IO_ReadOnly))
    {
        kdDebug(7034) << "Couldn't open " << QFile::encodeName(info.path()) << endl;
        return false;
    }

    // even the vcard thumbnail QString::fromUtf8(buf_name));creator reads the full file ...
    // The following is partly copied from there
    QString contents = file.readAll();
    file.close();

    KABC::VCardConverter converter;
    KABC::Addressee addr = converter.parseVCard(contents);

    KFileMetaInfoGroup group = appendGroup(info, "Technical");

    // prepare the text
    QString name = addr.formattedName().simplifyWhiteSpace();
    if ( name.isEmpty() )
      name = addr.givenName() + " " + addr.familyName();
    name = name.simplifyWhiteSpace();

    if ( ! name.isEmpty() )
        appendItem(group, "Name", name);

    if ( ! addr.preferredEmail().isEmpty() )
        appendItem(group, "Email", addr.preferredEmail());

    KABC::PhoneNumber::List pnList = addr.phoneNumbers();
    QStringList phoneNumbers;
    for (unsigned int no=0; no<pnList.count(); ++no) {
      QString pn = pnList[no].number().simplifyWhiteSpace();
      if (!pn.isEmpty() && !phoneNumbers.contains(pn))
        phoneNumbers.append(pn);
    }
    if ( !phoneNumbers.isEmpty() )
        appendItem(group, "Telephone", phoneNumbers.join("\n"));

    return true;
}

#include "kfile_vcf.moc"
