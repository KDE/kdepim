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

#include "importexportprogressindicatorbase.h"
#include <KMessageBox>

ImportExportProgressIndicatorBase::ImportExportProgressIndicatorBase(QObject *parent)
    : QObject(parent),
      mNumberOfStep(0)
{

}

ImportExportProgressIndicatorBase::~ImportExportProgressIndicatorBase()
{

}

void ImportExportProgressIndicatorBase::increaseProgressDialog()
{
    //Nothing
}

void ImportExportProgressIndicatorBase::createProgressDialog(const QString &title)
{
    //Nothing
}

void ImportExportProgressIndicatorBase::showInfo(const QString &text)
{
    Q_EMIT info(text);
}

bool ImportExportProgressIndicatorBase::wasCanceled() const
{
    return false;
}

void ImportExportProgressIndicatorBase::setNumberOfStep(int numberOfStep)
{
    mNumberOfStep = numberOfStep;
}

int ImportExportProgressIndicatorBase::mergeConfigMessageBox(const QString &configName) const
{
    return KMessageBox::Yes;
}

bool ImportExportProgressIndicatorBase::overwriteConfigMessageBox(const QString &configName) const
{
    Q_UNUSED(configName);
    return true;
}

bool ImportExportProgressIndicatorBase::overwriteDirectoryMessageBox(const QString &directory) const
{
    Q_UNUSED(directory);
    return true;
}

void ImportExportProgressIndicatorBase::showErrorMessage(const QString &message, const QString &title)
{
    Q_UNUSED(message);
    Q_UNUSED(title);
}
