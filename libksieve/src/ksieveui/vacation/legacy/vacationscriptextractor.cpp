/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "vacationscriptextractor.h"

using namespace KSieveUi::Legacy;
KSieveUi::Legacy::VacationDataExtractor::VacationDataExtractor()
    : KSieve::ScriptBuilder(),
      mContext(None),
      mNotificationInterval(0)
{
    qCDebug(LIBKSIEVE_LOG);
}

KSieveUi::Legacy::VacationDataExtractor::~VacationDataExtractor()
{

}

void KSieveUi::Legacy::VacationDataExtractor::commandStart(const QString &identifier, int lineNumber)
{
    Q_UNUSED(lineNumber);
    qCDebug(LIBKSIEVE_LOG) << "( \"" << identifier << "\" )";
    if (identifier != QLatin1String("vacation")) {
        return;
    }
    reset();
    mContext = VacationCommand;
}

void KSieveUi::Legacy::VacationDataExtractor::commandEnd(int lineNumber)
{
    Q_UNUSED(lineNumber);
    qCDebug(LIBKSIEVE_LOG);
    if (mContext != None && mContext != VacationEnd) {
        mContext = VacationEnd;
    } else {
        mContext = None;
    }
}

void KSieveUi::Legacy::VacationDataExtractor::error(const KSieve::Error &e)
{
    qCDebug(LIBKSIEVE_LOG) << e.asString() << "@" << e.line() << "," << e.column();
}

void KSieveUi::Legacy::VacationDataExtractor::finished()
{

}

void KSieveUi::Legacy::VacationDataExtractor::taggedArgument(const QString &tag)
{
    qCDebug(LIBKSIEVE_LOG) << "( \"" << tag << "\" )";
    if (mContext != VacationCommand) {
        return;
    }
    if (tag == QLatin1String("days")) {
        mContext = Days;
    } else if (tag == QLatin1String("addresses")) {
        mContext = Addresses;
    } else if (tag == QLatin1String("subject")) {
        mContext = Subject;
    }
}

void KSieveUi::Legacy::VacationDataExtractor::stringArgument(const QString &string, bool, const QString &)
{
    qCDebug(LIBKSIEVE_LOG) << "( \"" << string << "\" )";
    if (mContext == Addresses) {
        mAliases.push_back(string);
        mContext = VacationCommand;
    } else if (mContext == Subject) {
        mSubject = string;
        mContext = VacationCommand;
    } else if (mContext == VacationCommand) {
        mMessageText = string;
        mContext = VacationCommand;
    }
}

void KSieveUi::Legacy::VacationDataExtractor::numberArgument(unsigned long number, char)
{
    qCDebug(LIBKSIEVE_LOG) << "( \"" << number << "\" )";
    if (mContext != Days) {
        return;
    }
    if (number > INT_MAX) {
        mNotificationInterval = INT_MAX;
    } else {
        mNotificationInterval = number;
    }
    mContext = VacationCommand;
}

void KSieveUi::Legacy::VacationDataExtractor::stringListArgumentStart()
{

}
void KSieveUi::Legacy::VacationDataExtractor::stringListEntry(const QString &string, bool, const QString &)
{
    qCDebug(LIBKSIEVE_LOG) << "( \"" << string << "\" )";
    if (mContext != Addresses) {
        return;
    }
    mAliases.push_back(string);
}

void KSieveUi::Legacy::VacationDataExtractor::stringListArgumentEnd()
{
    qCDebug(LIBKSIEVE_LOG);
    if (mContext != Addresses) {
        return;
    }
    mContext = VacationCommand;
}

void KSieveUi::Legacy::VacationDataExtractor::reset()
{
    qCDebug(LIBKSIEVE_LOG);
    mContext = None;
    mNotificationInterval = 0;
    mAliases.clear();
    mMessageText.clear();
}
