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

#include <config.h>
#include "kfile_rfc822.h"

#include <kprocess.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kstringvalidator.h>
#include <kdebug.h>

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

typedef KGenericFactory<KRfc822Plugin> Rfc822Factory;

K_EXPORT_COMPONENT_FACTORY(kfile_rfc822, Rfc822Factory( "kfile_rfc822" ))

KRfc822Plugin::KRfc822Plugin(QObject *parent, const char *name,
                       const QStringList &args)

    : KFilePlugin(parent, name, args)
{
    KFileMimeTypeInfo* info = addMimeTypeInfo( "message/rfc822" );

    KFileMimeTypeInfo::GroupInfo* group = 0L;

    group = addGroupInfo(info, "Technical", i18n("Technical Details"));

    KFileMimeTypeInfo::ItemInfo* item;

    item = addItemInfo(group, "From", i18n("From"), QVariant::String);
    item = addItemInfo(group, "To", i18n("To"), QVariant::String);
    item = addItemInfo(group, "Subject", i18n("Subject"), QVariant::String);
    item = addItemInfo(group, "Date", i18n("Date"), QVariant::String);
    item = addItemInfo(group, "Content-Type", i18n("Content-Type"), QVariant::String);
}


bool KRfc822Plugin::readInfo( KFileMetaInfo& info, uint /*what*/ )
{

    QFile file(info.path());

    if (!file.open(IO_ReadOnly))
    {
        kdDebug(7034) << "Couldn't open " << QFile::encodeName(info.path()) << endl;
        return false;
    }

    /*
         Note to self: probably should use QCString for all this, but
         what we're doing is simple and self-contained so never mind..
    */

    char id_from[] = "From: ";
    char id_to[] = "To: ";
    char id_subject[] = "Subject: ";
    char id_date[] = "Date: ";
    char id_contenttype[] = "Content-Type: ";

    // we need a buffer for lines
    char linebuf[4096];

    // we need a buffer for other stuff
    char buf_from[1000] = "";
    char buf_to[1000] = "";
    char buf_subject[1000] = "";
    char buf_date[1000] = "";
    char buf_contenttype[1000] = "";

    memset(buf_from, 0, 999);
    memset(buf_to, 0, 999);
    memset(buf_subject, 0, 999);
    memset(buf_date, 0, 999);
    memset(buf_contenttype, 0, 999);
    char * myptr;

    bool done=false;
    while (!done) {

        // read a line
        file.readLine(linebuf, 4095);

        // have we got something useful?
        if (memcmp(linebuf, id_from, 6) == 0) {
            // we have a name
            myptr = linebuf + 6;
            strncpy(buf_from, myptr, 999);
            buf_from[998]='\0';
        } else if (memcmp(linebuf, id_to, 4) == 0) {
            // we have a name
            myptr = linebuf + 4;
            strncpy(buf_to, myptr, 999);
            buf_to[998]='\0';
        } else if (memcmp(linebuf, id_subject, 9) == 0) {
            // we have a name
            myptr = linebuf + 9;
            strncpy(buf_subject, myptr, 999);
            buf_subject[998]='\0';
        } else if (memcmp(linebuf, id_date, 6) == 0) {
            // we have a name
            myptr = linebuf + 6;
            strncpy(buf_date, myptr, 999);
            buf_date[998]='\0';
        } else if (memcmp(linebuf, id_contenttype, 14) == 0) {
            // we have a name
            myptr = linebuf + 14;
            strncpy(buf_contenttype, myptr, 999);
            buf_contenttype[998]='\0';
        }

        // are we done yet?
        if (
          ((strlen(buf_from) > 0) && (strlen(buf_to) > 0) &&
          (strlen(buf_subject) > 0) && (strlen(buf_date) > 0) &&
          (strlen(buf_contenttype) > 0)) ||
          (file.atEnd())
          )
            done = true;

    };

    KFileMetaInfoGroup group = appendGroup(info, "Technical");

    if (strlen(buf_from) > 0)           appendItem(group, "From", buf_from);
    if (strlen(buf_to) > 0)             appendItem(group, "To", buf_to);
    if (strlen(buf_subject) > 0)        appendItem(group, "Subject", buf_subject);
    if (strlen(buf_date) > 0)           appendItem(group, "Date", buf_date);
    if (strlen(buf_contenttype) > 0)    appendItem(group, "Content-Type", buf_contenttype);

    return true;
}

#include "kfile_rfc822.moc"
