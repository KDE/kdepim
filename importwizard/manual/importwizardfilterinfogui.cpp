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

#include "importwizardfilterinfogui.h"
#include <KMessageBox>
#include <QApplication>

ImportWizardFilterInfoGui::ImportWizardFilterInfoGui(ManualImportMailPage *dlg, QWidget *parent)
    : MailImporter::FilterInfoGui(),
      m_parent(parent),
      mManualImportMailPage(dlg)
{
}

ImportWizardFilterInfoGui::~ImportWizardFilterInfoGui()
{
}

void ImportWizardFilterInfoGui::setStatusMessage(const QString &status)
{
    mManualImportMailPage->widget()->mMailImporterWidget->setStatusMessage(status);
}

void ImportWizardFilterInfoGui::setFrom(const QString &from)
{
    mManualImportMailPage->widget()->mMailImporterWidget->setFrom(from);
}

void ImportWizardFilterInfoGui::setTo(const QString &to)
{
    mManualImportMailPage->widget()->mMailImporterWidget->setTo(to);
}

void ImportWizardFilterInfoGui::setCurrent(const QString &current)
{
    mManualImportMailPage->widget()->mMailImporterWidget->setCurrent(current);
    qApp->processEvents();
}

void  ImportWizardFilterInfoGui::setCurrent(int percent)
{
    mManualImportMailPage->widget()->mMailImporterWidget->setCurrent(percent);
    qApp->processEvents(); // Be careful - back & finish buttons disabled, so only user event that can happen is cancel/close button
}

void  ImportWizardFilterInfoGui::setOverall(int percent)
{
    mManualImportMailPage->widget()->mMailImporterWidget->setOverall(percent);
}

void ImportWizardFilterInfoGui::addInfoLogEntry(const QString &log)
{
    mManualImportMailPage->widget()->mMailImporterWidget->addInfoLogEntry(log);
    mManualImportMailPage->widget()->mMailImporterWidget->setLastCurrentItem();
    qApp->processEvents();
}

void ImportWizardFilterInfoGui::addErrorLogEntry(const QString &log)
{
    mManualImportMailPage->widget()->mMailImporterWidget->addErrorLogEntry(log);
    mManualImportMailPage->widget()->mMailImporterWidget->setLastCurrentItem();
    qApp->processEvents();
}

void ImportWizardFilterInfoGui::clear()
{
    mManualImportMailPage->widget()->mMailImporterWidget->clear();
}

void ImportWizardFilterInfoGui::alert(const QString &message)
{
    KMessageBox::information(m_parent, message);
}

QWidget *ImportWizardFilterInfoGui::parent() const
{
    return m_parent;
}

