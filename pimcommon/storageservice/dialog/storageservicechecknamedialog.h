/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#ifndef STORAGESERVICECHECKNAMEDIALOG_H
#define STORAGESERVICECHECKNAMEDIALOG_H

#include <QDialog>
#include <KConfigGroup>
#include "pimcommon_export.h"
class QLineEdit;
class QLabel;
class QPushButton;
namespace PimCommon
{
class PIMCOMMON_EXPORT StorageServiceCheckNameDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StorageServiceCheckNameDialog(QWidget *parent = 0);
    ~StorageServiceCheckNameDialog();

    void setDisallowedSymbols(const QRegExp &regExp);
    void setDisallowedSymbolsStr(const QString &str);
    void setOldName(const QString &name);

    QString newName() const;

private slots:
    void slotNameChanged(const QString &text);

private:
    QRegExp mRegExp;
    QLabel *mInfo;
    QLineEdit *mName;
    QPushButton *mOkButton;
};
}

#endif // STORAGESERVICECHECKNAMEDIALOG_H
