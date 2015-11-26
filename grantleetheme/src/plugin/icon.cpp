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

#include "icon.h"

#include <grantlee/exception.h>
#include <grantlee/parser.h>
#include <grantlee/variable.h>

#include <QDebug>

#include <KIconLoader>

IconTag::IconTag(QObject *parent)
    : Grantlee::AbstractNodeFactory(parent)
{
}

IconTag::~IconTag()
{
}

Grantlee::Node *IconTag::getNode(const QString &tagContent, Grantlee::Parser *p) const
{
    Q_UNUSED(p);

    static QHash<QString, int> sizeOrGroupLookup
    = { { QStringLiteral("desktop"), KIconLoader::Desktop },
        { QStringLiteral("toolbar"), KIconLoader::Toolbar },
        { QStringLiteral("maintoolbar"), KIconLoader::MainToolbar },
        { QStringLiteral("small"), KIconLoader::Small },
        { QStringLiteral("panel"), KIconLoader::Panel },
        { QStringLiteral("dialog"), KIconLoader::Dialog },
        { QStringLiteral("sizesmall"), KIconLoader::SizeSmall },
        { QStringLiteral("sizesmallmedium"), KIconLoader::SizeSmallMedium },
        { QStringLiteral("sizemedium"), KIconLoader::SizeMedium },
        { QStringLiteral("sizelarge"), KIconLoader::SizeLarge },
        { QStringLiteral("sizehuge"), KIconLoader::SizeHuge },
        { QStringLiteral("sizeenormous"), KIconLoader::SizeEnormous }
    };

    const QStringList parts = smartSplit(tagContent);
    const int partsSize = parts.size();
    if (partsSize < 2) {
        throw Grantlee::Exception(Grantlee::TagSyntaxError, QStringLiteral("icon tag takes at least 1 argument"));
    }
    if (partsSize > 4) {
        throw Grantlee::Exception(Grantlee::TagSyntaxError, QStringLiteral("icon tag takes at maximum 3 arguments, %1 given").arg(partsSize));
    }

    int sizeOrGroup = KIconLoader::Small;
    QString altText;
    if (partsSize >= 3) {
        const QString sizeStr = parts.at(2);
        bool ok = false;
        // Try to convert to pixel size
        sizeOrGroup = sizeStr.toInt(&ok);
        if (!ok) {
            // If failed, then try to map the string to one of tne enums
            const auto size = sizeOrGroupLookup.constFind(sizeStr);
            if (size == sizeOrGroupLookup.cend()) {
                // If it's not  a valid size string, assume it's an alt text
                altText = sizeStr;
            } else {
                sizeOrGroup = (*size);
            }
        }
    }
    if (partsSize == 4) {
        altText = parts.at(3);
    }

    return new IconNode(parts.at(1), sizeOrGroup, altText);
}

IconNode::IconNode(QObject *parent)
    : Grantlee::Node(parent)
{
}

IconNode::IconNode(const QString &iconName, int sizeOrGroup, const QString &altText, QObject *parent)
    : Grantlee::Node(parent)
    , mIconName(iconName)
    , mAltText(altText)
    , mSizeOrGroup(sizeOrGroup)
{
}

IconNode::~IconNode()
{
}

void IconNode::render(Grantlee::OutputStream *stream, Grantlee::Context *c) const
{
    Q_UNUSED(c);

    QString iconName = mIconName;
    if (iconName.startsWith(QLatin1Char('"')) && iconName.endsWith(QLatin1Char('"'))) {
        iconName = iconName.mid(1, iconName.size() - 2);
    } else {
        const QVariant val = Grantlee::Variable(mIconName).resolve(c);
        if (val.type() == QVariant::String) {
            iconName = val.toString();
        } else {
            iconName = val.value<Grantlee::SafeString>().get();
        }
    }

    QString altText;
    if (!mAltText.isEmpty()) {
        if (mAltText.startsWith(QLatin1Char('"')) && mAltText.endsWith(QLatin1Char('"'))) {
            altText = mAltText.mid(1, mAltText.size() - 2);
        } else {
            const QVariant v = Grantlee::Variable(mAltText).resolve(c);
            if (v.isValid()) {
                if (v.canConvert<Grantlee::SafeString>()) {
                    altText = v.value<Grantlee::SafeString>().get();
                } else {
                    altText = v.toString();
                }
            }
        }
    }

    const QString html = QStringLiteral("<img src=\"file://%1\" align=\"top\" height=\"%2\" width=\"%2\" alt=\"%3\" title=\"%4\" />")
                         .arg(KIconLoader::global()->iconPath(iconName, mSizeOrGroup < KIconLoader::LastGroup ? mSizeOrGroup : -mSizeOrGroup))
                         .arg(mSizeOrGroup < KIconLoader::LastGroup ?
                              IconSize(static_cast<KIconLoader::Group>(mSizeOrGroup))
                              : mSizeOrGroup)
                         .arg(altText.isEmpty() ? iconName : altText)
                         .arg(altText); // title is intentionally blank if no alt is provided
    (*stream) << Grantlee::SafeString(html, Grantlee::SafeString::IsSafe);
}
