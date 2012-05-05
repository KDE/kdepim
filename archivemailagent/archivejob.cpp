/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>
  
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

#include "archivejob.h"

ArchiveJob::ArchiveJob(const Akonadi::Collection &folder, bool immediate )
  : MailCommon::ScheduledJob( folder, immediate )
{
}

ArchiveJob::~ArchiveJob()
{

}

void ArchiveJob::execute()
{
  //TODO
}

void ArchiveJob::kill()
{
  ScheduledJob::kill();
}

#include "archivejob.moc"
