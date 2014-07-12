/*
 * Copyright (C) 2014  Daniel Vrátil <dvratil@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef TAGPROPERTIESDIALOG_H
#define TAGPROPERTIESDIALOG_H

#include <KDialog>
#include <akonadi/tag.h>

#include <QStandardItemModel>


#include "ui_tagpropertiesdialog.h"

class TagPropertiesDialog : public KDialog
{
    Q_OBJECT

public:
    explicit TagPropertiesDialog(QWidget *parent = 0);
    explicit TagPropertiesDialog(const Akonadi::Tag &tag, QWidget *parent = 0);
    virtual ~TagPropertiesDialog();

    Akonadi::Tag tag() const;
    bool changed() const;

protected:
    void accept();

private Q_SLOTS:
    void addAttributeClicked();
    void deleteAttributeClicked();
    void attributeChanged(QStandardItem *item);

    void addRIDClicked();
    void deleteRIDClicked();
    void remoteIdChanged(QStandardItem *item);

private:
    void setupUi();

    Ui::TagPropertiesDialog ui;
    Akonadi::Tag mTag;

    QStandardItemModel *mAttributesModel;
    QStandardItemModel *mRemoteIdsModel;

    bool mChanged;
    QSet<QString> mChangedAttrs;
    QSet<QString> mRemovedAttrs;
    QSet<QString> mChangedRIDs;
    QSet<QString> mRemovedRIDs;
};

#endif // TAGPROPERTIESDIALOG_H
