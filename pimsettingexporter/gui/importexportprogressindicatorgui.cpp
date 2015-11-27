/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "importexportprogressindicatorgui.h"
#include "pimsettingexporterglobalconfig.h"
#include <qprogressdialog.h>
#include <KMessageBox>
#include <KLocalizedString>

ImportExportProgressIndicatorGui::ImportExportProgressIndicatorGui(QWidget *parentWidget, QObject *parent)
    : ImportExportProgressIndicatorBase(parent),
      mProgressDialog(Q_NULLPTR),
      mParentWidget(parentWidget)
{

}

ImportExportProgressIndicatorGui::~ImportExportProgressIndicatorGui()
{
    delete mProgressDialog;
}

void ImportExportProgressIndicatorGui::increaseProgressDialog()
{
    if (mProgressDialog) {
        mProgressDialog->setValue(mProgressDialog->value() + 1);
    }
}

void ImportExportProgressIndicatorGui::createProgressDialog(const QString &title)
{
    if (!mProgressDialog) {
        mProgressDialog = new QProgressDialog(QString(), i18n("Cancel"), 0, mNumberOfStep, mParentWidget);
        mProgressDialog->setWindowTitle(title);
        mProgressDialog->setWindowModality(Qt::WindowModal);
        connect(mProgressDialog, &QProgressDialog::canceled, this, &ImportExportProgressIndicatorBase::canceled);
    }
    mProgressDialog->show();
    mProgressDialog->setValue(0);
}

void ImportExportProgressIndicatorGui::setProgressDialogLabel(const QString &text)
{
    if (mProgressDialog) {
        mProgressDialog->setLabelText(text);
    }
    Q_EMIT info(text);
}

bool ImportExportProgressIndicatorGui::wasCanceled() const
{
    if (mProgressDialog) {
        return mProgressDialog->wasCanceled();
    }
    return false;
}

int ImportExportProgressIndicatorGui::mergeConfigMessageBox(const QString &configName) const
{
    if (PimSettingExportGlobalConfig::self()->alwaysMergeConfigFile()) {
        return KMessageBox::Yes;
    }
    return KMessageBox::warningYesNoCancel(mParentWidget, i18n("\"%1\" already exists. Do you want to overwrite it or merge it?", configName), i18n("Restore"), KGuiItem(i18n("Overwrite")), KGuiItem(i18n("Merge")));
}

bool ImportExportProgressIndicatorGui::overwriteConfigMessageBox(const QString &configName) const
{
    if (PimSettingExportGlobalConfig::self()->alwaysOverrideFile()) {
        return true;
    }
    return (KMessageBox::warningYesNo(mParentWidget, i18n("\"%1\" already exists. Do you want to overwrite it?", configName), i18n("Restore")) == KMessageBox::Yes);
}

bool ImportExportProgressIndicatorGui::overwriteDirectoryMessageBox(const QString &directory) const
{
    if (PimSettingExportGlobalConfig::self()->alwaysOverrideDirectory()) {
        return true;
    }
    return (KMessageBox::warningYesNo(mParentWidget, i18n("Directory \"%1\" already exists. Do you want to overwrite it?", directory), i18n("Restore")) == KMessageBox::Yes);
}

void ImportExportProgressIndicatorGui::showErrorMessage(const QString &message, const QString &title)
{
    KMessageBox::error(mParentWidget, message, title);
}

