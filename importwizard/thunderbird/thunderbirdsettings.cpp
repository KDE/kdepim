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

#include "thunderbirdsettings.h"
#include <mailtransport/transportmanager.h>
#include "mailcommon/util/mailutil.h"
#include "importwizardutil.h"

#include <kpimidentities/identity.h>
#include <kpimidentities/signature.h>
#include <KStandardDirs>

#include <KABC/VCardConverter>
#include <QTextStream>
#include <QStringList>
#include <QFile>
#include <KDebug>

ThunderbirdSettings::ThunderbirdSettings( const QString& filename, ImportWizard *parent )
    :AbstractSettings( parent )
{
    QFile file(filename);
    if ( !file.open( QIODevice::ReadOnly ) ) {
        kDebug()<<" We can't open file"<<filename;
        return;
    }
    QTextStream stream(&file);
    while ( !stream.atEnd() ) {
        const QString line = stream.readLine();
        if (line.startsWith(QLatin1String("user_pref"))) {
            if (line.contains(QLatin1String("mail.smtpserver")) ||
                    line.contains(QLatin1String("mail.server.") ) ||
                    line.contains(QLatin1String("mail.identity.")) ||
                    line.contains(QLatin1String("mail.account.")) ||
                    line.contains(QLatin1String("mail.accountmanager.")) ||
                    line.contains(QLatin1String("mailnews."))||
                    line.contains(QLatin1String("mail.compose."))||
                    line.contains(QLatin1String("mail.spellcheck")) ||
                    line.contains(QLatin1String("mail.SpellCheckBeforeSend")) ||
                    line.contains(QLatin1String("spellchecker.dictionary")) ||
                    line.contains(QLatin1String("ldap_")) ||
                    line.contains(QLatin1String("mail.biff.")) ||
                    line.contains(QLatin1String("mailnews.tags.")) ||
                    line.contains(QLatin1String("extensions.AutoResizeImage.")) ||
                    line.contains(QLatin1String("mail.phishing."))) {
                insertIntoMap( line );
            }
        } else {
            kDebug()<<" unstored line :"<<line;
        }
    }
    const QString mailAccountPreference = mHashConfig.value( QLatin1String( "mail.accountmanager.accounts" ) ).toString();
    if ( mailAccountPreference.isEmpty() )
        return;
    mAccountList = mailAccountPreference.split( QLatin1Char( ',' ) );
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
    const QString filterPatternEnabledStr = QLatin1String("extensions.AutoResizeImage.filterPatterns");
    if (mHashConfig.contains(filterPatternEnabledStr)) {
        const int filterPatternType = mHashConfig.value(filterPatternEnabledStr).toInt();
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("filter-source-type"), filterPatternType);
    }
    const QString filterPatternListStr = QLatin1String("extensions.AutoResizeImage.filteringPatternsList");
    if (mHashConfig.contains(filterPatternListStr)) {
        const QString filterPatternList = mHashConfig.value(filterPatternListStr).toString();
        //TODO decode it.
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("filter-source-pattern"), filterPatternList );
    }

    const QString enlargeImagesStr = QLatin1String("extensions.AutoResizeImage.enlargeImages");
    if (mHashConfig.contains(enlargeImagesStr)) {
        const bool enlargeImages = mHashConfig.value(enlargeImagesStr).toBool();
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("enlarge-image-to-minimum"), enlargeImages );
    }

    const QString maxResolutionXStr = QLatin1String("extensions.AutoResizeImage.maxResolutionX");
    if (mHashConfig.contains(maxResolutionXStr)) {
        const int val = mHashConfig.value(maxResolutionXStr).toInt();
        int adaptedValue = adaptAutoResizeResolution(val,QLatin1String("extensions.AutoResizeImage.maxResolutionXList"));
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("maximum-width"), adaptedValue );
    }
    const QString maxResolutionYStr = QLatin1String("extensions.AutoResizeImage.maxResolutionY");
    if (mHashConfig.contains(maxResolutionYStr)) {
        const int val = mHashConfig.value(maxResolutionYStr).toInt();
        int adaptedValue = adaptAutoResizeResolution(val,QLatin1String("extensions.AutoResizeImage.maxResolutionYList"));
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("maximum-height"), adaptedValue );
    }
    const QString minResolutionXStr = QLatin1String("extensions.AutoResizeImage.minResolutionX");
    if (mHashConfig.contains(minResolutionXStr)) {
        const int val = mHashConfig.value(minResolutionXStr).toInt();
        int adaptedValue = adaptAutoResizeResolution(val,QLatin1String("extensions.AutoResizeImage.minResolutionXList"));
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("minimum-width"), adaptedValue );
    }
    const QString minResolutionYStr = QLatin1String("extensions.AutoResizeImage.minResolutionY");
    if (mHashConfig.contains(minResolutionYStr)) {
        const int val = mHashConfig.value(minResolutionYStr).toInt();
        int adaptedValue = adaptAutoResizeResolution(val,QLatin1String("extensions.AutoResizeImage.minResolutionYList"));
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("minimum-height"), adaptedValue );
    }

    //Default is true.
    const QString reduceImageStr(QLatin1String("extensions.AutoResizeImage.reduceImages"));
    if (mHashConfig.contains(reduceImageStr)) {
        const bool reduce = mHashConfig.value(reduceImageStr).toBool();
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("reduce-image-to-maximum"), reduce );
    } else {
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("reduce-image-to-maximum"), false );
    }

    const QString filterMinimumStr(QLatin1String("extensions.AutoResizeImage.filterMinimumSize"));
    if (mHashConfig.contains(filterMinimumStr)) {
        const bool filterMinimum = mHashConfig.value(filterMinimumStr).toBool();
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("skip-image-lower-size-enabled"), filterMinimum );
    }
    const QString skipMinimumSizeStr(QLatin1String("extensions.AutoResizeImage.minimumSize"));
    if (mHashConfig.contains(skipMinimumSizeStr)) {
        const int skipMinimumSize = mHashConfig.value(skipMinimumSizeStr).toInt();
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("skip-image-lower-size"), skipMinimumSize );
    }
    const QString confirmBeforeResizingStr(QLatin1String("extensions.AutoResizeImage.confirmResizing"));
    if (mHashConfig.contains(confirmBeforeResizingStr)) {
        const bool confirmBeforeResizing = mHashConfig.value(confirmBeforeResizingStr).toBool();
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("ask-before-resizing"), confirmBeforeResizing );
    }
    //extensions.AutoResizeImage.convertImages : not implemented in kmail

    const QString conversionFormatStr(QLatin1String("extensions.AutoResizeImage.conversionFormat"));
    if (mHashConfig.contains(conversionFormatStr)) {
        QString conversionFormat = mHashConfig.value(conversionFormatStr).toString();
        if (conversionFormat == QLatin1String("png")) {
            conversionFormat = QLatin1String("PNG");
        } else {
            conversionFormat = QLatin1String("JPG");
        }
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("write-format"), conversionFormat );
    }

    const QString filterRecipientsStr(QLatin1String("extensions.AutoResizeImage.filterRecipients"));
    if (mHashConfig.contains(filterRecipientsStr)) {
        const int filterRecipients = mHashConfig.value(filterRecipientsStr).toInt();
        switch(filterRecipients) {
        case 0:
            addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("FilterRecipientType"), QLatin1String("NoFilter") );
            break;
        case 1:
            addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("FilterRecipientType"), QLatin1String("ResizeEachEmailsContainsPattern") );
            break;
        case 2:
            addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("FilterRecipientType"), QLatin1String("ResizeOneEmailContainsPattern") );
            break;
        case 3:
            addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("FilterRecipientType"), QLatin1String("DontResizeEachEmailsContainsPattern") );
            break;
        case 4:
            addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("FilterRecipientType"), QLatin1String("DontResizeOneEmailContainsPattern") );
            break;
        default:
            qDebug()<<" unknow FilterRecipientType: "<<filterRecipients;
            break;
        }
    }

    const QString filteringRecipientsPatternsWhiteListStr(QLatin1String("extensions.AutoResizeImage.filteringRecipientsPatternsWhiteList"));
    if (mHashConfig.contains(filteringRecipientsPatternsWhiteListStr)) {
        const QString filteringRecipientsPatternsWhiteList = mHashConfig.value(filteringRecipientsPatternsWhiteListStr).toString();
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("resize-emails-pattern"), filteringRecipientsPatternsWhiteList );
    }

    const QString filteringRecipientsPatternsBlackListStr(QLatin1String("extensions.AutoResizeImage.filteringRecipientsPatternsBlackList"));
    if (mHashConfig.contains(filteringRecipientsPatternsBlackListStr)) {
        const QString filteringRecipientsPatternsBlackList = mHashConfig.value(filteringRecipientsPatternsBlackListStr).toString();
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("do-not-resize-emails-pattern"), filteringRecipientsPatternsBlackList );
    }

    const QString filteringRenamingPatternStr(QLatin1String("extensions.AutoResizeImage.renamingPattern"));
    if (mHashConfig.contains(filteringRenamingPatternStr)) {
        QString filteringRenamingPattern = mHashConfig.value(filteringRenamingPatternStr).toString();
        filteringRenamingPattern.replace(QLatin1String("%3Fn"), QLatin1String("%n"));
        filteringRenamingPattern.replace(QLatin1String("%3Ft"), QLatin1String("%t"));
        filteringRenamingPattern.replace(QLatin1String("%3Fd"), QLatin1String("%d"));
        filteringRenamingPattern.replace(QLatin1String("%3Fe"), QLatin1String("%e"));
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("rename-resized-images-pattern"), filteringRenamingPattern);
    }

    const QString filteringRenamingImageStr(QLatin1String("extensions.AutoResizeImage.renameResizedImages"));
    if (mHashConfig.contains(filteringRenamingImageStr)) {
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("rename-resized-images"), true);
    }

    const QString filteringImageFormatsStr(QLatin1String("extensions.AutoResizeImage.imageFormats"));
    if (mHashConfig.contains(filteringImageFormatsStr)) {
        const QString filteringImageFormats = mHashConfig.value(filteringImageFormatsStr).toString();
        //convert it.
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("resize-image-with-formats-type"), filteringImageFormats);
    }

    const QString filteringImageFormatsEnabledStr(QLatin1String("extensions.AutoResizeImage.filterFormats"));
    if (mHashConfig.contains(filteringImageFormatsEnabledStr)) {
        const bool filteringImageFormatsEnabled = mHashConfig.value(filteringImageFormatsEnabledStr).toBool();
        addKmailConfig(QLatin1String("AutoResizeImage"), QLatin1String("resize-image-with-formats"), filteringImageFormatsEnabled);
    }
}

int ThunderbirdSettings::adaptAutoResizeResolution(int index, const QString& configStrList)
{
    switch(index) {
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
    case 9: //custom case
    {
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
    ImportWizardUtil::addNepomukTag(mHashTag.values());
}

void ThunderbirdSettings::readLdapSettings()
{
    //qDebug()<<" mLdapAccountList:"<<mLdapAccountList;
    Q_FOREACH (const QString& ldapAccountName, mLdapAccountList) {
        ldapStruct ldap;
        const QString ldapDescription = QString::fromLatin1("%1.description").arg(ldapAccountName);
        if (mHashConfig.contains(ldapDescription)) {
            ldap.description = mHashConfig.value(ldapDescription).toString();
        }
        const QString ldapAuthDn = QString::fromLatin1("%1.auth.dn").arg(ldapAccountName);
        if (mHashConfig.contains(ldapAuthDn)) {
            ldap.dn = mHashConfig.value(ldapAuthDn).toString();
        }
        const QString ldapAuthSaslMech = QString::fromLatin1("%1.auth.saslmech").arg(ldapAccountName);
        if (mHashConfig.contains(ldapAuthSaslMech)) {
            ldap.saslMech = mHashConfig.value(ldapAuthSaslMech).toString();
        }
        const QString ldapFilename = QString::fromLatin1("%1.filename").arg(ldapAccountName);
        if (mHashConfig.contains(ldapFilename)) {
            ldap.fileName = mHashConfig.value(ldapFilename).toString();
        }
        const QString ldapMaxHits = QString::fromLatin1("%1.maxHits").arg(ldapAccountName);
        if (mHashConfig.contains(ldapMaxHits)) {
            ldap.fileName = mHashConfig.value(ldapMaxHits).toInt();
        }
        const QString ldapUri = QString::fromLatin1("%1.uri").arg(ldapAccountName);
        if (mHashConfig.contains(ldapUri)) {
            ldap.ldapUrl = KUrl(mHashConfig.value(ldapUri).toString());
            ldap.port = ldap.ldapUrl.port();

            if (ldap.ldapUrl.scheme() == QLatin1String("ldaps")) {
                ldap.useSSL = true;
            } else if (ldap.ldapUrl.scheme() == QLatin1String("ldap")) {
                ldap.useSSL = false;
            } else {
                qDebug()<<" Security not implemented :"<<ldap.ldapUrl.scheme();
            }
        }
        ImportWizardUtil::mergeLdap(ldap);
    }
}

void ThunderbirdSettings::readGlobalSettings()
{
    const QString markMessageReadStr = QLatin1String("mailnews.mark_message_read.delay");
    if (mHashConfig.contains(markMessageReadStr)) {
        const bool markMessageRead = mHashConfig.value(markMessageReadStr).toBool();
        addKmailConfig(QLatin1String("Behaviour"), QLatin1String("DelayedMarkAsRead"), markMessageRead);
    } else {
        //Default value
        addKmailConfig(QLatin1String("Behaviour"), QLatin1String("DelayedMarkAsRead"), true);
    }
    const QString markMessageReadIntervalStr = QLatin1String("mailnews.mark_message_read.delay.interval");
    if (mHashConfig.contains(markMessageReadIntervalStr)) {
        bool found = false;
        const int markMessageReadInterval = mHashConfig.value(markMessageReadIntervalStr).toInt(&found);
        if (found) {
            addKmailConfig(QLatin1String("Behaviour"), QLatin1String("DelayedMarkTime"), markMessageReadInterval);
        }
    } else {
        //Default 5 seconds
        addKmailConfig(QLatin1String("Behaviour"), QLatin1String("DelayedMarkTime"), 5);
    }

    const QString mailComposeAttachmentReminderStr = QLatin1String("mail.compose.attachment_reminder");
    if (mHashConfig.contains(mailComposeAttachmentReminderStr)) {
        const bool mailComposeAttachmentReminder = mHashConfig.value(mailComposeAttachmentReminderStr).toBool();
        addKmailConfig(QLatin1String("Composer"), QLatin1String("showForgottenAttachmentWarning"), mailComposeAttachmentReminder);
    } else {
        addKmailConfig(QLatin1String("Composer"), QLatin1String("showForgottenAttachmentWarning"), true);
    }

    const QString mailComposeAttachmentReminderKeywordsStr = QLatin1String("mail.compose.attachment_reminder_keywords");
    if (mHashConfig.contains(mailComposeAttachmentReminderKeywordsStr)) {
        const QString mailComposeAttachmentReminderKeywords = mHashConfig.value(mailComposeAttachmentReminderKeywordsStr).toString();
        addKmailConfig(QLatin1String("Composer"), QLatin1String("attachment-keywords"), mailComposeAttachmentReminderKeywords);
    } //not default value keep kmail use one default value

    const QString mailComposeAutosaveStr = QLatin1String("mail.compose.autosave");
    if (mHashConfig.contains(mailComposeAutosaveStr)) {
        const bool mailComposeAutosave = mHashConfig.value(mailComposeAutosaveStr).toBool();
        if (mailComposeAutosave) {
            const QString mailComposeAutosaveintervalStr = QLatin1String("mail.compose.autosaveinterval");
            if (mHashConfig.contains(mailComposeAutosaveintervalStr)) {
                bool found = false;
                const int mailComposeAutosaveinterval = mHashConfig.value(mailComposeAutosaveintervalStr).toInt(&found);
                if (found) {
                    addKmailConfig(QLatin1String("Composer"), QLatin1String("autosave"), mailComposeAutosaveinterval);
                } else {
                    addKmailConfig(QLatin1String("Composer"), QLatin1String("autosave"), 5);
                }
            } else {
                //Default value
                addKmailConfig(QLatin1String("Composer"), QLatin1String("autosave"), 5);
            }
        } else {
            //Don't autosave
            addKmailConfig(QLatin1String("Composer"), QLatin1String("autosave"), 0);
        }
    }

    const QString mailSpellCheckInlineStr = QLatin1String("mail.spellcheck.inline");
    if (mHashConfig.contains(mailSpellCheckInlineStr)) {
        const bool mailSpellCheckInline = mHashConfig.value(mailSpellCheckInlineStr).toBool();
        addKmailConfig(QLatin1String("Spelling"),QLatin1String("backgroundCheckerEnabled"),mailSpellCheckInline);
    } else {
        addKmailConfig(QLatin1String("Spelling"),QLatin1String("backgroundCheckerEnabled"),false);
    }
    const QString mailPlaySoundStr = QLatin1String("mail.biff.play_sound");
    if (mHashConfig.contains(mailPlaySoundStr)) {
        const bool mailPlaySound = mHashConfig.value(mailPlaySoundStr).toBool();
        addKmailConfig(QLatin1String("General"),QLatin1String("beep-on-mail"), mailPlaySound);
    } else {
        //Default value in thunderbird
        addKmailConfig(QLatin1String("General"),QLatin1String("beep-on-mail"), true);
    }
    const QString mailSpellCheckBeforeSendStr = QLatin1String("mail.SpellCheckBeforeSend");
    if (mHashConfig.contains(mailSpellCheckBeforeSendStr)) {
        const bool mailSpellCheckBeforeSend = mHashConfig.value(mailSpellCheckBeforeSendStr).toBool();
        addKmailConfig(QLatin1String("Composer"),QLatin1String("check-spelling-before-send"), mailSpellCheckBeforeSend);
    } else {
        addKmailConfig(QLatin1String("Composer"),QLatin1String("check-spelling-before-send"), false);
    }

    const QString mailSpellCheckLanguageStr = QLatin1String("spellchecker.dictionary");
    if (mHashConfig.contains(mailSpellCheckLanguageStr)) {
        const QString mailSpellCheckLanguage = mHashConfig.value(mailSpellCheckLanguageStr).toString();
        addKmailConfig(QLatin1String("Spelling"), QLatin1String("defaultLanguage"),mailSpellCheckLanguage);
        //TODO create map to convert thunderbird name to aspell name
    }

    const QString mailPhishingDetectionStr = QLatin1String("mail.phishing.detection.enabled");
    if (mHashConfig.contains(mailPhishingDetectionStr)) {
        const bool mailPhishingDetectionEnabled = mHashConfig.value(mailPhishingDetectionStr).toBool();
        addKmailConfig(QLatin1String("Reader"), QLatin1String("ScamDetectionEnabled"), mailPhishingDetectionEnabled);
    } else { //Default
        addKmailConfig(QLatin1String("Reader"), QLatin1String("ScamDetectionEnabled"), true);
    }

}

void ThunderbirdSettings::addAuth(QMap<QString, QVariant>& settings, const QString & argument, const QString &accountName )
{
    bool found = false;
    if ( mHashConfig.contains( accountName + QLatin1String( ".authMethod" ) ) ) {
        const int authMethod = mHashConfig.value( accountName + QLatin1String( ".authMethod" ) ).toInt(&found);
        if ( found ) {
            switch( authMethod ) {
            case 0:
                break;
            case 4: //Encrypted password ???
                settings.insert( argument, MailTransport::Transport::EnumAuthenticationType::LOGIN ); //????
                kDebug()<<" authmethod == encrypt password";
                break;
            case 5: //GSSAPI
                settings.insert( argument, MailTransport::Transport::EnumAuthenticationType::GSSAPI );
                break;
            case 6: //NTLM
                settings.insert( argument, MailTransport::Transport::EnumAuthenticationType::NTLM );
                break;
            case 7: //TLS
                kDebug()<<" authmethod method == TLS"; //????
                break;
            default:
                kDebug()<<" ThunderbirdSettings::addAuth unknown :"<<authMethod;
                break;
            }
        }
    }
}

void ThunderbirdSettings::readAccount()
{
    Q_FOREACH ( const QString&account, mAccountList )
    {
        const QString serverName = mHashConfig.value( QString::fromLatin1( "mail.account.%1" ).arg( account ) + QLatin1String( ".server" ) ).toString();
        const QString accountName = QString::fromLatin1( "mail.server.%1" ).arg( serverName );
        const QString host = mHashConfig.value( accountName + QLatin1String( ".hostname" ) ).toString();
        const QString userName = mHashConfig.value( accountName + QLatin1String( ".userName" ) ).toString();
        const QString name = mHashConfig.value( accountName + QLatin1String( ".name" ) ).toString();

        const QString type = mHashConfig.value( accountName + QLatin1String( ".type" ) ).toString();
        //TODO use it ?
        const QString directory = mHashConfig.value( accountName + QLatin1String( ".directory" ) ).toString();

        const QString loginAtStartupStr = accountName + QLatin1String( ".login_at_startup" );
        bool loginAtStartup = true; //Default for thunderbird;
        if ( mHashConfig.contains( loginAtStartupStr ) ) {
            loginAtStartup = mHashConfig.value( loginAtStartupStr ).toBool();
        }
        bool found = false;
        if ( type == QLatin1String("imap")) {
            QMap<QString, QVariant> settings;
            settings.insert(QLatin1String("ImapServer"),host);
            settings.insert(QLatin1String("UserName"),userName);
            const int port = mHashConfig.value( accountName + QLatin1String( ".port" ) ).toInt( &found);
            if ( found ) {
                settings.insert( QLatin1String( "ImapPort" ), port );
            }
            addAuth( settings, QLatin1String( "Authentication" ), accountName );
            const QString offline = accountName + QLatin1String( ".offline_download" );
            if ( mHashConfig.contains( offline ) ) {
                const bool offlineStatus = mHashConfig.value( offline ).toBool();
                if ( offlineStatus ) {
                    settings.insert( QLatin1String( "DisconnectedModeEnabled" ), offlineStatus );
                }
            } else {
                //default value == true
                settings.insert( QLatin1String( "DisconnectedModeEnabled" ), true );
            }

            found = false;
            const int socketType = mHashConfig.value( accountName + QLatin1String( ".socketType" ) ).toInt( &found);
            if (found) {
                switch(socketType) {
                case 0:
                    //None
                    settings.insert( QLatin1String( "Safety" ), QLatin1String("None") );
                    break;
                case 2:
                    //STARTTLS
                    settings.insert( QLatin1String( "Safety" ), QLatin1String("STARTTLS") );
                    break;
                case 3:
                    //SSL/TLS
                    settings.insert( QLatin1String( "Safety" ), QLatin1String("SSL") );
                    break;
                default:
                    kDebug()<<" socketType "<<socketType;
                }
            }
            const QString checkNewMailStr = accountName + QLatin1String( ".check_new_mail" );
            if (mHashConfig.contains(checkNewMailStr)) {
                const bool checkNewMail = mHashConfig.value(checkNewMailStr).toBool();
                settings.insert(QLatin1String("IntervalCheckEnabled"), checkNewMail);
            }

            const QString checkTimeStr = accountName + QLatin1String( ".check_time" );
            if (mHashConfig.contains(checkTimeStr)) {
                found = false;
                const int checkTime = mHashConfig.value( checkTimeStr ).toInt( &found);
                if (found) {
                    settings.insert(QLatin1String("IntervalCheckTime"),checkTime);
                }
            } else {
                //Default value from thunderbird
                settings.insert(QLatin1String("IntervalCheckTime"), 10 );
            }
            const QString trashFolderStr = accountName + QLatin1String( ".trash_folder_name" );
            if (mHashConfig.contains(trashFolderStr)) {
                settings.insert(QLatin1String("TrashCollection"),MailCommon::Util::convertFolderPathToCollectionId(mHashConfig.value(trashFolderStr).toString()));
            }

            const QString agentIdentifyName = AbstractBase::createResource( QLatin1String("akonadi_imap_resource"), name,settings );
            addCheckMailOnStartup(agentIdentifyName,loginAtStartup);
            //Not find a method to disable it in thunderbird
            addToManualCheck(agentIdentifyName,true);
        } else if ( type == QLatin1String("pop3")) {
            QMap<QString, QVariant> settings;
            settings.insert( QLatin1String( "Host" ), host );
            settings.insert( QLatin1String( "Login" ), userName );

            const bool leaveOnServer = mHashConfig.value( accountName + QLatin1String( ".leave_on_server")).toBool();
            if (leaveOnServer) {
                settings.insert(QLatin1String("LeaveOnServer"),leaveOnServer);
            }

            found = false;
            const int numberDayToLeave = mHashConfig.value( accountName + QLatin1String( ".num_days_to_leave_on_server")).toInt(&found);
            if ( found ) {
                settings.insert(QLatin1String("LeaveOnServerDays"),numberDayToLeave);
            }

            found = false;
            const int port = mHashConfig.value( accountName + QLatin1String( ".port" ) ).toInt( &found);
            if ( found ) {
                settings.insert( QLatin1String( "Port" ), port );
            }

            found = false;
            const int socketType = mHashConfig.value( accountName + QLatin1String( ".socketType" ) ).toInt( &found);
            if (found) {
                switch(socketType) {
                case 0:
                    //None
                    //nothing
                    break;
                case 2:
                    //STARTTLS
                    settings.insert( QLatin1String( "UseTLS" ), true );
                    break;
                case 3:
                    //SSL/TLS
                    settings.insert( QLatin1String( "UseSSL" ), true );
                    break;
                default:
                    kDebug()<<" socketType "<<socketType;
                }
            }
            addAuth( settings, QLatin1String( "AuthenticationMethod" ),accountName );
            const QString checkNewMailStr = accountName + QLatin1String( ".check_new_mail" );
            if (mHashConfig.contains(checkNewMailStr)) {
                const bool checkNewMail = mHashConfig.value(checkNewMailStr).toBool();
                settings.insert(QLatin1String("IntervalCheckEnabled"), checkNewMail);
            }
            const QString checkTimeStr = accountName + QLatin1String( ".check_time" );
            if (mHashConfig.contains(checkTimeStr)) {
                found = false;
                const int checkTime = mHashConfig.value( checkTimeStr ).toInt( &found);
                if (found) {
                    settings.insert(QLatin1String("IntervalCheckInterval"),checkTime);
                }
            } else {
                //Default value from thunderbird
                settings.insert(QLatin1String("IntervalCheckInterval"), 10 );
            }

            const QString agentIdentifyName = AbstractBase::createResource( QLatin1String("akonadi_pop3_resource"), name, settings );
            addCheckMailOnStartup(agentIdentifyName,loginAtStartup);
            //Not find a method to disable it in thunderbird
            addToManualCheck(agentIdentifyName,true);
        } else if ( type == QLatin1String( "none" ) ) {
            //FIXME look at if we can implement it
            kDebug()<<" account type none!";
        } else if (type == QLatin1String("movemail")) {
            kDebug()<<" movemail accound found and not implemented in importthunderbird";
            //TODO
        } else if (type == QLatin1String("rss")) {
            //TODO when akregator2 will merge in kdepim
            kDebug()<<" rss resource needs to be implemented";
            continue;
        } else if (type == QLatin1String("nntp")) {
            //TODO add config directly to knode
            //TODO when knode will merge in kdepim
            kDebug()<<" nntp resource need to be implemented";
            continue;
        } else {
            kDebug()<<" type unknown : "<<type;
            continue;
        }

        const QString identityConfig = QString::fromLatin1( "mail.account.%1" ).arg( account ) + QLatin1String( ".identities" );
        if ( mHashConfig.contains( identityConfig ) )
        {
            const QStringList idList = mHashConfig.value(identityConfig).toString().split(QLatin1Char(','));
            Q_FOREACH (const QString& id, idList) {
                readIdentity( id );
            }
        }
    }
}

void ThunderbirdSettings::readTransport()
{
    const QString mailSmtpServer = mHashConfig.value( QLatin1String( "mail.smtpservers" ) ).toString();
    if ( mailSmtpServer.isEmpty() )
        return;
    QStringList smtpList = mailSmtpServer.split( QLatin1Char( ',' ) );
    QString defaultSmtp = mHashConfig.value( QLatin1String( "mail.smtp.defaultserver" ) ).toString();
    if (smtpList.count() == 1 && defaultSmtp.isEmpty())
    {
        //Be sure to define default smtp
        defaultSmtp = smtpList.at(0);
    }

    Q_FOREACH ( const QString &smtp, smtpList )
    {
        const QString smtpName = QString::fromLatin1( "mail.smtpserver.%1" ).arg( smtp );
        MailTransport::Transport *mt = createTransport();
        const QString name = mHashConfig.value( smtpName + QLatin1String( ".description" ) ).toString();
        mt->setName(name);
        const QString hostName = mHashConfig.value( smtpName + QLatin1String( ".hostname" ) ).toString();
        mt->setHost( hostName );

        const int port = mHashConfig.value( smtpName + QLatin1String( ".port" ) ).toInt();
        if ( port > 0 )
            mt->setPort( port );

        const int authMethod = mHashConfig.value( smtpName + QLatin1String( ".authMethod" ) ).toInt();
        switch(authMethod) {
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
            kDebug()<<" authMethod unknown :"<<authMethod;
        }

        const int trySsl = mHashConfig.value( smtpName + QLatin1String( ".try_ssl" ) ).toInt();
        switch(trySsl) {
        case 0:
            mt->setEncryption( MailTransport::Transport::EnumEncryption::None );
            break;
        case 2:
            mt->setEncryption( MailTransport::Transport::EnumEncryption::TLS );
            break;
        case 3:
            mt->setEncryption( MailTransport::Transport::EnumEncryption::SSL );
            break;
        default:
            kDebug()<<" trySsl unknown :"<<trySsl;
        }

        const QString userName = mHashConfig.value( smtpName + QLatin1String( ".username" ) ).toString();
        if ( !userName.isEmpty() ) {
            mt->setUserName( userName );
            if (authMethod > 1) {
                mt->setRequiresAuthentication( true );
            }
        }

        storeTransport( mt, ( smtp == defaultSmtp ) );
        mHashSmtp.insert( smtp, QString::number( mt->id() ) );
    }
}

QString convertThunderbirdPath(const QString& path)
{
    QString newPath;
    KUrl url(path);
    newPath = url.path();
    newPath.remove(0,1);
    return MailCommon::Util::convertFolderPathToCollectionStr(newPath);
}

void ThunderbirdSettings::readIdentity( const QString& account )
{
    const QString identity = QString::fromLatin1( "mail.identity.%1" ).arg( account );
    QString fullName = mHashConfig.value( identity + QLatin1String( ".fullName" ) ).toString();
    KPIMIdentities::Identity* newIdentity = createIdentity(fullName);


    const QString smtpServer = mHashConfig.value( identity + QLatin1String( ".smtpServer" ) ).toString();
    if (!smtpServer.isEmpty() && mHashSmtp.contains(smtpServer))
    {
        newIdentity->setTransport(mHashSmtp.value(smtpServer));
    }

    const QString userEmail = mHashConfig.value( identity + QLatin1String( ".useremail" ) ).toString();
    newIdentity->setPrimaryEmailAddress(userEmail);

    newIdentity->setFullName( fullName );
    newIdentity->setIdentityName( fullName );

    const QString organization = mHashConfig.value(identity + QLatin1String(".organization")).toString();
    newIdentity->setOrganization(organization);

    bool doBcc = mHashConfig.value(identity + QLatin1String(".doBcc")).toBool();
    if (doBcc) {
        const QString bcc = mHashConfig.value(identity + QLatin1String(".doBccList")).toString();
        newIdentity->setBcc( bcc );
    }

    bool doCc = mHashConfig.value(identity + QLatin1String(".doCc")).toBool();
    if (doCc) {
        const QString cc = mHashConfig.value(identity + QLatin1String(".doCcList")).toString();
        newIdentity->setCc( cc );
    }

    const QString replyTo = mHashConfig.value(identity + QLatin1String( ".reply_to")).toString();
    newIdentity->setReplyToAddr( replyTo );

    KPIMIdentities::Signature signature;
    const bool signatureHtml = mHashConfig.value(identity + QLatin1String( ".htmlSigFormat" )).toBool();
    if (signatureHtml) {
        signature.setInlinedHtml( true );
    }

    const bool attachSignature = mHashConfig.value(identity + QLatin1String( ".attach_signature" )).toBool();
    if ( attachSignature ) {
        const QString fileSignature = mHashConfig.value(identity + QLatin1String( ".sig_file")).toString();
        signature.setType( KPIMIdentities::Signature::FromFile );
        signature.setUrl( fileSignature,false );
    }
    else {
        const QString textSignature = mHashConfig.value(identity + QLatin1String( ".htmlSigText" ) ).toString();
        signature.setType( KPIMIdentities::Signature::Inlined );
        signature.setText( textSignature );
    }


    if ( mHashConfig.contains( identity + QLatin1String( ".drafts_folder_picker_mode" ) ) )
    {
        const int useSpecificDraftFolder = mHashConfig.value(  identity + QLatin1String( ".drafts_folder_picker_mode" ) ).toInt();
        if ( useSpecificDraftFolder == 1 )
        {
            const QString draftFolder = convertThunderbirdPath( mHashConfig.value( identity + QLatin1String( ".draft_folder" ) ).toString() );
            newIdentity->setDrafts( draftFolder );
        }
    }

    if ( mHashConfig.contains( identity + QLatin1String( ".fcc" ) ) ) {
        const bool fccEnabled = mHashConfig.value(identity + QLatin1String( ".fcc" )).toBool();
        newIdentity->setDisabledFcc( !fccEnabled );
    }

    //fcc_reply_follows_parent not implemented in kmail
    //fcc_folder_picker_mode is just a flag for thunderbird. Not necessary during import.
    //if ( mHashConfig.contains( identity + QLatin1String( ".fcc_folder_picker_mode" ) ) )
    {
        if (mHashConfig.contains( identity + QLatin1String( ".fcc_folder" ) )) {
            const QString fccFolder = convertThunderbirdPath( mHashConfig.value( identity + QLatin1String( ".fcc_folder" ) ).toString() );
            newIdentity->setFcc( fccFolder );
        }
    }

    //if ( mHashConfig.contains( identity + QLatin1String( ".tmpl_folder_picker_mode" ) ) )
    {
        if (mHashConfig.contains( identity + QLatin1String( ".stationery_folder" ) )) {
            const QString templateFolder = convertThunderbirdPath( mHashConfig.value( identity + QLatin1String( ".stationery_folder" ) ).toString() );
            newIdentity->setTemplates( templateFolder );
        }
    }


    const QString attachVcardStr( identity + QLatin1String( ".attach_vcard" ) );
    if ( mHashConfig.contains( attachVcardStr ) ) {
        const bool attachVcard = mHashConfig.value( attachVcardStr ).toBool();
        newIdentity->setAttachVcard(attachVcard);
    }
    const QString attachVcardContentStr( identity + QLatin1String( ".escapedVCard" ) );
    if ( mHashConfig.contains( attachVcardContentStr ) ) {
        const QString str = mHashConfig.value( attachVcardContentStr ).toString();
        QByteArray vcard = QByteArray::fromPercentEncoding ( str.toLocal8Bit() );
        KABC::VCardConverter converter;
        KABC::Addressee addr = converter.parseVCard( vcard );

        const QString filename = KStandardDirs::locateLocal("appdata",newIdentity->identityName() + QLatin1String(".vcf"));
        QFile file(filename);
        if ( file.open( QIODevice::WriteOnly |QIODevice::Text ) ) {
            const QByteArray data = converter.exportVCard( addr, KABC::VCardConverter::v3_0 );
            file.write( data );
            file.flush();
            file.close();
            newIdentity->setVCardFile(filename);
        }

    }
    const QString composeHtmlStr( identity + QLatin1String( ".compose_html" ) );
    //TODO: implement it in kmail

    newIdentity->setSignature( signature );

    storeIdentity(newIdentity);
}

void ThunderbirdSettings::insertIntoMap( const QString& line )
{
    QString newLine = line;
    newLine.remove( QLatin1String( "user_pref(\"" ) );
    newLine.remove( QLatin1String( ");" ) );
    const int pos = newLine.indexOf( QLatin1Char( ',' ) );
    QString key = newLine.left( pos );
    key.remove( key.length() -1, 1 );
    QString valueStr = newLine.right( newLine.length() - pos -2);
    if ( valueStr.at( 0 ) == QLatin1Char( '"' ) ) {
        valueStr.remove( 0, 1 );
        const int pos(valueStr.length()-1);
        if ( valueStr.at( pos ) == QLatin1Char( '"' ) )
            valueStr.remove( pos, 1 );
        //Store as String
        mHashConfig.insert( key, valueStr );
    } else {
        if ( valueStr == QLatin1String( "true" ) ) {
            mHashConfig.insert( key, true );
        } else if ( valueStr == QLatin1String( "false" ) ) {
            mHashConfig.insert( key, false );
        } else {
            //Store as integer
            const int value = valueStr.toInt();
            mHashConfig.insert( key, value );
        }
    }
    if (key.contains(QLatin1String("ldap_")) && key.endsWith(QLatin1String(".description"))) {
        QString ldapAccountName = key;
        mLdapAccountList.append(ldapAccountName.remove(QLatin1String(".description")));
    }
    if (key.contains(QLatin1String("mailnews.tags.")) &&
            (key.endsWith(QLatin1String(".color")) || key.endsWith(QLatin1String(".tag")))) {
        QString name = key;
        name.remove(QLatin1String("mailnews.tags."));
        name.remove(QLatin1String(".color"));
        name.remove(QLatin1String(".tag"));
        tagStruct tag;
        if (mHashTag.contains(name)) {
            tag = mHashTag.value(name);
            mHashTag.remove(name);
        }
        if (key.endsWith(QLatin1String(".color"))) {
            tag.color = QColor(mHashConfig.value(key).toString());
        } else {
            tag.name = mHashConfig.value(key).toString();
        }
        mHashTag.insert(name,tag);
        kDebug()<<" tag :"<<name<<" tag.name"<<tag.name<<" color :"<<tag.color;
    }
}
