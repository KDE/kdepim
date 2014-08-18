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

#ifndef SELECTMULTICOLLECTIONDIALOG_H
#define SELECTMULTICOLLECTIONDIALOG_H
#include "pimcommon_export.h"
#include <QDialog>
#include <Collection>
#include <KConfigGroup>

namespace PimCommon {
class SelectMultiCollectionWidget;
class PIMCOMMON_EXPORT SelectMultiCollectionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SelectMultiCollectionDialog(const QString &mimetype, const QList<Akonadi::Collection::Id> &selectedCollection, QWidget *parent = 0);
    explicit SelectMultiCollectionDialog(const QString &mimetype, QWidget *parent = 0);
    ~SelectMultiCollectionDialog();

    QList<Akonadi::Collection> selectedCollection() const;

private:
    void initialize(const QString &mimetype, const QList<Akonadi::Collection::Id> &selectedCollection = QList<Akonadi::Collection::Id>());
    void writeConfig();
    void readConfig();
    SelectMultiCollectionWidget *mSelectMultiCollection;
};
}

#endif // SELECTMULTICOLLECTIONDIALOG_H
