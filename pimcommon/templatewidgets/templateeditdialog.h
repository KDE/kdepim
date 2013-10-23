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

#ifndef SIEVETEMPLATEEDITDIALOG_H
#define SIEVETEMPLATEEDITDIALOG_H

#include <KDialog>

class KLineEdit;
class KTextEdit;
namespace PimCommon {
class RichTextEditorWidget;
class TemplateEditDialog : public KDialog
{
    Q_OBJECT
public:
    explicit TemplateEditDialog(QWidget *parent = 0, bool defaultTemplate = false);
    ~TemplateEditDialog();

    void setTemplateName(const QString &name);
    QString templateName() const;

    void setScript(const QString &);
    QString script() const;

private Q_SLOTS:
    void slotTemplateNameChanged(const QString &);

private:
    void readConfig();
    void writeConfig();
    PimCommon::RichTextEditorWidget *mTextEdit;
    KLineEdit *mTemplateNameEdit;
};
}

#endif // TEMPLATEEDITDIALOG_H
