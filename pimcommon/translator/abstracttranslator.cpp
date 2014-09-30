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

#include "abstracttranslator.h"

using namespace PimCommon;

AbstractTranslator::AbstractTranslator()
{
}

AbstractTranslator::~AbstractTranslator()
{
}

void AbstractTranslator::setInputText(const QString &text)
{
    mInputText = text;
}

void AbstractTranslator::setFrom(const QString &language)
{
    mFrom = language;
}

void AbstractTranslator::setTo(const QString &language)
{
    mTo = language;
}

QString AbstractTranslator::resultTranslate() const
{
    return mResult;
}

