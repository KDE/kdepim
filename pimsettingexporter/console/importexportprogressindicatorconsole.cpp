/*
  Copyright (c) 2015-2016 Montel Laurent <montel@kde.org>

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

#include "importexportprogressindicatorconsole.h"
#include <QDebug>

ImportExportProgressIndicatorConsole::ImportExportProgressIndicatorConsole(QObject *parent)
    : ImportExportProgressIndicatorBase(parent)
{

}

ImportExportProgressIndicatorConsole::~ImportExportProgressIndicatorConsole()
{
}

void ImportExportProgressIndicatorConsole::setProgressDialogLabel(const QString &text)
{
    Q_EMIT info(text);
}

void ImportExportProgressIndicatorConsole::showErrorMessage(const QString &message, const QString &title)
{
    qDebug() << " message " << message << " title " << title;
}

