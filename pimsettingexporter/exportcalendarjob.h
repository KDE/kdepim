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

#ifndef EXPORTCALENDARJOB_H
#define EXPORTCALENDARJOB_H

#include "abstractimportexportjob.h"

class QWidget;
class QProgressDialog;
class ArchiveStorage;

class ExportCalendarJob : public AbstractImportExportJob
{
  Q_OBJECT
public:
  explicit ExportCalendarJob(QWidget *parent, ArchiveStorage *archiveStorage);
  ~ExportCalendarJob();
  void start();
private:
  void backupResources();
};

#endif // EXPORTCALENDARJOB_H
