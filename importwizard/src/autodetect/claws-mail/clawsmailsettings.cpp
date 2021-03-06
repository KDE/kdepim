/*
   Copyright (C) 2012-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "clawsmailsettings.h"
#include "importwizardutil.h"

#include <MailTransport/mailtransport/transportmanager.h>
#include "MailCommon/MailUtil"

#include <KIdentityManagement/kidentitymanagement/identity.h>
#include <KIdentityManagement/kidentitymanagement/signature.h>

#include "importwizard_debug.h"
#include <KConfig>
#include <KConfigGroup>
#include <QFile>

#include <QRegularExpression>

ClawsMailSettings::ClawsMailSettings(ImportWizard *parent)
    : SylpheedSettings(parent)
{
}

ClawsMailSettings::~ClawsMailSettings()
{

}

void ClawsMailSettings::importSettings(const QString &filename, const QString &path)
{
    bool checkMailOnStartup = true;
    int intervalCheckMail = -1;
    const QString sylpheedrc = path + QLatin1String("/clawsrc");
    if (QFile(sylpheedrc).exists()) {
        KConfig configCommon(sylpheedrc);
        if (configCommon.hasGroup("Common")) {
            KConfigGroup common = configCommon.group("Common");
            checkMailOnStartup = (common.readEntry("check_on_startup", 1) == 1);
            if (common.readEntry(QStringLiteral("autochk_newmail"), 1) == 1) {
                intervalCheckMail = common.readEntry(QStringLiteral("autochk_interval"), -1);
            }
            readGlobalSettings(common);
        }
    }
    KConfig config(filename);
    const QStringList accountList = config.groupList().filter(QRegularExpression(QStringLiteral("Account: \\d+")));
    const QStringList::const_iterator end(accountList.constEnd());
    for (QStringList::const_iterator it = accountList.constBegin(); it != end; ++it) {
        KConfigGroup group = config.group(*it);
        readAccount(group, checkMailOnStartup, intervalCheckMail);
        readIdentity(group);
    }
    const QString customheaderrc = path + QLatin1String("/customheaderrc");
    QFile customHeaderFile(customheaderrc);
    if (customHeaderFile.exists()) {
        if (!customHeaderFile.open(QIODevice::ReadOnly)) {
            qCDebug(IMPORTWIZARD_LOG) << " We can't open file" << customheaderrc;
        } else {
            readCustomHeader(&customHeaderFile);
        }
    }
}

void ClawsMailSettings::readSettingsColor(const KConfigGroup &group)
{
    const bool enableColor = group.readEntry("enable_color", false);
    if (enableColor) {
        const QString colorLevel1 = group.readEntry("quote_level1_color");
        if (!colorLevel1.isEmpty()) {
            const QColor col = QColor(colorLevel1);
            if (col.isValid()) {
                addKmailConfig(QStringLiteral("Reader"), QStringLiteral("QuotedText1"), writeColor(col));
            }
            //[Reader]  QuotedText1
        }
        const QString colorLevel2 = group.readEntry("quote_level2_color");
        if (!colorLevel2.isEmpty()) {
            const QColor col = QColor(colorLevel2);
            if (col.isValid()) {
                addKmailConfig(QStringLiteral("Reader"), QStringLiteral("QuotedText2"), writeColor(col));
            }
            //[Reader]  QuotedText2
        }
        const QString colorLevel3 = group.readEntry("quote_level3_color");
        if (!colorLevel3.isEmpty()) {
            const QColor col = QColor(colorLevel3);
            if (col.isValid()) {
                addKmailConfig(QStringLiteral("Reader"), QStringLiteral("QuotedText3"), writeColor(col));
            }
            //[Reader]  QuotedText3
        }
        const QString misspellColor = group.readEntry(QStringLiteral("misspelled_color"));
        if (!misspellColor.isEmpty()) {
            const QColor col = QColor(misspellColor);
            if (col.isValid()) {
                addKmailConfig(QStringLiteral("Reader"), QStringLiteral("MisspelledColor"), writeColor(col));
            }
        }
        const QString uriColor = group.readEntry(QStringLiteral("uri_color"));
        if (!uriColor.isEmpty()) {
            const QColor col(uriColor);
            if (col.isValid()) {
                addKmailConfig(QStringLiteral("Reader"), QStringLiteral("LinkColor"), writeColor(col));
            }
        }
        const QString newColor = group.readEntry(QStringLiteral("color_new"));
        if (!newColor.isEmpty()) {
            const QColor col(newColor);
            if (col.isValid()) {
                addKmailConfig(QStringLiteral("MessageListView::Colors"), QStringLiteral("UnreadMessageColor"), writeColor(col));
            }
        }
    }
}

QString ClawsMailSettings::writeColor(const QColor &col)
{
    QStringList list;
    list.insert(0, QString::number(col.red()));
    list.insert(1, QString::number(col.green()));
    list.insert(2, QString::number(col.blue()));
    if (col.alpha() != 255) {
        list.insert(3, QString::number(col.alpha()));
    }
    return list.join(QLatin1Char(','));
}

void ClawsMailSettings::readTemplateFormat(const KConfigGroup &group)
{
    SylpheedSettings::readTemplateFormat(group);
    const QString composerNewMessage = group.readEntry(QStringLiteral("compose_body_format"));
    if (!composerNewMessage.isEmpty()) {
        addKmailConfig(QStringLiteral("TemplateParser"), QStringLiteral("TemplateNewMessage"), convertToKmailTemplate(composerNewMessage));
    }
}

void ClawsMailSettings::readGlobalSettings(const KConfigGroup &group)
{
    SylpheedSettings::readGlobalSettings(group);
    if (group.readEntry(QStringLiteral("check_while_typing"), 0) == 1) {
        addKmailConfig(QStringLiteral("Spelling"), QStringLiteral("backgroundCheckerEnabled"), true);
    }
    const int markAsRead = group.readEntry(QStringLiteral("mark_as_read_delay"), -1);
    if (markAsRead != -1) {
        addKmailConfig(QStringLiteral("Behaviour"), QStringLiteral("DelayedMarkTime"), markAsRead);
        addKmailConfig(QStringLiteral("Behaviour"), QStringLiteral("DelayedMarkAsRead"), true);
    }

    const int warnLargeFileInserting = group.readEntry(QStringLiteral("warn_large_insert"), 0);
    if (warnLargeFileInserting == 0) {
        addKmailConfig(QStringLiteral("Composer"), QStringLiteral("MaximumAttachmentSize"), -1);
    } else {
        const int warnLargeFileSize = group.readEntry(QStringLiteral("warn_large_insert_size"), -1);
        if (warnLargeFileSize > 0) {
            addKmailConfig(QStringLiteral("Composer"), QStringLiteral("MaximumAttachmentSize"), warnLargeFileSize * 1024);
        }
    }
}

void ClawsMailSettings::readTagColor(const KConfigGroup &group)
{
    const QString customColorPattern(QStringLiteral("custom_color%1"));
    const QString customColorLabelPattern(QStringLiteral("custom_colorlabel%1"));
    QVector<tagStruct> listTag;
    for (int i = 1; i <= 15; ++i) {
        if (group.hasKey(customColorPattern.arg(i))
                && group.hasKey(customColorLabelPattern.arg(i))) {
            tagStruct tag;
            const QString colorStr = group.readEntry(customColorPattern.arg(i));
            const QString labelStr = group.readEntry(customColorLabelPattern.arg(i));
            if (!colorStr.isEmpty() && !labelStr.isEmpty()) {
                tag.color = QColor(colorStr).name();
                tag.name = labelStr;
                listTag << tag;
            }
        }
    }
    if (!listTag.isEmpty()) {
        ImportWizardUtil::addAkonadiTag(listTag);
    }
}
