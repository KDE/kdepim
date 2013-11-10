/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#ifndef SHOWARCHIVESTRUCTUREDIALOG_H
#define SHOWARCHIVESTRUCTUREDIALOG_H

#include <KDialog>

class QTreeWidget;
class QTreeWidgetItem;

class ShowArchiveStructureDialog : public KDialog
{
    Q_OBJECT
public:
    explicit ShowArchiveStructureDialog(const KUrl &archiveUrl, QWidget *parent=0);
    ~ShowArchiveStructureDialog();

private:
    void fillTree(const KUrl &archiveUrl);

private:
    QTreeWidgetItem *addTopItem(const QString &name);

    QTreeWidget *mTreeWidget;
};

#endif // SHOWARCHIVESTRUCTUREDIALOG_H
