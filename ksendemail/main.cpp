/*
    This file is part of KSendEmail.
    Copyright (c) 2008 Pradeepto Bhattacharya <pradeepto@kde.org>
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "mailerservice.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

static const char description[] =
  I18N_NOOP( "KDE Command Line Emailer." );

static KCmdLineOptions kmail_options ()
{
    KCmdLineOptions options;
    options.add("s");
    options.add("subject <subject>", ki18n("Set subject of message"));
    options.add("c");
    options.add("cc <address>",      ki18n("Send CC: to 'address'"));
    options.add("b");
    options.add("bcc <address>",     ki18n("Send BCC: to 'address'"));
    options.add("h");
    options.add("header <header>",   ki18n("Add 'header' to message"));
    options.add("msg <file>",        ki18n("Read message body from 'file'"));
    options.add("body <text>",       ki18n("Set body of message"));
    options.add("attach <url>",      ki18n("Add an attachment to the mail. This can be repeated"));
    options.add("composer",          ki18n("Only open composer window"));
    options.add("+[address]",        ki18n("Address to send the message to"));
    return options;
}


int main( int argc, char **argv )
{
  KAboutData aboutData( "ksendemail", 0, ki18n( "KSendEmail" ), "0.01", ki18n(description),
                      KAboutData::License_GPL,
                      ki18n( "(C) 2008 Pradeepto Bhattacharya" ),
                             KLocalizedString(), "http://kontact.kde.org" );

  aboutData.addAuthor( ki18n( "Pradeepto Bhattacharya" ), KLocalizedString(), "pradeepto@kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( kmail_options() );

  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  MailerService *ms = new MailerService();
  ms->processArgs( args );
  args->clear();
  delete ms;
  return 0;
}
