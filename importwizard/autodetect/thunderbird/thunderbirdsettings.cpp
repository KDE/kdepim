/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#include "thunderbirdsettings.h"
#include <MailTransport/mailtransport/transportmanager.h>
#include "mailcommon/util/mailutil.h"
#include "importwizardutil.h"

#include <KIdentityManagement/kidentitymanagement/identity.h>
#include <KIdentityManagement/kidentitymanagement/signature.h>

#include <QUrl>
#include <KContacts/VCardConverter>
#include <QTextStream>
#include <QStringList>
#include <QFile>
#include "importwizard_debug.h"
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>

ThunderbirdSettings::ThunderbirdSettings(const QString &filename, ImportWizard *parent)
    : AbstractSettings(parent)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(IMPORTWIZARD_LOG) << " We can't open file" << filename;
        return;
    }
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        if (line.startsWith(QStringLiteral("user_pref"))) {
            if (line.contains(QStringLiteral("mail.smtpserver")) ||
                    line.contains(QStringLiteral("mail.server.")) ||
                    line.contains(QStringLiteral("mail.identity.")) ||
                    line.contains(QStringLiteral("mail.account.")) ||
                    line.contains(QStringLiteral("mail.accountmanager.")) ||
                    line.contains(QStringLiteral("mailnews.")) ||
                    line.contains(QStringLiteral("mail.compose.")) ||
                    line.contains(QStringLiteral("mail.spellcheck")) ||
                    line.contains(QStringLiteral("mail.SpellCheckBeforeSend")) ||
                    line.contains(QStringLiteral("spellchecker.dictionary")) ||
                    line.contains(QStringLiteral("ldap_")) ||
                    line.contains(QStringLiteral("mail.biff.")) ||
                    line.contains(QStringLiteral("mailnews.tags.")) ||
                    line.contains(QStringLiteral("extensions.AutoResizeImage.")) ||
                    line.contains(QStringLiteral("mail.phishing.")) ||
                    line.contains(QStringLiteral("mail.display_glyph"))) {
                insertIntoMap(line);
            }
        } else {
            qCDebug(IMPORTWIZARD_LOG) << " unstored line :" << line;
        }
    }
    const QString mailAccountPreference = mHashConfig.value(QStringLiteral("mail.accountmanager.accounts")).toString();
    if (mailAccountPreference.isEmpty()) {
        return;
    }
    mAccountList = mailAccountPreference.split(QLatin1Char(','));
    readTransport();
    readAccount();
    readGlobalSettings();
    readLdapSettings();
    readTagSettings();
    readExtensionsSettings();
}

ThunderbirdSettings::~ThunderbirdSettings()
{
}

void ThunderbirdSettings::readExtensionsSettings()
{
    //AutoResizeImage
    const QString filterPatternEnabledStr = QStringLiteral("extensions.AutoResizeImage.filterPatterns");
    if (mHashConfig.contains(filterPatternEnabledStr)) {
        const int filterPatternType = mHashConfig.value(filterPatternEnabledStr).toInt();
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("filter-source-type"), filterPatternType);
    }
    const QString filterPatternListStr = QStringLiteral("extensions.AutoResizeImage.filteringPatternsList");
    if (mHashConfig.contains(filterPatternListStr)) {
        const QString filterPatternList = mHashConfig.value(filterPatternListStr).toString();
        //TODO decode it.
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("filter-source-pattern"), filterPatternList);
    }

    const QString enlargeImagesStr = QStringLiteral("extensions.AutoResizeImage.enlargeImages");
    if (mHashConfig.contains(enlargeImagesStr)) {
        const bool enlargeImages = mHashConfig.value(enlargeImagesStr).toBool();
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("enlarge-image-to-minimum"), enlargeImages);
    }

    const QString maxResolutionXStr = QStringLiteral("extensions.AutoResizeImage.maxResolutionX");
    if (mHashConfig.contains(maxResolutionXStr)) {
        const int val = mHashConfig.value(maxResolutionXStr).toInt();
        int adaptedValue = adaptAutoResizeResolution(val, QStringLiteral("extensions.AutoResizeImage.maxResolutionXList"));
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("maximum-width"), adaptedValue);
    }
    const QString maxResolutionYStr = QStringLiteral("extensions.AutoResizeImage.maxResolutionY");
    if (mHashConfig.contains(maxResolutionYStr)) {
        const int val = mHashConfig.value(maxResolutionYStr).toInt();
        int adaptedValue = adaptAutoResizeResolution(val, QStringLiteral("extensions.AutoResizeImage.maxResolutionYList"));
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("maximum-height"), adaptedValue);
    }
    const QString minResolutionXStr = QStringLiteral("extensions.AutoResizeImage.minResolutionX");
    if (mHashConfig.contains(minResolutionXStr)) {
        const int val = mHashConfig.value(minResolutionXStr).toInt();
        int adaptedValue = adaptAutoResizeResolution(val, QStringLiteral("extensions.AutoResizeImage.minResolutionXList"));
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("minimum-width"), adaptedValue);
    }
    const QString minResolutionYStr = QStringLiteral("extensions.AutoResizeImage.minResolutionY");
    if (mHashConfig.contains(minResolutionYStr)) {
        const int val = mHashConfig.value(minResolutionYStr).toInt();
        int adaptedValue = adaptAutoResizeResolution(val, QStringLiteral("extensions.AutoResizeImage.minResolutionYList"));
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("minimum-height"), adaptedValue);
    }

    //Default is true.
    const QString reduceImageStr(QStringLiteral("extensions.AutoResizeImage.reduceImages"));
    if (mHashConfig.contains(reduceImageStr)) {
        const bool reduce = mHashConfig.value(reduceImageStr).toBool();
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("reduce-image-to-maximum"), reduce);
    } else {
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("reduce-image-to-maximum"), false);
    }

    const QString filterMinimumStr(QStringLiteral("extensions.AutoResizeImage.filterMinimumSize"));
    if (mHashConfig.contains(filterMinimumStr)) {
        const bool filterMinimum = mHashConfig.value(filterMinimumStr).toBool();
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("skip-image-lower-size-enabled"), filterMinimum);
    }
    const QString skipMinimumSizeStr(QStringLiteral("extensions.AutoResizeImage.minimumSize"));
    if (mHashConfig.contains(skipMinimumSizeStr)) {
        const int skipMinimumSize = mHashConfig.value(skipMinimumSizeStr).toInt();
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("skip-image-lower-size"), skipMinimumSize);
    }
    const QString confirmBeforeResizingStr(QStringLiteral("extensions.AutoResizeImage.confirmResizing"));
    if (mHashConfig.contains(confirmBeforeResizingStr)) {
        const bool confirmBeforeResizing = mHashConfig.value(confirmBeforeResizingStr).toBool();
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("ask-before-resizing"), confirmBeforeResizing);
    }
    //extensions.AutoResizeImage.convertImages : not implemented in kmail

    const QString conversionFormatStr(QStringLiteral("extensions.AutoResizeImage.conversionFormat"));
    if (mHashConfig.contains(conversionFormatStr)) {
        QString conversionFormat = mHashConfig.value(conversionFormatStr).toString();
        if (conversionFormat == QLatin1String("png")) {
            conversionFormat = QStringLiteral("PNG");
        } else {
            conversionFormat = QStringLiteral("JPG");
        }
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("write-format"), conversionFormat);
    }

    const QString filterRecipientsStr(QStringLiteral("extensions.AutoResizeImage.filterRecipients"));
    if (mHashConfig.contains(filterRecipientsStr)) {
        const int filterRecipients = mHashConfig.value(filterRecipientsStr).toInt();
        switch (filterRecipients) {
        case 0:
            addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("FilterRecipientType"), QStringLiteral("NoFilter"));
            break;
        case 1:
            addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("FilterRecipientType"), QStringLiteral("ResizeEachEmailsContainsPattern"));
            break;
        case 2:
            addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("FilterRecipientType"), QStringLiteral("ResizeOneEmailContainsPattern"));
            break;
        case 3:
            addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("FilterRecipientType"), QStringLiteral("DontResizeEachEmailsContainsPattern"));
            break;
        case 4:
            addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("FilterRecipientType"), QStringLiteral("DontResizeOneEmailContainsPattern"));
            break;
        default:
            qCDebug(IMPORTWIZARD_LOG) << " unknown FilterRecipientType: " << filterRecipients;
            break;
        }
    }

    const QString filteringRecipientsPatternsWhiteListStr(QStringLiteral("extensions.AutoResizeImage.filteringRecipientsPatternsWhiteList"));
    if (mHashConfig.contains(filteringRecipientsPatternsWhiteListStr)) {
        const QString filteringRecipientsPatternsWhiteList = mHashConfig.value(filteringRecipientsPatternsWhiteListStr).toString();
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("resize-emails-pattern"), filteringRecipientsPatternsWhiteList);
    }

    const QString filteringRecipientsPatternsBlackListStr(QStringLiteral("extensions.AutoResizeImage.filteringRecipientsPatternsBlackList"));
    if (mHashConfig.contains(filteringRecipientsPatternsBlackListStr)) {
        const QString filteringRecipientsPatternsBlackList = mHashConfig.value(filteringRecipientsPatternsBlackListStr).toString();
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("do-not-resize-emails-pattern"), filteringRecipientsPatternsBlackList);
    }

    const QString filteringRenamingPatternStr(QStringLiteral("extensions.AutoResizeImage.renamingPattern"));
    if (mHashConfig.contains(filteringRenamingPatternStr)) {
        QString filteringRenamingPattern = mHashConfig.value(filteringRenamingPatternStr).toString();
        filteringRenamingPattern.replace(QStringLiteral("%3Fn"), QStringLiteral("%n"));
        filteringRenamingPattern.replace(QStringLiteral("%3Ft"), QStringLiteral("%t"));
        filteringRenamingPattern.replace(QStringLiteral("%3Fd"), QStringLiteral("%d"));
        filteringRenamingPattern.replace(QStringLiteral("%3Fe"), QStringLiteral("%e"));
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("rename-resized-images-pattern"), filteringRenamingPattern);
    }

    const QString filteringRenamingImageStr(QStringLiteral("extensions.AutoResizeImage.renameResizedImages"));
    if (mHashConfig.contains(filteringRenamingImageStr)) {
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("rename-resized-images"), true);
    }

    const QString filteringImageFormatsStr(QStringLiteral("extensions.AutoResizeImage.imageFormats"));
    if (mHashConfig.contains(filteringImageFormatsStr)) {
        const QString filteringImageFormats = mHashConfig.value(filteringImageFormatsStr).toString();
        //convert it.
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("resize-image-with-formats-type"), filteringImageFormats);
    }

    const QString filteringImageFormatsEnabledStr(QStringLiteral("extensions.AutoResizeImage.filterFormats"));
    if (mHashConfig.contains(filteringImageFormatsEnabledStr)) {
        const bool filteringImageFormatsEnabled = mHashConfig.value(filteringImageFormatsEnabledStr).toBool();
        addKmailConfig(QStringLiteral("AutoResizeImage"), QStringLiteral("resize-image-with-formats"), filteringImageFormatsEnabled);
    }
}

int ThunderbirdSettings::adaptAutoResizeResolution(int index, const QString &configStrList)
{
    switch (index) {
    case 0:
        return 240;
    case 1:
        return 320;
    case 2:
        return 512;
    case 3:
        return 640;
    case 4:
        return 800;
    case 5:
        return 1024;
    case 6:
        return 1280;
    case 7:
        return 2048;
    case 8:
        return 1024;
    case 9: { //custom case
        if (mHashConfig.contains(configStrList)) {
            const QString res =  mHashConfig.value(configStrList).toString();
            const QStringList lst = res.split(QLatin1Char(';'));
            int val = lst.last().toInt();
            return val;
        }
    }
    default:
        return -1;
    }
}

void ThunderbirdSettings::readTagSettings()
{
    QVector<tagStruct> v;
    v.reserve(mHashTag.size());
    foreach (const tagStruct &tag, mHashTag) {
        v.push_back(tag);
    }

    ImportWizardUtil::addAkonadiTag(v);
}

void ThunderbirdSettings::readLdapSettings()
{
    //qCDebug(IMPORTWIZARD_LOG)<<" mLdapAccountList:"<<mLdapAccountList;
    Q_FOREACH (const QString &ldapAccountName, mLdapAccountList) {
        ldapStruct ldap;
        const QString ldapDescription = QStringLiteral("%1.description").arg(ldapAccountName);
        if (mHashConfig.contains(ldapDescription)) {
            ldap.description = mHashConfig.value(ldapDescription).toString();
        }
        const QString ldapAuthDn = QStringLiteral("%1.auth.dn").arg(ldapAccountName);
        if (mHashConfig.contains(ldapAuthDn)) {
            ldap.dn = mHashConfig.value(ldapAuthDn).toString();
        }
        const QString ldapAuthSaslMech = QStringLiteral("%1.auth.saslmech").arg(ldapAccountName);
        if (mHashConfig.contains(ldapAuthSaslMech)) {
            ldap.saslMech = mHashConfig.value(ldapAuthSaslMech).toString();
        }
        const QString ldapFilename = QStringLiteral("%1.filename").arg(ldapAccountName);
        if (mHashConfig.contains(ldapFilename)) {
            ldap.fileName = mHashConfig.value(ldapFilename).toString();
        }
        const QString ldapMaxHits = QStringLiteral("%1.maxHits").arg(ldapAccountName);
        if (mHashConfig.contains(ldapMaxHits)) {
            ldap.fileName = mHashConfig.value(ldapMaxHits).toInt();
        }
        const QString ldapUri = QStringLiteral("%1.uri").arg(ldapAccountName);
        if (mHashConfig.contains(ldapUri)) {
            ldap.ldapUrl = QUrl(mHashConfig.value(ldapUri).toString());
            ldap.port = ldap.ldapUrl.port();

            if (ldap.ldapUrl.scheme() == QLatin1String("ldaps")) {
                ldap.useSSL = true;
            } else if (ldap.ldapUrl.scheme() == QLatin1String("ldap")) {
                ldap.useSSL = false;
            } else {
                qCDebug(IMPORTWIZARD_LOG) << " Security not implemented :" << ldap.ldapUrl.scheme();
            }
        }
        ImportWizardUtil::mergeLdap(ldap);
    }
}

void ThunderbirdSettings::readGlobalSettings()
{
    const QString markMessageReadStr = QStringLiteral("mailnews.mark_message_read.delay");
    if (mHashConfig.contains(markMessageReadStr)) {
        const bool markMessageRead = mHashConfig.value(markMessageReadStr).toBool();
        addKmailConfig(QStringLiteral("Behaviour"), QStringLiteral("DelayedMarkAsRead"), markMessageRead);
    } else {
        //Default value
        addKmailConfig(QStringLiteral("Behaviour"), QStringLiteral("DelayedMarkAsRead"), true);
    }
    const QString markMessageReadIntervalStr = QStringLiteral("mailnews.mark_message_read.delay.interval");
    if (mHashConfig.contains(markMessageReadIntervalStr)) {
        bool found = false;
        const int markMessageReadInterval = mHashConfig.value(markMessageReadIntervalStr).toInt(&found);
        if (found) {
            addKmailConfig(QStringLiteral("Behaviour"), QStringLiteral("DelayedMarkTime"), markMessageReadInterval);
        }
    } else {
        //Default 5 seconds
        addKmailConfig(QStringLiteral("Behaviour"), QStringLiteral("DelayedMarkTime"), 5);
    }

    const QString mailComposeAttachmentReminderStr = QStringLiteral("mail.compose.attachment_reminder");
    if (mHashConfig.contains(mailComposeAttachmentReminderStr)) {
        const bool mailComposeAttachmentReminder = mHashConfig.value(mailComposeAttachmentReminderStr).toBool();
        addKmailConfig(QStringLiteral("Composer"), QStringLiteral("showForgottenAttachmentWarning"), mailComposeAttachmentReminder);
    } else {
        addKmailConfig(QStringLiteral("Composer"), QStringLiteral("showForgottenAttachmentWarning"), true);
    }

    const QString mailComposeAttachmentReminderKeywordsStr = QStringLiteral("mail.compose.attachment_reminder_keywords");
    if (mHashConfig.contains(mailComposeAttachmentReminderKeywordsStr)) {
        const QString mailComposeAttachmentReminderKeywords = mHashConfig.value(mailComposeAttachmentReminderKeywordsStr).toString();
        addKmailConfig(QStringLiteral("Composer"), QStringLiteral("attachment-keywords"), mailComposeAttachmentReminderKeywords);
    } //not default value keep kmail use one default value

    const QString mailComposeAutosaveStr = QStringLiteral("mail.compose.autosave");
    if (mHashConfig.contains(mailComposeAutosaveStr)) {
        const bool mailComposeAutosave = mHashConfig.value(mailComposeAutosaveStr).toBool();
        if (mailComposeAutosave) {
            const QString mailComposeAutosaveintervalStr = QStringLiteral("mail.compose.autosaveinterval");
            if (mHashConfig.contains(mailComposeAutosaveintervalStr)) {
                bool found = false;
                const int mailComposeAutosaveinterval = mHashConfig.value(mailComposeAutosaveintervalStr).toInt(&found);
                if (found) {
                    addKmailConfig(QStringLiteral("Composer"), QStringLiteral("autosave"), mailComposeAutosaveinterval);
                } else {
                    addKmailConfig(QStringLiteral("Composer"), QStringLiteral("autosave"), 5);
                }
            } else {
                //Default value
                addKmailConfig(QStringLiteral("Composer"), QStringLiteral("autosave"), 5);
            }
        } else {
            //Don't autosave
            addKmailConfig(QStringLiteral("Composer"), QStringLiteral("autosave"), 0);
        }
    }

    const QString mailSpellCheckInlineStr = QStringLiteral("mail.spellcheck.inline");
    if (mHashConfig.contains(mailSpellCheckInlineStr)) {
        const bool mailSpellCheckInline = mHashConfig.value(mailSpellCheckInlineStr).toBool();
        addKmailConfig(QStringLiteral("Spelling"), QStringLiteral("backgroundCheckerEnabled"), mailSpellCheckInline);
    } else {
        addKmailConfig(QStringLiteral("Spelling"), QStringLiteral("backgroundCheckerEnabled"), false);
    }

    const QString mailAlertShowSubjectStr = QStringLiteral("mail.biff.alert.show_subject");
    if (mHashConfig.contains(mailAlertShowSubjectStr)) {
        const bool mailAlertShowSubject = mHashConfig.value(mailAlertShowSubjectStr).toBool();
        addNewMailNotifier(QStringLiteral("General"), QStringLiteral("showSubject"), mailAlertShowSubject);
    } else {
        //Default value in thunderbird
        addNewMailNotifier(QStringLiteral("General"), QStringLiteral("showSubject"), true);
    }

    const QString mailAlertShowPreviewStr = QStringLiteral("mail.biff.alert.show_preview");
    //TODO add show preview
    if (mHashConfig.contains(mailAlertShowPreviewStr)) {
        const bool mailAlertShowPreview = mHashConfig.value(mailAlertShowPreviewStr).toBool();
        //addNewMailNotifier(QStringLiteral("General"),QStringLiteral("showSubject"), mailAlertShowPreview);
    } else {
        //Default value in thunderbird
        //addNewMailNotifier(QStringLiteral("General"),QStringLiteral("showSubject"), true);
    }

    const QString mailAlertShowSenderStr = QStringLiteral("mail.biff.alert.show_sender");
    if (mHashConfig.contains(mailAlertShowSenderStr)) {
        const bool mailAlertShowSender = mHashConfig.value(mailAlertShowSenderStr).toBool();
        addNewMailNotifier(QStringLiteral("General"), QStringLiteral("showFrom"), mailAlertShowSender);
    } else {
        //Default value in thunderbird
        addNewMailNotifier(QStringLiteral("General"), QStringLiteral("showFrom"), true);
    }

    const QString mailSpellCheckBeforeSendStr = QStringLiteral("mail.SpellCheckBeforeSend");
    if (mHashConfig.contains(mailSpellCheckBeforeSendStr)) {
        const bool mailSpellCheckBeforeSend = mHashConfig.value(mailSpellCheckBeforeSendStr).toBool();
        addKmailConfig(QStringLiteral("Composer"), QStringLiteral("check-spelling-before-send"), mailSpellCheckBeforeSend);
    } else {
        addKmailConfig(QStringLiteral("Composer"), QStringLiteral("check-spelling-before-send"), false);
    }

    const QString mailSpellCheckLanguageStr = QStringLiteral("spellchecker.dictionary");
    if (mHashConfig.contains(mailSpellCheckLanguageStr)) {
        const QString mailSpellCheckLanguage = mHashConfig.value(mailSpellCheckLanguageStr).toString();
        addKmailConfig(QStringLiteral("Spelling"), QStringLiteral("defaultLanguage"), mailSpellCheckLanguage);
        //TODO create map to convert thunderbird name to aspell name
    }

    const QString mailPhishingDetectionStr = QStringLiteral("mail.phishing.detection.enabled");
    if (mHashConfig.contains(mailPhishingDetectionStr)) {
        const bool mailPhishingDetectionEnabled = mHashConfig.value(mailPhishingDetectionStr).toBool();
        addKmailConfig(QStringLiteral("Reader"), QStringLiteral("ScamDetectionEnabled"), mailPhishingDetectionEnabled);
    } else { //Default
        addKmailConfig(QStringLiteral("Reader"), QStringLiteral("ScamDetectionEnabled"), true);
    }

    const QString mailDisplayGlyphStr = QStringLiteral("mail.display_glyph");
    if (mHashConfig.contains(mailDisplayGlyphStr)) {
        const bool mailDisplayGlyphEnabled = mHashConfig.value(mailDisplayGlyphStr).toBool();
        addKmailConfig(QStringLiteral("Reader"), QStringLiteral("ShowEmoticons"), mailDisplayGlyphEnabled);
    } else { //Default
        addKmailConfig(QStringLiteral("Reader"), QStringLiteral("ShowEmoticons"), true);
    }

}

void ThunderbirdSettings::addAuth(QMap<QString, QVariant> &settings, const QString &argument, const QString &accountName)
{
    bool found = false;
    if (mHashConfig.contains(accountName + QStringLiteral(".authMethod"))) {
        const int authMethod = mHashConfig.value(accountName + QStringLiteral(".authMethod")).toInt(&found);
        if (found) {
            switch (authMethod) {
            case 0:
                break;
            case 4: //Encrypted password ???
                settings.insert(argument, MailTransport::Transport::EnumAuthenticationType::LOGIN);   //????
                qCDebug(IMPORTWIZARD_LOG) << " authmethod == encrypt password";
                break;
            case 5: //GSSAPI
                settings.insert(argument, MailTransport::Transport::EnumAuthenticationType::GSSAPI);
                break;
            case 6: //NTLM
                settings.insert(argument, MailTransport::Transport::EnumAuthenticationType::NTLM);
                break;
            case 7: //TLS
                qCDebug(IMPORTWIZARD_LOG) << " authmethod method == TLS"; //????
                break;
            default:
                qCDebug(IMPORTWIZARD_LOG) << " ThunderbirdSettings::addAuth unknown :" << authMethod;
                break;
            }
        }
    }
}

void ThunderbirdSettings::readAccount()
{
    Q_FOREACH (const QString &account, mAccountList) {
        const QString serverName = mHashConfig.value(QStringLiteral("mail.account.%1").arg(account) + QStringLiteral(".server")).toString();
        const QString accountName = QStringLiteral("mail.server.%1").arg(serverName);
        const QString host = mHashConfig.value(accountName + QStringLiteral(".hostname")).toString();
        const QString userName = mHashConfig.value(accountName + QStringLiteral(".userName")).toString();
        const QString name = mHashConfig.value(accountName + QStringLiteral(".name")).toString();

        const QString type = mHashConfig.value(accountName + QStringLiteral(".type")).toString();
        //TODO use it ?
        const QString directory = mHashConfig.value(accountName + QStringLiteral(".directory")).toString();

        const QString loginAtStartupStr = accountName + QStringLiteral(".login_at_startup");
        bool loginAtStartup = true; //Default for thunderbird;
        if (mHashConfig.contains(loginAtStartupStr)) {
            loginAtStartup = mHashConfig.value(loginAtStartupStr).toBool();
        }
        bool found = false;
        if (type == QLatin1String("imap")) {
            QMap<QString, QVariant> settings;
            settings.insert(QStringLiteral("ImapServer"), host);
            settings.insert(QStringLiteral("UserName"), userName);
            const int port = mHashConfig.value(accountName + QStringLiteral(".port")).toInt(&found);
            if (found) {
                settings.insert(QStringLiteral("ImapPort"), port);
            }
            addAuth(settings, QStringLiteral("Authentication"), accountName);
            const QString offline = accountName + QStringLiteral(".offline_download");
            if (mHashConfig.contains(offline)) {
                const bool offlineStatus = mHashConfig.value(offline).toBool();
                if (offlineStatus) {
                    settings.insert(QStringLiteral("DisconnectedModeEnabled"), offlineStatus);
                }
            } else {
                //default value == true
                settings.insert(QStringLiteral("DisconnectedModeEnabled"), true);
            }

            found = false;
            const int socketType = mHashConfig.value(accountName + QStringLiteral(".socketType")).toInt(&found);
            if (found) {
                switch (socketType) {
                case 0:
                    //None
                    settings.insert(QStringLiteral("Safety"), QStringLiteral("None"));
                    break;
                case 2:
                    //STARTTLS
                    settings.insert(QStringLiteral("Safety"), QStringLiteral("STARTTLS"));
                    break;
                case 3:
                    //SSL/TLS
                    settings.insert(QStringLiteral("Safety"), QStringLiteral("SSL"));
                    break;
                default:
                    qCDebug(IMPORTWIZARD_LOG) << " socketType " << socketType;
                }
            }
            const QString checkNewMailStr = accountName + QStringLiteral(".check_new_mail");
            if (mHashConfig.contains(checkNewMailStr)) {
                const bool checkNewMail = mHashConfig.value(checkNewMailStr).toBool();
                settings.insert(QStringLiteral("IntervalCheckEnabled"), checkNewMail);
            }

            const QString checkTimeStr = accountName + QStringLiteral(".check_time");
            if (mHashConfig.contains(checkTimeStr)) {
                found = false;
                const int checkTime = mHashConfig.value(checkTimeStr).toInt(&found);
                if (found) {
                    settings.insert(QStringLiteral("IntervalCheckTime"), checkTime);
                }
            } else {
                //Default value from thunderbird
                settings.insert(QStringLiteral("IntervalCheckTime"), 10);
            }
            const QString trashFolderStr = accountName + QStringLiteral(".trash_folder_name");
            if (mHashConfig.contains(trashFolderStr)) {
                settings.insert(QStringLiteral("TrashCollection"), MailCommon::Util::convertFolderPathToCollectionId(mHashConfig.value(trashFolderStr).toString()));
            }

            const QString agentIdentifyName = AbstractBase::createResource(QStringLiteral("akonadi_imap_resource"), name, settings);
            addCheckMailOnStartup(agentIdentifyName, loginAtStartup);
            //Not find a method to disable it in thunderbird
            addToManualCheck(agentIdentifyName, true);
        } else if (type == QLatin1String("pop3")) {
            QMap<QString, QVariant> settings;
            settings.insert(QStringLiteral("Host"), host);
            settings.insert(QStringLiteral("Login"), userName);

            const bool leaveOnServer = mHashConfig.value(accountName + QStringLiteral(".leave_on_server")).toBool();
            if (leaveOnServer) {
                settings.insert(QStringLiteral("LeaveOnServer"), leaveOnServer);
            }

            found = false;
            const int numberDayToLeave = mHashConfig.value(accountName + QStringLiteral(".num_days_to_leave_on_server")).toInt(&found);
            if (found) {
                settings.insert(QStringLiteral("LeaveOnServerDays"), numberDayToLeave);
            }

            found = false;
            const int port = mHashConfig.value(accountName + QStringLiteral(".port")).toInt(&found);
            if (found) {
                settings.insert(QStringLiteral("Port"), port);
            }

            found = false;
            const int socketType = mHashConfig.value(accountName + QStringLiteral(".socketType")).toInt(&found);
            if (found) {
                switch (socketType) {
                case 0:
                    //None
                    //nothing
                    break;
                case 2:
                    //STARTTLS
                    settings.insert(QStringLiteral("UseTLS"), true);
                    break;
                case 3:
                    //SSL/TLS
                    settings.insert(QStringLiteral("UseSSL"), true);
                    break;
                default:
                    qCDebug(IMPORTWIZARD_LOG) << " socketType " << socketType;
                }
            }
            addAuth(settings, QStringLiteral("AuthenticationMethod"), accountName);
            const QString checkNewMailStr = accountName + QStringLiteral(".check_new_mail");
            if (mHashConfig.contains(checkNewMailStr)) {
                const bool checkNewMail = mHashConfig.value(checkNewMailStr).toBool();
                settings.insert(QStringLiteral("IntervalCheckEnabled"), checkNewMail);
            }
            const QString checkTimeStr = accountName + QStringLiteral(".check_time");
            if (mHashConfig.contains(checkTimeStr)) {
                found = false;
                const int checkTime = mHashConfig.value(checkTimeStr).toInt(&found);
                if (found) {
                    settings.insert(QStringLiteral("IntervalCheckInterval"), checkTime);
                }
            } else {
                //Default value from thunderbird
                settings.insert(QStringLiteral("IntervalCheckInterval"), 10);
            }

            const QString agentIdentifyName = AbstractBase::createResource(QStringLiteral("akonadi_pop3_resource"), name, settings);
            addCheckMailOnStartup(agentIdentifyName, loginAtStartup);
            //Not find a method to disable it in thunderbird
            addToManualCheck(agentIdentifyName, true);
        } else if (type == QLatin1String("none")) {
            //FIXME look at if we can implement it
            qCDebug(IMPORTWIZARD_LOG) << " account type none!";
        } else if (type == QLatin1String("movemail")) {
            qCDebug(IMPORTWIZARD_LOG) << " movemail accound found and not implemented in importthunderbird";
            //TODO
        } else if (type == QLatin1String("rss")) {
            //TODO when akregator2 will merge in kdepim
            qCDebug(IMPORTWIZARD_LOG) << " rss resource needs to be implemented";
            continue;
        } else if (type == QLatin1String("nntp")) {
            //TODO add config directly to knode
            //TODO when knode will merge in kdepim
            qCDebug(IMPORTWIZARD_LOG) << " nntp resource need to be implemented";
            continue;
        } else {
            qCDebug(IMPORTWIZARD_LOG) << " type unknown : " << type;
            continue;
        }

        const QString identityConfig = QStringLiteral("mail.account.%1").arg(account) + QStringLiteral(".identities");
        if (mHashConfig.contains(identityConfig)) {
            const QStringList idList = mHashConfig.value(identityConfig).toString().split(QLatin1Char(','));
            Q_FOREACH (const QString &id, idList) {
                readIdentity(id);
            }
        }
    }
}

void ThunderbirdSettings::readTransport()
{
    const QString mailSmtpServer = mHashConfig.value(QStringLiteral("mail.smtpservers")).toString();
    if (mailSmtpServer.isEmpty()) {
        return;
    }
    QStringList smtpList = mailSmtpServer.split(QLatin1Char(','));
    QString defaultSmtp = mHashConfig.value(QStringLiteral("mail.smtp.defaultserver")).toString();
    if (smtpList.count() == 1 && defaultSmtp.isEmpty()) {
        //Be sure to define default smtp
        defaultSmtp = smtpList.at(0);
    }

    Q_FOREACH (const QString &smtp, smtpList) {
        const QString smtpName = QStringLiteral("mail.smtpserver.%1").arg(smtp);
        MailTransport::Transport *mt = createTransport();
        const QString name = mHashConfig.value(smtpName + QStringLiteral(".description")).toString();
        mt->setName(name);
        const QString hostName = mHashConfig.value(smtpName + QStringLiteral(".hostname")).toString();
        mt->setHost(hostName);

        const int port = mHashConfig.value(smtpName + QStringLiteral(".port")).toInt();
        if (port > 0) {
            mt->setPort(port);
        }

        const int authMethod = mHashConfig.value(smtpName + QStringLiteral(".authMethod")).toInt();
        switch (authMethod) {
        case 0:
            break;
        case 1: //No authentication
            mt->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::PLAIN); //????
            break;
        case 3: //Unencrypted password
            mt->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::CLEAR); //???
            break;
        case 4: //crypted password
            mt->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::LOGIN); //???
            break;
        case 5: //GSSAPI
            mt->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::GSSAPI);
            break;
        case 6: //NTLM
            mt->setAuthenticationType(MailTransport::Transport::EnumAuthenticationType::NTLM);
            break;
        default:
            qCDebug(IMPORTWIZARD_LOG) << " authMethod unknown :" << authMethod;
        }

        const int trySsl = mHashConfig.value(smtpName + QStringLiteral(".try_ssl")).toInt();
        switch (trySsl) {
        case 0:
            mt->setEncryption(MailTransport::Transport::EnumEncryption::None);
            break;
        case 2:
            mt->setEncryption(MailTransport::Transport::EnumEncryption::TLS);
            break;
        case 3:
            mt->setEncryption(MailTransport::Transport::EnumEncryption::SSL);
            break;
        default:
            qCDebug(IMPORTWIZARD_LOG) << " trySsl unknown :" << trySsl;
        }

        const QString userName = mHashConfig.value(smtpName + QStringLiteral(".username")).toString();
        if (!userName.isEmpty()) {
            mt->setUserName(userName);
            if (authMethod > 1) {
                mt->setRequiresAuthentication(true);
            }
        }

        storeTransport(mt, (smtp == defaultSmtp));
        mHashSmtp.insert(smtp, QString::number(mt->id()));
    }
}

QString convertThunderbirdPath(const QString &path)
{
    QString newPath;
    QUrl url(path);
    newPath = url.path();
    newPath.remove(0, 1);
    return MailCommon::Util::convertFolderPathToCollectionStr(newPath);
}

void ThunderbirdSettings::readIdentity(const QString &account)
{
    const QString identity = QStringLiteral("mail.identity.%1").arg(account);
    QString fullName = mHashConfig.value(identity + QStringLiteral(".fullName")).toString();
    KIdentityManagement::Identity *newIdentity = createIdentity(fullName);

    const QString smtpServer = mHashConfig.value(identity + QStringLiteral(".smtpServer")).toString();
    if (!smtpServer.isEmpty() && mHashSmtp.contains(smtpServer)) {
        newIdentity->setTransport(mHashSmtp.value(smtpServer));
    }

    const QString userEmail = mHashConfig.value(identity + QStringLiteral(".useremail")).toString();
    newIdentity->setPrimaryEmailAddress(userEmail);

    newIdentity->setFullName(fullName);
    newIdentity->setIdentityName(fullName);

    const QString organization = mHashConfig.value(identity + QStringLiteral(".organization")).toString();
    newIdentity->setOrganization(organization);

    bool doBcc = mHashConfig.value(identity + QStringLiteral(".doBcc")).toBool();
    if (doBcc) {
        const QString bcc = mHashConfig.value(identity + QStringLiteral(".doBccList")).toString();
        newIdentity->setBcc(bcc);
    }

    bool doCc = mHashConfig.value(identity + QStringLiteral(".doCc")).toBool();
    if (doCc) {
        const QString cc = mHashConfig.value(identity + QStringLiteral(".doCcList")).toString();
        newIdentity->setCc(cc);
    }

    const QString replyTo = mHashConfig.value(identity + QStringLiteral(".reply_to")).toString();
    newIdentity->setReplyToAddr(replyTo);

    KIdentityManagement::Signature signature;
    const bool signatureHtml = mHashConfig.value(identity + QStringLiteral(".htmlSigFormat")).toBool();
    if (signatureHtml) {
        signature.setInlinedHtml(true);
    }

    const bool attachSignature = mHashConfig.value(identity + QStringLiteral(".attach_signature")).toBool();
    if (attachSignature) {
        const QString fileSignature = mHashConfig.value(identity + QStringLiteral(".sig_file")).toString();
        signature.setType(KIdentityManagement::Signature::FromFile);
        signature.setUrl(fileSignature, false);
    } else {
        const QString textSignature = mHashConfig.value(identity + QStringLiteral(".htmlSigText")).toString();
        signature.setType(KIdentityManagement::Signature::Inlined);
        signature.setText(textSignature);
    }

    if (mHashConfig.contains(identity + QStringLiteral(".drafts_folder_picker_mode"))) {
        const int useSpecificDraftFolder = mHashConfig.value(identity + QStringLiteral(".drafts_folder_picker_mode")).toInt();
        if (useSpecificDraftFolder == 1) {
            const QString draftFolder = convertThunderbirdPath(mHashConfig.value(identity + QStringLiteral(".draft_folder")).toString());
            newIdentity->setDrafts(draftFolder);
        }
    }

    if (mHashConfig.contains(identity + QStringLiteral(".fcc"))) {
        const bool fccEnabled = mHashConfig.value(identity + QStringLiteral(".fcc")).toBool();
        newIdentity->setDisabledFcc(!fccEnabled);
    }

    //fcc_reply_follows_parent not implemented in kmail
    //fcc_folder_picker_mode is just a flag for thunderbird. Not necessary during import.
    //if ( mHashConfig.contains( identity + QStringLiteral( ".fcc_folder_picker_mode" ) ) )
    {
        if (mHashConfig.contains(identity + QStringLiteral(".fcc_folder"))) {
            const QString fccFolder = convertThunderbirdPath(mHashConfig.value(identity + QStringLiteral(".fcc_folder")).toString());
            newIdentity->setFcc(fccFolder);
        }
    }

    //if ( mHashConfig.contains( identity + QStringLiteral( ".tmpl_folder_picker_mode" ) ) )
    {
        if (mHashConfig.contains(identity + QStringLiteral(".stationery_folder"))) {
            const QString templateFolder = convertThunderbirdPath(mHashConfig.value(identity + QStringLiteral(".stationery_folder")).toString());
            newIdentity->setTemplates(templateFolder);
        }
    }

    const QString attachVcardStr(identity + QStringLiteral(".attach_vcard"));
    if (mHashConfig.contains(attachVcardStr)) {
        const bool attachVcard = mHashConfig.value(attachVcardStr).toBool();
        newIdentity->setAttachVcard(attachVcard);
    }
    const QString attachVcardContentStr(identity + QStringLiteral(".escapedVCard"));
    if (mHashConfig.contains(attachVcardContentStr)) {
        const QString str = mHashConfig.value(attachVcardContentStr).toString();
        QByteArray vcard = QByteArray::fromPercentEncoding(str.toLocal8Bit());
        KContacts::VCardConverter converter;
        KContacts::Addressee addr = converter.parseVCard(vcard);

        const QString filename = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') + newIdentity->identityName() + QStringLiteral(".vcf");
        QFileInfo fileInfo(filename);
        QDir().mkpath(fileInfo.absolutePath());
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            const QByteArray data = converter.exportVCard(addr, KContacts::VCardConverter::v3_0);
            file.write(data);
            file.flush();
            file.close();
            newIdentity->setVCardFile(filename);
        }
    }
    const QString signMailStr(identity + QStringLiteral(".sign_mail"));
    if (mHashConfig.contains(signMailStr)) {
        const bool signMail = mHashConfig.value(signMailStr).toBool();
        newIdentity->setPgpAutoSign(signMail);
    }

    const QString composeHtmlStr(identity + QStringLiteral(".compose_html"));
    //TODO: implement it in kmail

    newIdentity->setSignature(signature);

    storeIdentity(newIdentity);
}

void ThunderbirdSettings::insertIntoMap(const QString &line)
{
    QString newLine = line;
    newLine.remove(QStringLiteral("user_pref(\""));
    newLine.remove(QStringLiteral(");"));
    const int pos = newLine.indexOf(QLatin1Char(','));
    QString key = newLine.left(pos);
    key.remove(key.length() - 1, 1);
    QString valueStr = newLine.right(newLine.length() - pos - 2);
    if (valueStr.at(0) == QLatin1Char('"')) {
        valueStr.remove(0, 1);
        const int pos(valueStr.length() - 1);
        if (valueStr.at(pos) == QLatin1Char('"')) {
            valueStr.remove(pos, 1);
        }
        //Store as String
        mHashConfig.insert(key, valueStr);
    } else {
        if (valueStr == QLatin1String("true")) {
            mHashConfig.insert(key, true);
        } else if (valueStr == QLatin1String("false")) {
            mHashConfig.insert(key, false);
        } else {
            //Store as integer
            const int value = valueStr.toInt();
            mHashConfig.insert(key, value);
        }
    }
    if (key.contains(QStringLiteral("ldap_")) && key.endsWith(QStringLiteral(".description"))) {
        QString ldapAccountName = key;
        mLdapAccountList.append(ldapAccountName.remove(QStringLiteral(".description")));
    }
    if (key.contains(QStringLiteral("mailnews.tags.")) &&
            (key.endsWith(QStringLiteral(".color")) || key.endsWith(QStringLiteral(".tag")))) {
        QString name = key;
        name.remove(QStringLiteral("mailnews.tags."));
        name.remove(QStringLiteral(".color"));
        name.remove(QStringLiteral(".tag"));
        tagStruct tag;
        if (mHashTag.contains(name)) {
            tag = mHashTag.value(name);
            mHashTag.remove(name);
        }
        if (key.endsWith(QStringLiteral(".color"))) {
            tag.color = QColor(mHashConfig.value(key).toString());
        } else {
            tag.name = mHashConfig.value(key).toString();
        }
        mHashTag.insert(name, tag);
        qCDebug(IMPORTWIZARD_LOG) << " tag :" << name << " tag.name" << tag.name << " color :" << tag.color;
    }
}

void ThunderbirdSettings::addNewMailNotifier(const QString &group, const QString &key, bool value)
{
    KConfig config(QStringLiteral("akonadi_newmailnotifier_agentrc"));
    KConfigGroup grp = config.group(group);
    grp.writeEntry(key, value);
}
