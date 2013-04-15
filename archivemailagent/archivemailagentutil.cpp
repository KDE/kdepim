/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  
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

#include "archivemailagentutil.h"
#include <KDebug>

QDate ArchiveMailAgentUtil::diffDate(ArchiveMailInfo*info)
{
    QDate diffDate(info->lastDateSaved());
    switch(info->archiveUnit()) {
    case ArchiveMailInfo::ArchiveDays:
        diffDate = diffDate.addDays(info->archiveAge());
        break;
    case ArchiveMailInfo::ArchiveWeeks:
        diffDate = diffDate.addDays(info->archiveAge()*7);
        break;
    case ArchiveMailInfo::ArchiveMonths:
        diffDate = diffDate.addMonths(info->archiveAge());
        break;
    default:
        kDebug()<<"archiveUnit not defined :"<<info->archiveUnit();
        break;
    }
    return diffDate;
}

