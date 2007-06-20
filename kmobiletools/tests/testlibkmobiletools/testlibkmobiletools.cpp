/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>


   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.


   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.


   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#include "testlibkmobiletools.h"
#include <libkmobiletools/sms.h>
#include <QTextStream>
#include <kcmdlineargs.h>
#include <QTimer>

// using namespace KMobileTools;

TestLibKMobileToolsApp::TestLibKMobileToolsApp()
    : KApplication(false), in(stdin, QIODevice::ReadOnly), out(stdout, QIODevice::WriteOnly), err(stderr, QIODevice::WriteOnly)
{
    mainloop();
}

TestLibKMobileToolsApp::~TestLibKMobileToolsApp()
{
}

void TestLibKMobileToolsApp::help() {
    out << "Available commands:\n"
    << "help\t\tThis help screen\n"
    << "sms\t\tTest sms object\n"
    << "quit\t\tClose this application\n"
    ;
}

void TestLibKMobileToolsApp::mainloop() {
    bool ok=false;
    out << "Enter a command to test libkmobiletools. \"help\" to see available commands\n> ";
    out.flush();
    QString cmd;
    in >> cmd;
    if(cmd=="help" || cmd=="?") help();
    if(cmd=="sms") checkSMS();
    if(cmd=="quit" || cmd=="q") { QTimer::singleShot(200, this, SLOT(quit()) ) ; return; }
    mainloop();
}

void TestLibKMobileToolsApp::checkSMS() {
    out << "LibKMobileTools tester application\n";
    SMS *sms=new SMS();
    out << "sms created\n";
    sms->setText("Testo di prova");
    out << "setting test to sms: " << sms->getText() << endl;
    out << "Raw content: " << sms->body() << endl;
    sms->setDateTime( KDateTime(QDate(2002,7,2), QTime(21,12,13) ) );
    out << "Set date time to " << sms->getDateTime().toString() << endl;
    sms->assemble();
    out << "****************** SMS Serialization ******************\n"
        << sms->encodedContent()
    << "\n**************** SMS Serialization End ****************\n\n";
    out << "Deleting SMS...";
    delete sms;
    out << " Done" << endl;
}

#include "testlibkmobiletools.moc"

int main(int argc, char** argv)
{
    KCmdLineArgs::init(argc, argv, "testlibkmobiletools", "testlibkmobiletools", "a little test tool", "0.1");
    TestLibKMobileToolsApp *app=new TestLibKMobileToolsApp();
    return app->exec();
}
