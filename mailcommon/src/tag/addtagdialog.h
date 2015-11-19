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

#ifndef ADDTAGDIALOG_H
#define ADDTAGDIALOG_H

#include "mailcommon_export.h"
#include "tag.h"
#include <QDialog>
#include <AkonadiCore/Tag>
#include <AkonadiCore/TagCreateJob>
#include <KConfigGroup>

class KActionCollection;

namespace MailCommon
{
class AddTagDialogPrivate;
class MAILCOMMON_EXPORT AddTagDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddTagDialog(const QList<KActionCollection *> &actions, QWidget *parent = Q_NULLPTR);
    ~AddTagDialog();

    void setTags(const QList<MailCommon::Tag::Ptr> &tags);
    QString label() const;
    Akonadi::Tag tag() const;

private Q_SLOTS:
    void slotSave();
    void slotTagNameChanged(const QString &text);
    void onTagCreated(KJob *job);

private:
    AddTagDialogPrivate *const d;
};
}
#endif // ADDTAGDIALOG_H
