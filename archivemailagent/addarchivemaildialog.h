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

#ifndef ADDARCHIVEMAILDIALOG_H
#define ADDARCHIVEMAILDIALOG_H
#include <kdialog.h>
#include <Akonadi/Collection>

class KComboBox;
class QCheckBox;

namespace MailCommon {
  class FolderRequester;
}


class AddArchiveMailDialog : public KDialog
{
  Q_OBJECT
public:
  explicit AddArchiveMailDialog(QWidget *parent = 0);
  ~AddArchiveMailDialog();

private Q_SLOTS:
  void slotFolderChanged(const Akonadi::Collection&);

private:
  MailCommon::FolderRequester *mFolderRequester;
  KComboBox *mFormatComboBox;
  QCheckBox *mRecursiveCheckBox;
};

#endif // ADDARCHIVEMAILDIALOG_H
