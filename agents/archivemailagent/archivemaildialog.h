/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#ifndef ARCHIVEMAILDIALOG_H
#define ARCHIVEMAILDIALOG_H

#include "archivemailinfo.h"
#include <KDialog>
class QTreeWidget;
class KAboutData;
class ArchiveMailWidget;
class ArchiveMailDialog : public KDialog
{
    Q_OBJECT
public:
    explicit ArchiveMailDialog(QWidget *parent = 0);
    ~ArchiveMailDialog();

Q_SIGNALS:
    void archiveNow(ArchiveMailInfo *info);

public Q_SLOTS:
    void slotNeedReloadConfig();


protected Q_SLOTS:
    void slotSave();

private:
    void writeConfig();
    void readConfig();
    ArchiveMailWidget *mWidget;
    KAboutData *mAboutData;
};


#endif /* ARCHIVEMAILWIDGET_H */

