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

#include <KAction>
#include <KActionMenu>
#include <KDebug>
#include <KLocalizedString>
#include <KMenu>

#include <QSignalMapper>

#undef I18N_NOOP
#define I18N_NOOP(t) 0, t
#undef I18N_NOOP2
#define I18N_NOOP2(c,t) c, t

using namespace TemplateParser;

struct InsertCommand
{
    const char *context;
    const char *name;
    const TemplatesInsertCommand::Command command;

    QString getLocalizedDisplayName() const
    {
        return i18nc( context, name );
    }
};

static const InsertCommand originalCommands[] =
{
    { I18N_NOOP( "Quoted Message Text" ),
      TemplatesInsertCommand::CQuote
    },

    { I18N_NOOP( "Message Text as Is" ),
      TemplatesInsertCommand::CText
    },

    { I18N_NOOP( "Message Id" ),
      TemplatesInsertCommand::COMsgId
    },

    { I18N_NOOP( "Date" ),
      TemplatesInsertCommand::CODate
    },

    { I18N_NOOP( "Date in Short Format" ),
      TemplatesInsertCommand::CODateShort
    },

    { I18N_NOOP( "Date in C Locale" ),
      TemplatesInsertCommand::CODateEn
    },

    { I18N_NOOP( "Day of Week" ),
      TemplatesInsertCommand::CODow
    },

    { I18N_NOOP( "Time" ),
      TemplatesInsertCommand::COTime
    },

    { I18N_NOOP( "Time in Long Format" ),
      TemplatesInsertCommand::COTimeLong
    },

    { I18N_NOOP( "Time in C Locale" ),
      TemplatesInsertCommand::COTimeLongEn
    },

    { I18N_NOOP( "To Field Address" ),
      TemplatesInsertCommand::COToAddr
    },

    { I18N_NOOP( "To Field Name" ),
      TemplatesInsertCommand::COToName
    },

    { I18N_NOOP( "To Field First Name" ),
      TemplatesInsertCommand::COToFName
    },

    { I18N_NOOP( "To Field Last Name" ),
      TemplatesInsertCommand::COToLName
    },

    { I18N_NOOP( "CC Field Address" ),
      TemplatesInsertCommand::COCCAddr
    },

    { I18N_NOOP( "CC Field Name" ),
      TemplatesInsertCommand::COCCName
    },

    { I18N_NOOP( "CC Field First Name" ),
      TemplatesInsertCommand::COCCFName
    },

    { I18N_NOOP( "CC Field Last Name" ),
      TemplatesInsertCommand::COCCLName
    },

    { I18N_NOOP( "From Field Address" ),
      TemplatesInsertCommand::COFromAddr
    },

    { I18N_NOOP( "From Field Name" ),
      TemplatesInsertCommand::COFromName
    },

    { I18N_NOOP( "From Field First Name" ),
      TemplatesInsertCommand::COFromFName
    },

    { I18N_NOOP( "From Field Last Name" ),
      TemplatesInsertCommand::COFromLName },

    { I18N_NOOP( "Addresses of all recipients" ),
      TemplatesInsertCommand::COAddresseesAddr
    },

    { I18N_NOOP2( "Template value for subject of the message", "Subject" ),
      TemplatesInsertCommand::COFullSubject
    },

    { I18N_NOOP( "Quoted Headers" ),
      TemplatesInsertCommand::CQHeaders
    },

    { I18N_NOOP( "Headers as Is" ),
      TemplatesInsertCommand::CHeaders
    },

    { I18N_NOOP( "Header Content" ),
      TemplatesInsertCommand::COHeader
    },

    { I18N_NOOP( "Reply as Quoted Plain Text" ),
      TemplatesInsertCommand::CQuotePlain
    },

    { I18N_NOOP( "Reply as Quoted HTML Text" ),
      TemplatesInsertCommand::CQuoteHtml
    }
};
static const int originalCommandsCount =
        sizeof( originalCommands ) / sizeof( *originalCommands );

static const InsertCommand currentCommands[] =
{
    { I18N_NOOP( "Date" ),
      TemplatesInsertCommand::CDate
    },

    { I18N_NOOP( "Date in Short Format" ),
      TemplatesInsertCommand::CDateShort
    },

    { I18N_NOOP( "Date in C Locale" ),
      TemplatesInsertCommand::CDateEn
    },

    { I18N_NOOP( "Day of Week" ),
      TemplatesInsertCommand::CDow
    },

    { I18N_NOOP( "Time" ),
      TemplatesInsertCommand::CTime
    },

    { I18N_NOOP( "Time in Long Format" ),
      TemplatesInsertCommand::CTimeLong
    },

    { I18N_NOOP( "Time in C Locale" ),
      TemplatesInsertCommand::CTimeLongEn },
    { I18N_NOOP( "To Field Address" ),
      TemplatesInsertCommand::CToAddr
    },

    { I18N_NOOP( "To Field Name" ),
      TemplatesInsertCommand::CToName
    },

    { I18N_NOOP( "To Field First Name" ),
      TemplatesInsertCommand::CToFName
    },

    { I18N_NOOP( "To Field Last Name" ),
      TemplatesInsertCommand::CToLName
    },

    { I18N_NOOP( "CC Field Address" ),
      TemplatesInsertCommand::CCCAddr
    },

    { I18N_NOOP( "CC Field Name" ),
      TemplatesInsertCommand::CCCName
    },

    { I18N_NOOP( "CC Field First Name" ),
      TemplatesInsertCommand::CCCFName
    },

    { I18N_NOOP( "CC Field Last Name" ),
      TemplatesInsertCommand::CCCLName
    },

    { I18N_NOOP( "From Field Address" ),
      TemplatesInsertCommand::CFromAddr
    },

    { I18N_NOOP( "From field Name" ),
      TemplatesInsertCommand::CFromName
    },

    { I18N_NOOP( "From Field First Name" ),
      TemplatesInsertCommand::CFromFName
    },

    { I18N_NOOP( "From Field Last Name" ),
      TemplatesInsertCommand::CFromLName
    },

    { I18N_NOOP2( "Template subject command.", "Subject" ),
      TemplatesInsertCommand::CFullSubject
    },

    { I18N_NOOP( "Header Content" ),
      TemplatesInsertCommand::CHeader
    }
};
static const int currentCommandsCount = sizeof( currentCommands ) / sizeof( *currentCommands );

static const InsertCommand extCommands[] =
{
    { I18N_NOOP( "Pipe Original Message Body and Insert Result as Quoted Text" ),
      TemplatesInsertCommand::CQuotePipe
    },

    { I18N_NOOP( "Pipe Original Message Body and Insert Result as Is" ),
      TemplatesInsertCommand::CTextPipe
    },

    { I18N_NOOP( "Pipe Original Message with Headers and Insert Result as Is" ),
      TemplatesInsertCommand::CMsgPipe
    },

    { I18N_NOOP( "Pipe Current Message Body and Insert Result as Is" ),
      TemplatesInsertCommand::CBodyPipe
    },

    { I18N_NOOP( "Pipe Current Message Body and Replace with Result" ),
      TemplatesInsertCommand::CClearPipe
    }
};

static const int extCommandsCount =
        sizeof( extCommands ) / sizeof( *extCommands );

static const InsertCommand miscCommands[] =
{
    { I18N_NOOP2( "Inserts user signature, also known as footer, into message", "Signature" ),
      TemplatesInsertCommand::CSignature
    },

    { I18N_NOOP( "Insert File Content" ),
      TemplatesInsertCommand::CInsert
    },

    { I18N_NOOP2( "All characters, up to and including the next newline, "
      "are discarded without performing any macro expansion",
      "Discard to Next Line" ),
      TemplatesInsertCommand::CDnl
    },

    { I18N_NOOP( "Template Comment" ),
      TemplatesInsertCommand::CRem
    },

    { I18N_NOOP( "No Operation" ),
      TemplatesInsertCommand::CNop
    },

    { I18N_NOOP( "Clear Generated Message" ),
      TemplatesInsertCommand::CClear
    },

    { I18N_NOOP( "Turn Debug On" ),
      TemplatesInsertCommand::CDebug
    },

    { I18N_NOOP( "Turn Debug Off" ),
      TemplatesInsertCommand::CDebugOff
    },

    { I18N_NOOP( "Cursor position" ),
      TemplatesInsertCommand::CCursor
    },

    { I18N_NOOP( "Blank text" ),
      TemplatesInsertCommand::CBlank
    }
};
static const int miscCommandsCount = sizeof( miscCommands ) / sizeof( *miscCommands );

static void fillMenuFromActionMap( const QMap< QString, TemplatesInsertCommand::Command > &map,
                                   KActionMenu *menu, QSignalMapper *mapper )
{
    QMap< QString, TemplatesInsertCommand::Command >::const_iterator it = map.constBegin();
    QMap< QString, TemplatesInsertCommand::Command >::const_iterator end = map.constEnd();

    while ( it != end ) {
        KAction *action = new KAction( it.key(), menu ); //krazy:exclude=tipsandthis
        QObject::connect( action, SIGNAL(triggered(bool)), mapper, SLOT(map()) );
        mapper->setMapping( action, it.value() );
        menu->addAction( action );
        ++it;
    }
}

TemplatesInsertCommand::TemplatesInsertCommand( QWidget *parent, const QString &name )
    : QPushButton( parent )
{
    setObjectName( name );
    setText( i18n( "&Insert Command" ) );

    KActionMenu *menu;
    QMap< QString, Command > commandMap;

    QSignalMapper *mapper = new QSignalMapper( this );
    connect( mapper, SIGNAL(mapped(int)),
             this, SLOT(slotMapped(int)) );

    mMenu = new KActionMenu( i18n( "Insert Command" ), this );
    setToolTip(
                i18nc( "@info:tooltip",
                       "Select a command to insert into the template" ) );
    setWhatsThis(
                i18nc( "@info:whatsthis",
                       "Traverse this menu to find a command to insert into the current template "
                       "being edited.  The command will be inserted at the cursor location, "
                       "so you want to move your cursor to the desired insertion point first." ) );

    // ******************************************************
    menu = new KActionMenu( i18n( "Original Message" ), mMenu );
    mMenu->addAction( menu );

    // Map sorts commands
    for ( int i = 0; i < originalCommandsCount; ++i ) {
        commandMap.insert( originalCommands[i].getLocalizedDisplayName(), originalCommands[i].command );
    }

    fillMenuFromActionMap( commandMap, menu, mapper );
    commandMap.clear();

    // ******************************************************
    menu = new KActionMenu( i18n( "Current Message" ), mMenu );
    mMenu->addAction( menu );

    for ( int i = 0; i < currentCommandsCount; ++i ) {
        commandMap.insert( currentCommands[i].getLocalizedDisplayName(), currentCommands[i].command );
    }

    fillMenuFromActionMap( commandMap, menu, mapper );
    commandMap.clear();

    // ******************************************************
    menu = new KActionMenu( i18n( "Process with External Programs" ), mMenu );
    mMenu->addAction( menu );

    for ( int i = 0; i < extCommandsCount; ++i ) {
        commandMap.insert( extCommands[i].getLocalizedDisplayName(), extCommands[i].command );
    }

    fillMenuFromActionMap( commandMap, menu, mapper );
    commandMap.clear();

    // ******************************************************
    menu = new KActionMenu( i18nc( "Miscellaneous template commands menu", "Miscellaneous" ), mMenu );
    mMenu->addAction( menu );

    for ( int i = 0; i < miscCommandsCount; ++i ) {
        commandMap.insert( miscCommands[i].getLocalizedDisplayName(), miscCommands[i].command );
    }

    fillMenuFromActionMap( commandMap, menu, mapper );

    setMenu( mMenu->menu() );
}

TemplatesInsertCommand::~TemplatesInsertCommand()
{
}

void TemplatesInsertCommand::slotMapped( int cmd )
{
    emit insertCommand( static_cast<Command>( cmd ) );

    switch( cmd ) {
    case TemplatesInsertCommand::CQuote:
        emit insertCommand( QLatin1String("%QUOTE") );
        break;
    case TemplatesInsertCommand::CText:
        emit insertCommand( QLatin1String("%TEXT") );
        break;
    case TemplatesInsertCommand::COMsgId:
        emit insertCommand( QLatin1String("%OMSGID") );
        break;
    case TemplatesInsertCommand::CODate:
        emit insertCommand( QLatin1String("%ODATE") );
        break;
    case TemplatesInsertCommand::CODateShort:
        emit insertCommand( QLatin1String("%ODATESHORT") );
        break;
    case TemplatesInsertCommand::CODateEn:
        emit insertCommand( QLatin1String("%ODATEEN") );
        break;
    case TemplatesInsertCommand::CODow:
        emit insertCommand( QLatin1String("%ODOW") );
        break;
    case TemplatesInsertCommand::COTime:
        emit insertCommand( QLatin1String("%OTIME") );
        break;
    case TemplatesInsertCommand::COTimeLong:
        emit insertCommand( QLatin1String("%OTIMELONG") );
        break;
    case TemplatesInsertCommand::COTimeLongEn:
        emit insertCommand( QLatin1String("%OTIMELONGEN") );
        break;
    case TemplatesInsertCommand::COToAddr:
        emit insertCommand( QLatin1String("%OTOADDR" ));
        break;
    case TemplatesInsertCommand::COToName:
        emit insertCommand( QLatin1String("%OTONAME") );
        break;
    case TemplatesInsertCommand::COToFName:
        emit insertCommand( QLatin1String("%OTOFNAME") );
        break;
    case TemplatesInsertCommand::COToLName:
        emit insertCommand( QLatin1String("%OTOLNAME") );
        break;
    case TemplatesInsertCommand::COCCAddr:
        emit insertCommand( QLatin1String("%OCCADDR") );
        break;
    case TemplatesInsertCommand::COCCName:
        emit insertCommand( QLatin1String("%OCCNAME") );
        break;
    case TemplatesInsertCommand::COCCFName:
        emit insertCommand( QLatin1String("%OCCFNAME") );
        break;
    case TemplatesInsertCommand::COCCLName:
        emit insertCommand( QLatin1String("%OCCLNAME") );
        break;
    case TemplatesInsertCommand::COFromAddr:
        emit insertCommand( QLatin1String("%OFROMADDR") );
        break;
    case TemplatesInsertCommand::COFromName:
        emit insertCommand( QLatin1String("%OFROMNAME") );
        break;
    case TemplatesInsertCommand::COFromFName:
        emit insertCommand( QLatin1String("%OFROMFNAME") );
        break;
    case TemplatesInsertCommand::COFromLName:
        emit insertCommand( QLatin1String("%OFROMLNAME") );
        break;
    case TemplatesInsertCommand::COFullSubject:
        emit insertCommand( QLatin1String("%OFULLSUBJECT") );
        break;
    case TemplatesInsertCommand::CQHeaders:
        emit insertCommand( QLatin1String("%QHEADERS") );
        break;
    case TemplatesInsertCommand::CHeaders:
        emit insertCommand( QLatin1String("%HEADERS") );
        break;
    case TemplatesInsertCommand::COHeader:
        emit insertCommand( QLatin1String("%OHEADER=\"\""), -1 );
        break;
    case TemplatesInsertCommand::CMsgId:
        emit insertCommand( QLatin1String("%MSGID") );
        break;
    case TemplatesInsertCommand::CDate:
        emit insertCommand( QLatin1String("%DATE") );
        break;
    case TemplatesInsertCommand::CDateShort:
        emit insertCommand( QLatin1String("%DATESHORT") );
        break;
    case TemplatesInsertCommand::CDateEn:
        emit insertCommand( QLatin1String("%DATEEN") );
        break;
    case TemplatesInsertCommand::CDow:
        emit insertCommand( QLatin1String("%DOW") );
        break;
    case TemplatesInsertCommand::CTime:
        emit insertCommand( QLatin1String("%TIME") );
        break;
    case TemplatesInsertCommand::CTimeLong:
        emit insertCommand( QLatin1String("%TIMELONG") );
        break;
    case TemplatesInsertCommand::CTimeLongEn:
        emit insertCommand( QLatin1String("%TIMELONGEN") );
        break;
    case TemplatesInsertCommand::COAddresseesAddr:
        emit insertCommand( QLatin1String("%OADDRESSEESADDR") );
        break;
    case TemplatesInsertCommand::CToAddr:
        emit insertCommand( QLatin1String("%TOADDR") );
        break;
    case TemplatesInsertCommand::CToName:
        emit insertCommand( QLatin1String("%TONAME") );
        break;
    case TemplatesInsertCommand::CToFName:
        emit insertCommand( QLatin1String("%TOFNAME") );
        break;
    case TemplatesInsertCommand::CToLName:
        emit insertCommand( QLatin1String("%TOLNAME") );
        break;
    case TemplatesInsertCommand::CCCAddr:
        emit insertCommand( QLatin1String("%CCADDR") );
        break;
    case TemplatesInsertCommand::CCCName:
        emit insertCommand( QLatin1String("%CCNAME") );
        break;
    case TemplatesInsertCommand::CCCFName:
        emit insertCommand( QLatin1String("%CCFNAME") );
        break;
    case TemplatesInsertCommand::CCCLName:
        emit insertCommand( QLatin1String("%CCLNAME") );
        break;
    case TemplatesInsertCommand::CFromAddr:
        emit insertCommand( QLatin1String("%FROMADDR") );
        break;
    case TemplatesInsertCommand::CFromName:
        emit insertCommand( QLatin1String("%FROMNAME") );
        break;
    case TemplatesInsertCommand::CFromFName:
        emit insertCommand( QLatin1String("%FROMFNAME") );
        break;
    case TemplatesInsertCommand::CFromLName:
        emit insertCommand( QLatin1String("%FROMLNAME") );
        break;
    case TemplatesInsertCommand::CFullSubject:
        emit insertCommand( QLatin1String("%FULLSUBJECT") );
        break;
    case TemplatesInsertCommand::CHeader:
        emit insertCommand( QLatin1String("%HEADER=\"\""), -1 );
        break;
    case TemplatesInsertCommand::CSystem:
        emit insertCommand( QLatin1String("%SYSTEM=\"\""), -1 );
        break;
    case TemplatesInsertCommand::CQuotePipe:
        emit insertCommand( QLatin1String("%QUOTEPIPE=\"\""), -1 );
        break;
    case TemplatesInsertCommand::CTextPipe:
        emit insertCommand( QLatin1String("%TEXTPIPE=\"\""), -1 );
        break;
    case TemplatesInsertCommand::CMsgPipe:
        emit insertCommand( QLatin1String("%MSGPIPE=\"\""), -1 );
        break;
    case TemplatesInsertCommand::CBodyPipe:
        emit insertCommand( QLatin1String("%BODYPIPE=\"\""), -1 );
        break;
    case TemplatesInsertCommand::CClearPipe:
        emit insertCommand( QLatin1String("%CLEARPIPE=\"\""), -1 );
        break;
    case TemplatesInsertCommand::CCursor:
        emit insertCommand( QLatin1String("%CURSOR") );
        break;
    case TemplatesInsertCommand::CSignature:
        emit insertCommand( QLatin1String("%SIGNATURE") );
        break;
    case TemplatesInsertCommand::CInsert:
        emit insertCommand( QLatin1String("%INSERT=\"\""), -1 );
        break;
    case TemplatesInsertCommand::CDnl:
        emit insertCommand( QLatin1String("%-") );
        break;
    case TemplatesInsertCommand::CRem:
        emit insertCommand( QLatin1String("%REM=\"\""), -1 );
        break;
    case TemplatesInsertCommand::CNop:
        emit insertCommand( QLatin1String("%NOP") );
        break;
    case TemplatesInsertCommand::CClear:
        emit insertCommand( QLatin1String("%CLEAR") );
        break;
    case TemplatesInsertCommand::CDebug:
        emit insertCommand( QLatin1String("%DEBUG") );
        break;
    case TemplatesInsertCommand::CDebugOff:
        emit insertCommand( QLatin1String("%DEBUGOFF") );
        break;
    case TemplatesInsertCommand::CQuotePlain:
        emit insertCommand( QLatin1String("%FORCEDPLAIN") );
        break;
    case TemplatesInsertCommand::CQuoteHtml:
        emit insertCommand( QLatin1String("%FORCEDHTML") );
        break;
    default:
        kDebug() << "Unknown template command index:" << cmd;
        break;
    }
}

