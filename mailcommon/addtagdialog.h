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

#ifndef ADDTAGDIALOG_H
#define ADDTAGDIALOG_H

#include "mailcommon_export.h"
#include "tag.h"
#include <KDialog>
class KActionCollection;

namespace MailCommon {
class TagWidget;

class MAILCOMMON_EXPORT AddTagDialog : public KDialog
{
  Q_OBJECT
public:
  explicit AddTagDialog(const QList<KActionCollection *>& actions, QWidget *parent = 0);
  ~AddTagDialog();

  void setTags(const QList<MailCommon::Tag::Ptr>& tags);
  QString label() const;
  QString nepomukUrl() const;
private Q_SLOTS:
  void slotOk();
  void slotTagNameChanged(const QString& text);

private:
  QString mLabel;
  QString mNepomukUrl;
  MailCommon::TagWidget *mTagWidget;
  QList<MailCommon::Tag::Ptr> mTags;
};
}
#endif // ADDTAGDIALOG_H
