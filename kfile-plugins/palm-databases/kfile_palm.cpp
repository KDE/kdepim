/* This file is part of the KDE project
 * Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
 * Based on the vcf plugin:
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

#include "kfile_palm.h"

#include <klocale.h>
#include <kgenericfactory.h>

#include <qfile.h>
#include <qdatetime.h>
#include <pi-file.h>


typedef KGenericFactory<KPalmPlugin> PalmFactory;

K_EXPORT_COMPONENT_FACTORY(kfile_palm, PalmFactory( "kfile_palm" ))

KPalmPlugin::KPalmPlugin(QObject *parent, const char *name,
                       const QStringList &args)

    : KFilePlugin(parent, name, args)
{
    KFileMimeTypeInfo* info = addMimeTypeInfo( "application/vnd.palm" );

    KFileMimeTypeInfo::GroupInfo* group;
    KFileMimeTypeInfo::ItemInfo* item;

    group = addGroupInfo(info, "General", i18n("General Information"));
    item = addItemInfo(group, "Name", i18n("Name"), QVariant::String);
    item = addItemInfo(group, "DBType", i18n("DB Type"), QVariant::String);
    item = addItemInfo(group, "TypeID", i18n("Type ID"), QVariant::String);
    item = addItemInfo(group, "CreatorID", i18n("Creator ID"), QVariant::String);
    item = addItemInfo(group, "NrRecords", i18n("# of Records"), QVariant::Int);

    group = addGroupInfo(info, "TimeStamps", i18n("Time Stamps"));
    item = addItemInfo(group, "CreationDate", i18n("Creation Date"), QVariant::DateTime);
    item = addItemInfo(group, "ModificationDate", i18n("Modification Date"), QVariant::DateTime);
    item = addItemInfo(group, "BackupDate", i18n("Backup Date"), QVariant::DateTime);

    group = addGroupInfo(info, "Flags", i18n("Flags"));
    item = addItemInfo(group, "ReadOnly", i18n("Read-Only"), QVariant::String);
    item = addItemInfo(group, "MakeBackup", i18n("Make Backup"), QVariant::String);
    item = addItemInfo(group, "CopyProtected", i18n("Copy Protected"), QVariant::String);
    item = addItemInfo(group, "Reset", i18n("Reset Handheld After Installing"), QVariant::String);
    item = addItemInfo(group, "ExcludeFromSync", i18n("Exclude From Sync"), QVariant::String);
}


bool KPalmPlugin::readInfo( KFileMetaInfo& info, uint /*what*/ )
{
    int nrRec;
    QString tempName = info.path();
    QCString fileName = QFile::encodeName(tempName);
    pi_file *dbFile = pi_file_open(const_cast < char *>((const char *) fileName));
    if (dbFile == 0L) return false;
    
    struct DBInfo dBInfo;
    pi_file_get_info( dbFile, &dBInfo );
    pi_file_get_entries( dbFile, &nrRec );
    pi_file_close(dbFile);

    KFileMetaInfoGroup generalGroup = appendGroup(info, "General");
    appendItem(generalGroup, "Name", dBInfo.name );
    appendItem(generalGroup, "DBType", (dBInfo.flags & dlpDBFlagResource)?i18n("PalmOS Application"):i18n("PalmOS Record Database") );

    char buff[5];
    set_long(buff, dBInfo.type);
    buff[4]='\0';
    appendItem(generalGroup, "TypeID", buff );
    
    set_long(buff, dBInfo.creator);
    buff[4]='\0';
    appendItem(generalGroup, "CreatorID", buff );
    appendItem(generalGroup, "NrRecords", nrRec );

    
    KFileMetaInfoGroup timeGroup = appendGroup(info, "TimeStamps");
    QDateTime tm;
    tm.setTime_t( dBInfo.createDate );
    appendItem(timeGroup, "CreationDate", tm);
    tm.setTime_t( dBInfo.modifyDate );
    appendItem(timeGroup, "ModificationDate", tm);
    tm.setTime_t( dBInfo.backupDate );
    appendItem(timeGroup, "BackupDate", tm);

    KFileMetaInfoGroup flagGroup = appendGroup(info, "Flags");
    appendItem(flagGroup, "ReadOnly", (dBInfo.flags & dlpDBFlagReadOnly)?i18n("Yes"):i18n("No") );
    appendItem(flagGroup, "MakeBackup", (dBInfo.flags & dlpDBFlagBackup)?i18n("Yes"):i18n("No") );
    appendItem(flagGroup, "CopyProtected", (dBInfo.flags & dlpDBFlagCopyPrevention)?i18n("Yes"):i18n("No") );
    appendItem(flagGroup, "Reset", (dBInfo.flags & dlpDBFlagReset)?i18n("Yes"):i18n("No") );
    appendItem(flagGroup, "ExcludeFromSync", (dBInfo.miscFlags & dlpDBMiscFlagExcludeFromSync)?i18n("Yes"):i18n("No") );

    return true;
}

/*bool KPalmPlugin::writeInfo( const KFileMetaInfo& info ) const
{
//    int pi_file_set_info((struct pi_file * pf, struct DBInfo * infop));
//info["tuteTextTechnical"].value("An integer").toInt()
//  Do the stuff with low-level functions. See lines 1119-1142 of pi-file.cc for writing, 244-273 for reading.
}*/

#include "kfile_palm.moc"
