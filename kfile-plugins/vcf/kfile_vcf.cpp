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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include <kdebug.h>
#include <config.h>
#include "kfile_vcf.h"

#include <kprocess.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kstringvalidator.h>

#include <qdict.h>
#include <qvalidator.h>
#include <qcstring.h>
#include <qfile.h>
#include <qdatetime.h>

#if !defined(__osf__)
#include <inttypes.h>
#else
typedef unsigned short uint32_t;
#endif

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

    char id_name[] = "FN:";
    char id_email[] = "EMAIL;INTERNET:";

    // we need a buffer for lines
    char linebuf[1000];

    // we need a buffer for other stuff
    char buf_name[1000] = "";
    char buf_email[1000] = "";
    buf_name[999] = '\0';
    buf_email[999] = '\0';
    char * myptr;

    // FIXME: This is intensely inefficient!!!

    bool done=false;
    while (!done) {

        // read a line
        int r = file.readLine(linebuf, sizeof( linebuf ));

        if ( r < 0 ) {
            done = true;
            break;
        }

        // have we got something useful?
        if (memcmp(linebuf, id_name, 3) == 0) {
            // we have a name
            myptr = linebuf + 3;
            strlcpy(buf_name, myptr, sizeof( buf_name ));
        } else if (memcmp(linebuf, id_email, 15) == 0) {
            // we have an email
            myptr = linebuf + 15;
            strlcpy(buf_email, myptr, sizeof( buf_email ));
        }

        // are we done yet?
        if ((strlen(buf_name) > 0 && strlen(buf_email) > 0) || file.atEnd())
            done = true;

    }


    KFileMetaInfoGroup group = appendGroup(info, "Technical");

    if (strlen(buf_name) > 0)
        appendItem(group, "Name", QString::fromUtf8(buf_name));

    if (strlen(buf_email) > 0)
        appendItem(group, "Email", buf_email);

    return true;
}

#include "kfile_vcf.moc"
