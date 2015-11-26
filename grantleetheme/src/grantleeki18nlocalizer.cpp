/*
 * Copyright (C) 2015  Daniel Vrátil <dvratil@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "grantleeki18nlocalizer_p.h"
#include "grantleetheme_debug.h"

#include <QLocale>
#include <QDate>

#include <grantlee/safestring.h>

#include <KLocalizedString>

GrantleeKi18nLocalizer::GrantleeKi18nLocalizer()
    : Grantlee::QtLocalizer()
{
}

GrantleeKi18nLocalizer::~GrantleeKi18nLocalizer()
{
}

QString GrantleeKi18nLocalizer::processArguments(const KLocalizedString &kstr,
        const QVariantList &arguments) const
{
    KLocalizedString str = kstr;
    for (auto iter = arguments.cbegin(), end = arguments.cend(); iter != end; ++iter) {
        switch (iter->type()) {
        case QVariant::String:
            str = str.subs(iter->toString());
            break;
        case QVariant::Int:
            str = str.subs(iter->toInt());
            break;
        case QVariant::UInt:
            str = str.subs(iter->toUInt());
            break;
        case QVariant::LongLong:
            str = str.subs(iter->toLongLong());
            break;
        case QVariant::ULongLong:
            str = str.subs(iter->toULongLong());
            break;
        case QVariant::Char:
            str = str.subs(iter->toChar());
            break;
        case QVariant::Double:
            str = str.subs(iter->toDouble());
            break;
        case QVariant::UserType:
            if (iter->canConvert<Grantlee::SafeString>()) {
                str = str.subs(iter->value<Grantlee::SafeString>().get());
                break;
            }
        // fall-through
        default:
            qCWarning(GRANTLEETHEME_LOG) << "Unknown type" << iter->typeName() << "(" << iter->type() << ")";
            break;
        }
    }

    // Return localized in the currenctly active locale
    return str.toString({ currentLocale() });
}

QString GrantleeKi18nLocalizer::localizeContextString(const QString &string, const QString &context, const QVariantList &arguments) const
{
    const KLocalizedString str = kxi18nc(qPrintable(context), qPrintable(string));
    return processArguments(str, arguments);
}

QString GrantleeKi18nLocalizer::localizeString(const QString &string, const QVariantList &arguments) const
{
    const KLocalizedString str = kxi18n(qPrintable(string));
    return processArguments(str, arguments);
}

QString GrantleeKi18nLocalizer::localizePluralContextString(const QString &string, const QString &pluralForm,
        const QString &context, const QVariantList &arguments) const
{
    const KLocalizedString str = kxi18ncp(qPrintable(context), qPrintable(string), qPrintable(pluralForm));
    return processArguments(str, arguments);
}

QString GrantleeKi18nLocalizer::localizePluralString(const QString &string, const QString &pluralForm,
        const QVariantList &arguments) const
{
    const KLocalizedString str = kxi18np(qPrintable(string), qPrintable(pluralForm));
    return processArguments(str, arguments);
}

QString GrantleeKi18nLocalizer::localizeMonetaryValue(qreal value, const QString &currencySymbol) const
{
    return QLocale(currentLocale()).toCurrencyString(value, currencySymbol);
}
