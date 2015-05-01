/*
 * Copyright (C) 2006 Dmitry Morozhnikov <dmiceman@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "templatesinsertcommand.h"

#include <QAction>
#include <KActionMenu>
#include "templateparser_debug.h"
#include <KLocalizedString>
#include <QMenu>

#include <QSignalMapper>

#undef I18N_NOOP
#define I18N_NOOP(t) 0, t
#undef I18N_NOOP2
#define I18N_NOOP2(c,t) c, t

using namespace TemplateParser;

struct InsertCommand {
    const char *context;
    const char *name;
    const TemplatesInsertCommand::Command command;

    QString getLocalizedDisplayName() const
    {
        return i18nc(context, name);
    }
};

static const InsertCommand originalCommands[] = {
    {
        I18N_NOOP("Quoted Message Text"),
        TemplatesInsertCommand::CQuote
    },

    {
        I18N_NOOP("Message Text as Is"),
        TemplatesInsertCommand::CText
    },

    {
        I18N_NOOP("Message Id"),
        TemplatesInsertCommand::COMsgId
    },

    {
        I18N_NOOP("Date"),
        TemplatesInsertCommand::CODate
    },

    {
        I18N_NOOP("Date in Short Format"),
        TemplatesInsertCommand::CODateShort
    },

    {
        I18N_NOOP("Date in C Locale"),
        TemplatesInsertCommand::CODateEn
    },

    {
        I18N_NOOP("Day of Week"),
        TemplatesInsertCommand::CODow
    },

    {
        I18N_NOOP("Time"),
        TemplatesInsertCommand::COTime
    },

    {
        I18N_NOOP("Time in Long Format"),
        TemplatesInsertCommand::COTimeLong
    },

    {
        I18N_NOOP("Time in C Locale"),
        TemplatesInsertCommand::COTimeLongEn
    },

    {
        I18N_NOOP("To Field Address"),
        TemplatesInsertCommand::COToAddr
    },

    {
        I18N_NOOP("To Field Name"),
        TemplatesInsertCommand::COToName
    },

    {
        I18N_NOOP("To Field First Name"),
        TemplatesInsertCommand::COToFName
    },

    {
        I18N_NOOP("To Field Last Name"),
        TemplatesInsertCommand::COToLName
    },

    {
        I18N_NOOP("CC Field Address"),
        TemplatesInsertCommand::COCCAddr
    },

    {
        I18N_NOOP("CC Field Name"),
        TemplatesInsertCommand::COCCName
    },

    {
        I18N_NOOP("CC Field First Name"),
        TemplatesInsertCommand::COCCFName
    },

    {
        I18N_NOOP("CC Field Last Name"),
        TemplatesInsertCommand::COCCLName
    },

    {
        I18N_NOOP("From Field Address"),
        TemplatesInsertCommand::COFromAddr
    },

    {
        I18N_NOOP("From Field Name"),
        TemplatesInsertCommand::COFromName
    },

    {
        I18N_NOOP("From Field First Name"),
        TemplatesInsertCommand::COFromFName
    },

    {
        I18N_NOOP("From Field Last Name"),
        TemplatesInsertCommand::COFromLName
    },

    {
        I18N_NOOP("Addresses of all recipients"),
        TemplatesInsertCommand::COAddresseesAddr
    },

    {
        I18N_NOOP2("Template value for subject of the message", "Subject"),
        TemplatesInsertCommand::COFullSubject
    },

    {
        I18N_NOOP("Quoted Headers"),
        TemplatesInsertCommand::CQHeaders
    },

    {
        I18N_NOOP("Headers as Is"),
        TemplatesInsertCommand::CHeaders
    },

    {
        I18N_NOOP("Header Content"),
        TemplatesInsertCommand::COHeader
    },

    {
        I18N_NOOP("Reply as Quoted Plain Text"),
        TemplatesInsertCommand::CQuotePlain
    },

    {
        I18N_NOOP("Reply as Quoted HTML Text"),
        TemplatesInsertCommand::CQuoteHtml
    }
};
static const int originalCommandsCount =
    sizeof(originalCommands) / sizeof(*originalCommands);

static const InsertCommand currentCommands[] = {
    {
        I18N_NOOP("Date"),
        TemplatesInsertCommand::CDate
    },

    {
        I18N_NOOP("Date in Short Format"),
        TemplatesInsertCommand::CDateShort
    },

    {
        I18N_NOOP("Date in C Locale"),
        TemplatesInsertCommand::CDateEn
    },

    {
        I18N_NOOP("Day of Week"),
        TemplatesInsertCommand::CDow
    },

    {
        I18N_NOOP("Time"),
        TemplatesInsertCommand::CTime
    },

    {
        I18N_NOOP("Time in Long Format"),
        TemplatesInsertCommand::CTimeLong
    },

    {
        I18N_NOOP("Time in C Locale"),
        TemplatesInsertCommand::CTimeLongEn
    },
    {
        I18N_NOOP("To Field Address"),
        TemplatesInsertCommand::CToAddr
    },

    {
        I18N_NOOP("To Field Name"),
        TemplatesInsertCommand::CToName
    },

    {
        I18N_NOOP("To Field First Name"),
        TemplatesInsertCommand::CToFName
    },

    {
        I18N_NOOP("To Field Last Name"),
        TemplatesInsertCommand::CToLName
    },

    {
        I18N_NOOP("CC Field Address"),
        TemplatesInsertCommand::CCCAddr
    },

    {
        I18N_NOOP("CC Field Name"),
        TemplatesInsertCommand::CCCName
    },

    {
        I18N_NOOP("CC Field First Name"),
        TemplatesInsertCommand::CCCFName
    },

    {
        I18N_NOOP("CC Field Last Name"),
        TemplatesInsertCommand::CCCLName
    },

    {
        I18N_NOOP("From Field Address"),
        TemplatesInsertCommand::CFromAddr
    },

    {
        I18N_NOOP("From field Name"),
        TemplatesInsertCommand::CFromName
    },

    {
        I18N_NOOP("From Field First Name"),
        TemplatesInsertCommand::CFromFName
    },

    {
        I18N_NOOP("From Field Last Name"),
        TemplatesInsertCommand::CFromLName
    },

    {
        I18N_NOOP2("Template subject command.", "Subject"),
        TemplatesInsertCommand::CFullSubject
    },

    {
        I18N_NOOP("Header Content"),
        TemplatesInsertCommand::CHeader
    }
};
static const int currentCommandsCount = sizeof(currentCommands) / sizeof(*currentCommands);

static const InsertCommand extCommands[] = {
    {
        I18N_NOOP("Pipe Original Message Body and Insert Result as Quoted Text"),
        TemplatesInsertCommand::CQuotePipe
    },

    {
        I18N_NOOP("Pipe Original Message Body and Insert Result as Is"),
        TemplatesInsertCommand::CTextPipe
    },

    {
        I18N_NOOP("Pipe Original Message with Headers and Insert Result as Is"),
        TemplatesInsertCommand::CMsgPipe
    },

    {
        I18N_NOOP("Pipe Current Message Body and Insert Result as Is"),
        TemplatesInsertCommand::CBodyPipe
    },

    {
        I18N_NOOP("Pipe Current Message Body and Replace with Result"),
        TemplatesInsertCommand::CClearPipe
    }
};

static const int extCommandsCount =
    sizeof(extCommands) / sizeof(*extCommands);

static const InsertCommand miscCommands[] = {
    {
        I18N_NOOP2("Inserts user signature, also known as footer, into message", "Signature"),
        TemplatesInsertCommand::CSignature
    },

    {
        I18N_NOOP("Insert File Content"),
        TemplatesInsertCommand::CInsert
    },

    {
        I18N_NOOP2("All characters, up to and including the next newline, "
        "are discarded without performing any macro expansion",
        "Discard to Next Line"),
        TemplatesInsertCommand::CDnl
    },

    {
        I18N_NOOP("Template Comment"),
        TemplatesInsertCommand::CRem
    },

    {
        I18N_NOOP("No Operation"),
        TemplatesInsertCommand::CNop
    },

    {
        I18N_NOOP("Clear Generated Message"),
        TemplatesInsertCommand::CClear
    },

    {
        I18N_NOOP("Turn Debug On"),
        TemplatesInsertCommand::CDebug
    },

    {
        I18N_NOOP("Turn Debug Off"),
        TemplatesInsertCommand::CDebugOff
    },

    {
        I18N_NOOP("Cursor position"),
        TemplatesInsertCommand::CCursor
    },

    {
        I18N_NOOP("Blank text"),
        TemplatesInsertCommand::CBlank
    },

    {
        I18N_NOOP("Dictionary Language"),
        TemplatesInsertCommand::CDictionaryLanguage
    },
    {
        I18N_NOOP("Language"),
        TemplatesInsertCommand::CLanguage
    },

};
static const int miscCommandsCount = sizeof(miscCommands) / sizeof(*miscCommands);

static void fillMenuFromActionMap(const QMap< QString, TemplatesInsertCommand::Command > &map,
                                  KActionMenu *menu, QSignalMapper *mapper)
{
    QMap< QString, TemplatesInsertCommand::Command >::const_iterator it = map.constBegin();
    QMap< QString, TemplatesInsertCommand::Command >::const_iterator end = map.constEnd();

    while (it != end) {
        QAction *action = new QAction(it.key(), menu);   //krazy:exclude=tipsandthis
        QObject::connect(action, SIGNAL(triggered(bool)), mapper, SLOT(map()));
        mapper->setMapping(action, it.value());
        menu->addAction(action);
        ++it;
    }
}

TemplatesInsertCommand::TemplatesInsertCommand(QWidget *parent, const QString &name)
    : QPushButton(parent)
{
    setObjectName(name);
    setText(i18n("&Insert Command"));

    KActionMenu *menu;
    QMap< QString, Command > commandMap;

    QSignalMapper *mapper = new QSignalMapper(this);
    connect(mapper, SIGNAL(mapped(int)),
            this, SLOT(slotMapped(int)));

    mMenu = new KActionMenu(i18n("Insert Command"), this);
    setToolTip(
        i18nc("@info:tooltip",
              "Select a command to insert into the template"));
    setWhatsThis(
        i18nc("@info:whatsthis",
              "Traverse this menu to find a command to insert into the current template "
              "being edited.  The command will be inserted at the cursor location, "
              "so you want to move your cursor to the desired insertion point first."));

    // ******************************************************
    menu = new KActionMenu(i18n("Original Message"), mMenu);
    mMenu->addAction(menu);

    // Map sorts commands
    for (int i = 0; i < originalCommandsCount; ++i) {
        commandMap.insert(originalCommands[i].getLocalizedDisplayName(), originalCommands[i].command);
    }

    fillMenuFromActionMap(commandMap, menu, mapper);
    commandMap.clear();

    // ******************************************************
    menu = new KActionMenu(i18n("Current Message"), mMenu);
    mMenu->addAction(menu);

    for (int i = 0; i < currentCommandsCount; ++i) {
        commandMap.insert(currentCommands[i].getLocalizedDisplayName(), currentCommands[i].command);
    }

    fillMenuFromActionMap(commandMap, menu, mapper);
    commandMap.clear();

    // ******************************************************
    menu = new KActionMenu(i18n("Process with External Programs"), mMenu);
    mMenu->addAction(menu);

    for (int i = 0; i < extCommandsCount; ++i) {
        commandMap.insert(extCommands[i].getLocalizedDisplayName(), extCommands[i].command);
    }

    fillMenuFromActionMap(commandMap, menu, mapper);
    commandMap.clear();

    // ******************************************************
    menu = new KActionMenu(i18nc("Miscellaneous template commands menu", "Miscellaneous"), mMenu);
    mMenu->addAction(menu);

    for (int i = 0; i < miscCommandsCount; ++i) {
        commandMap.insert(miscCommands[i].getLocalizedDisplayName(), miscCommands[i].command);
    }

    fillMenuFromActionMap(commandMap, menu, mapper);

    setMenu(mMenu->menu());
}

TemplatesInsertCommand::~TemplatesInsertCommand()
{
}

void TemplatesInsertCommand::slotMapped(int cmd)
{
    Q_EMIT insertCommand(static_cast<Command>(cmd));

    switch (cmd) {
    case TemplatesInsertCommand::CBlank:
        Q_EMIT insertCommand(QStringLiteral("%BLANK"));
        break;
    case TemplatesInsertCommand::CQuote:
        Q_EMIT insertCommand(QStringLiteral("%QUOTE"));
        break;
    case TemplatesInsertCommand::CText:
        Q_EMIT insertCommand(QStringLiteral("%TEXT"));
        break;
    case TemplatesInsertCommand::COMsgId:
        Q_EMIT insertCommand(QStringLiteral("%OMSGID"));
        break;
    case TemplatesInsertCommand::CODate:
        Q_EMIT insertCommand(QStringLiteral("%ODATE"));
        break;
    case TemplatesInsertCommand::CODateShort:
        Q_EMIT insertCommand(QStringLiteral("%ODATESHORT"));
        break;
    case TemplatesInsertCommand::CODateEn:
        Q_EMIT insertCommand(QStringLiteral("%ODATEEN"));
        break;
    case TemplatesInsertCommand::CODow:
        Q_EMIT insertCommand(QStringLiteral("%ODOW"));
        break;
    case TemplatesInsertCommand::COTime:
        Q_EMIT insertCommand(QStringLiteral("%OTIME"));
        break;
    case TemplatesInsertCommand::COTimeLong:
        Q_EMIT insertCommand(QStringLiteral("%OTIMELONG"));
        break;
    case TemplatesInsertCommand::COTimeLongEn:
        Q_EMIT insertCommand(QStringLiteral("%OTIMELONGEN"));
        break;
    case TemplatesInsertCommand::COToAddr:
        Q_EMIT insertCommand(QStringLiteral("%OTOADDR"));
        break;
    case TemplatesInsertCommand::COToName:
        Q_EMIT insertCommand(QStringLiteral("%OTONAME"));
        break;
    case TemplatesInsertCommand::COToFName:
        Q_EMIT insertCommand(QStringLiteral("%OTOFNAME"));
        break;
    case TemplatesInsertCommand::COToLName:
        Q_EMIT insertCommand(QStringLiteral("%OTOLNAME"));
        break;
    case TemplatesInsertCommand::COCCAddr:
        Q_EMIT insertCommand(QStringLiteral("%OCCADDR"));
        break;
    case TemplatesInsertCommand::COCCName:
        Q_EMIT insertCommand(QStringLiteral("%OCCNAME"));
        break;
    case TemplatesInsertCommand::COCCFName:
        Q_EMIT insertCommand(QStringLiteral("%OCCFNAME"));
        break;
    case TemplatesInsertCommand::COCCLName:
        Q_EMIT insertCommand(QStringLiteral("%OCCLNAME"));
        break;
    case TemplatesInsertCommand::COFromAddr:
        Q_EMIT insertCommand(QStringLiteral("%OFROMADDR"));
        break;
    case TemplatesInsertCommand::COFromName:
        Q_EMIT insertCommand(QStringLiteral("%OFROMNAME"));
        break;
    case TemplatesInsertCommand::COFromFName:
        Q_EMIT insertCommand(QStringLiteral("%OFROMFNAME"));
        break;
    case TemplatesInsertCommand::COFromLName:
        Q_EMIT insertCommand(QStringLiteral("%OFROMLNAME"));
        break;
    case TemplatesInsertCommand::COFullSubject:
        Q_EMIT insertCommand(QStringLiteral("%OFULLSUBJECT"));
        break;
    case TemplatesInsertCommand::CQHeaders:
        Q_EMIT insertCommand(QStringLiteral("%QHEADERS"));
        break;
    case TemplatesInsertCommand::CHeaders:
        Q_EMIT insertCommand(QStringLiteral("%HEADERS"));
        break;
    case TemplatesInsertCommand::COHeader:
        Q_EMIT insertCommand(QStringLiteral("%OHEADER=\"\""), -1);
        break;
    case TemplatesInsertCommand::CMsgId:
        Q_EMIT insertCommand(QStringLiteral("%MSGID"));
        break;
    case TemplatesInsertCommand::CDate:
        Q_EMIT insertCommand(QStringLiteral("%DATE"));
        break;
    case TemplatesInsertCommand::CDateShort:
        Q_EMIT insertCommand(QStringLiteral("%DATESHORT"));
        break;
    case TemplatesInsertCommand::CDateEn:
        Q_EMIT insertCommand(QStringLiteral("%DATEEN"));
        break;
    case TemplatesInsertCommand::CDow:
        Q_EMIT insertCommand(QStringLiteral("%DOW"));
        break;
    case TemplatesInsertCommand::CTime:
        Q_EMIT insertCommand(QStringLiteral("%TIME"));
        break;
    case TemplatesInsertCommand::CTimeLong:
        Q_EMIT insertCommand(QStringLiteral("%TIMELONG"));
        break;
    case TemplatesInsertCommand::CTimeLongEn:
        Q_EMIT insertCommand(QStringLiteral("%TIMELONGEN"));
        break;
    case TemplatesInsertCommand::COAddresseesAddr:
        Q_EMIT insertCommand(QStringLiteral("%OADDRESSEESADDR"));
        break;
    case TemplatesInsertCommand::CToAddr:
        Q_EMIT insertCommand(QStringLiteral("%TOADDR"));
        break;
    case TemplatesInsertCommand::CToName:
        Q_EMIT insertCommand(QStringLiteral("%TONAME"));
        break;
    case TemplatesInsertCommand::CToFName:
        Q_EMIT insertCommand(QStringLiteral("%TOFNAME"));
        break;
    case TemplatesInsertCommand::CToLName:
        Q_EMIT insertCommand(QStringLiteral("%TOLNAME"));
        break;
    case TemplatesInsertCommand::CCCAddr:
        Q_EMIT insertCommand(QStringLiteral("%CCADDR"));
        break;
    case TemplatesInsertCommand::CCCName:
        Q_EMIT insertCommand(QStringLiteral("%CCNAME"));
        break;
    case TemplatesInsertCommand::CCCFName:
        Q_EMIT insertCommand(QStringLiteral("%CCFNAME"));
        break;
    case TemplatesInsertCommand::CCCLName:
        Q_EMIT insertCommand(QStringLiteral("%CCLNAME"));
        break;
    case TemplatesInsertCommand::CFromAddr:
        Q_EMIT insertCommand(QStringLiteral("%FROMADDR"));
        break;
    case TemplatesInsertCommand::CFromName:
        Q_EMIT insertCommand(QStringLiteral("%FROMNAME"));
        break;
    case TemplatesInsertCommand::CFromFName:
        Q_EMIT insertCommand(QStringLiteral("%FROMFNAME"));
        break;
    case TemplatesInsertCommand::CFromLName:
        Q_EMIT insertCommand(QStringLiteral("%FROMLNAME"));
        break;
    case TemplatesInsertCommand::CFullSubject:
        Q_EMIT insertCommand(QStringLiteral("%FULLSUBJECT"));
        break;
    case TemplatesInsertCommand::CHeader:
        Q_EMIT insertCommand(QStringLiteral("%HEADER=\"\""), -1);
        break;
    case TemplatesInsertCommand::CSystem:
        Q_EMIT insertCommand(QStringLiteral("%SYSTEM=\"\""), -1);
        break;
    case TemplatesInsertCommand::CQuotePipe:
        Q_EMIT insertCommand(QStringLiteral("%QUOTEPIPE=\"\""), -1);
        break;
    case TemplatesInsertCommand::CTextPipe:
        Q_EMIT insertCommand(QStringLiteral("%TEXTPIPE=\"\""), -1);
        break;
    case TemplatesInsertCommand::CMsgPipe:
        Q_EMIT insertCommand(QStringLiteral("%MSGPIPE=\"\""), -1);
        break;
    case TemplatesInsertCommand::CBodyPipe:
        Q_EMIT insertCommand(QStringLiteral("%BODYPIPE=\"\""), -1);
        break;
    case TemplatesInsertCommand::CClearPipe:
        Q_EMIT insertCommand(QStringLiteral("%CLEARPIPE=\"\""), -1);
        break;
    case TemplatesInsertCommand::CCursor:
        Q_EMIT insertCommand(QStringLiteral("%CURSOR"));
        break;
    case TemplatesInsertCommand::CSignature:
        Q_EMIT insertCommand(QStringLiteral("%SIGNATURE"));
        break;
    case TemplatesInsertCommand::CInsert:
        Q_EMIT insertCommand(QStringLiteral("%INSERT=\"\""), -1);
        break;
    case TemplatesInsertCommand::CDnl:
        Q_EMIT insertCommand(QStringLiteral("%-"));
        break;
    case TemplatesInsertCommand::CRem:
        Q_EMIT insertCommand(QStringLiteral("%REM=\"\""), -1);
        break;
    case TemplatesInsertCommand::CNop:
        Q_EMIT insertCommand(QStringLiteral("%NOP"));
        break;
    case TemplatesInsertCommand::CClear:
        Q_EMIT insertCommand(QStringLiteral("%CLEAR"));
        break;
    case TemplatesInsertCommand::CDebug:
        Q_EMIT insertCommand(QStringLiteral("%DEBUG"));
        break;
    case TemplatesInsertCommand::CDebugOff:
        Q_EMIT insertCommand(QStringLiteral("%DEBUGOFF"));
        break;
    case TemplatesInsertCommand::CQuotePlain:
        Q_EMIT insertCommand(QStringLiteral("%FORCEDPLAIN"));
        break;
    case TemplatesInsertCommand::CQuoteHtml:
        Q_EMIT insertCommand(QStringLiteral("%FORCEDHTML"));
        break;
    case TemplatesInsertCommand::CDictionaryLanguage:
        Q_EMIT insertCommand(QStringLiteral("%DICTIONARYLANGUAGE=\"\""), -1);
        break;
    case TemplatesInsertCommand::CLanguage:
        Q_EMIT insertCommand(QStringLiteral("%LANGUAGE=\"\""), -1);
        break;
    default:
        qCDebug(TEMPLATEPARSER_LOG) << "Unknown template command index:" << cmd;
        break;
    }
}

