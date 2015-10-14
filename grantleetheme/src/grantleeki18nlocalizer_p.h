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

#ifndef GRANTLEEKI18NLOCALIZER_H
#define GRANTLEEKI18NLOCALIZER_H

#include <grantlee/qtlocalizer.h>

#include <QLocale>
#include <QVariantHash>

class KLocalizedString;

class GrantleeKi18nLocalizer : public Grantlee::QtLocalizer
{
public:
    explicit GrantleeKi18nLocalizer();
    ~GrantleeKi18nLocalizer();

    // Only reimplement string localization to use KLocalizedString instead of
    // tr(), the remaining methods use QLocale internally, so we can reuse them
    QString localizeContextString(const QString &string, const QString &context,
                                  const QVariantList &arguments) const Q_DECL_OVERRIDE;
    QString localizeString(const QString &string, const QVariantList &arguments) const Q_DECL_OVERRIDE;
    QString localizePluralContextString(const QString &string, const QString &pluralForm,
                                        const QString &context, const QVariantList &arguments) const Q_DECL_OVERRIDE;
    QString localizePluralString(const QString &string, const QString &pluralForm,
                                 const QVariantList &arguments) const Q_DECL_OVERRIDE;

    // Only exception, Grantlee's implementation is not using QLocale for this
    // for some reason
    QString localizeMonetaryValue(qreal value, const QString &currenctCode) const Q_DECL_OVERRIDE;

private:
    QString processArguments(const KLocalizedString &str,
                             const QVariantList &arguments) const;
};

#endif // GRANTLEEKI18NLOCALIZER_H
