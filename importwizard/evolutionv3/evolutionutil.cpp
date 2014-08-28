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

#include "evolutionutil.h"

#include <QDebug>

#include <QDomDocument>
#include <QFile>

bool EvolutionUtil::loadInDomDocument(QFile *file, QDomDocument &doc)
{
    QString errorMsg;
    int errorRow;
    int errorCol;
    if (!doc.setContent(file, &errorMsg, &errorRow, &errorCol)) {
        qDebug() << "Unable to load document.Parse error in line " << errorRow
                 << ", col " << errorCol << ": " << errorMsg;
        return false;
    }
    return true;
}

bool EvolutionUtil::loadInDomDocument(const QString &file, QDomDocument &doc)
{
    QString errorMsg;
    int errorRow;
    int errorCol;
    if (!doc.setContent(file, &errorMsg, &errorRow, &errorCol)) {
        qDebug() << "Unable to load document.Parse error in line " << errorRow
                 << ", col " << errorCol << ": " << errorMsg;
        return false;
    }
    return true;
}
