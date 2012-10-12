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

#ifndef IMPORTABSTRACTAUTOCORRECTION_H
#define IMPORTABSTRACTAUTOCORRECTION_H

#include <QSet>
#include <QString>
#include <QHash>

#include "autocorrection/kmcomposerautocorrection.h"

namespace MessageComposer {
class ImportAbstractAutocorrection
{
public:
  explicit ImportAbstractAutocorrection(QWidget *parent);
  virtual ~ImportAbstractAutocorrection();

  virtual bool import(const QString& fileName) = 0;

  QSet<QString> upperCaseExceptions() const;
  QSet<QString> twoUpperLetterExceptions() const;
  QHash<QString, QString> autocorrectEntries() const;

  KMComposerAutoCorrection::TypographicQuotes typographicSingleQuotes() const;
  KMComposerAutoCorrection::TypographicQuotes typographicDoubleQuotes() const;

protected:
  QSet<QString> mUpperCaseExceptions;
  QSet<QString> mTwoUpperLetterExceptions;
  QHash<QString, QString> mAutocorrectEntries;
  KMComposerAutoCorrection::TypographicQuotes mTypographicSingleQuotes;
  KMComposerAutoCorrection::TypographicQuotes mTypographicDoubleQuotes;
  QWidget *mParent;
};
}

#endif // IMPORTABSTRACTAUTOCORRECTION_H
