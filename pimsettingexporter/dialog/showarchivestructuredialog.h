/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include <QDialog>

class QTreeWidget;
class QTreeWidgetItem;
class KArchiveEntry;
class KArchiveDirectory;

class ShowArchiveStructureDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ShowArchiveStructureDialog(const QString &filename, QWidget *parent = 0);
    ~ShowArchiveStructureDialog();

private Q_SLOTS:
    void slotExportAsLogFile();

private:
    void exportAsLogFile();
    bool fillTree();
    void readConfig();
    void writeConfig();

    bool searchArchiveElement(const QString &path, const KArchiveDirectory *topDirectory, const QString &name);
    QTreeWidgetItem *addTopItem(const QString &name);
    void addSubItems(QTreeWidgetItem *parent, const KArchiveEntry *entry, int indent);
    QTreeWidgetItem *addItem(QTreeWidgetItem *parent, const QString &name);
    QString mFileName;
    QString mLogFile;
    QTreeWidget *mTreeWidget;
};

#endif // SHOWARCHIVESTRUCTUREDIALOG_H
