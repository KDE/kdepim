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

#ifndef SIEVETEMPLATEEDITDIALOG_H
#define SIEVETEMPLATEEDITDIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;

namespace PimCommon {
class PlainTextEditFindBar;
}


namespace KSieveUi {
class SieveTextEdit;
class SieveTemplateEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SieveTemplateEditDialog(QWidget *parent = 0, bool defaultTemplate = false);
    ~SieveTemplateEditDialog();

    void setTemplateName(const QString &name);
    QString templateName() const;

    void setScript(const QString &);
    QString script() const;

private Q_SLOTS:
    void slotTemplateChanged();
    void slotFind();

private:
    void readConfig();
    void writeConfig();
    SieveTextEdit *mTextEdit;
    PimCommon::PlainTextEditFindBar *mFindBar;
    QLineEdit *mTemplateNameEdit;
    QPushButton *mOkButton;
};
}

#endif // SIEVETEMPLATEEDITDIALOG_H
