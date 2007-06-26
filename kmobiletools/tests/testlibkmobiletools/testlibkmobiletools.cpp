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
#include <libkmobiletools/akonadi/akonadi_serializer_sms.cpp>
#include <QTextStream>
#include <kcmdlineargs.h>
#include <QTimer>
#include <libakonadi/collection.h>
#include <KDateTime>

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
    << "akonadi\t\tTest Akonadi resources object\n"
    << "quit\t\tClose this application\n\n"
    ;
}

void TestLibKMobileToolsApp::mainloop() {
    out << "Enter a command to test libkmobiletools. \"help\" to see available commands\n> ";
    out.flush();
    QString cmd;
    in >> cmd;
    if(cmd=="help" || cmd=="?") help();
    if(cmd=="sms") checkSMS();
    if(cmd=="akonadi") testAkonadi();
    if(cmd=="quit" || cmd=="q") { QTimer::singleShot(200, this, SLOT(quit()) ) ; return; }
    mainloop();
}

KMobileTools::SMS *TestLibKMobileToolsApp::checkSMS(bool deleteOnReturn) {
    out << "LibKMobileTools tester application\n";
    KMobileTools::SMS *sms=new KMobileTools::SMS();
    out << "sms created\n";
    sms->setText("Testo di prova");
    out << "setting test to sms: " << sms->getText() << endl;
    out << "Raw content: " << sms->body() << endl;
    sms->setDateTime( KDateTime(QDate(2002,7,2), QTime(21,12,13) ) );
    sms->setUnread(false);
    out << "Set date time to " << sms->getDateTime().toString() << endl;
    sms->setSender( "+39123456789", "Sender Name");
    out << "Added numbers: Sender=" << sms->sender()->as7BitString() << endl;
    sms->addDestination("+22123456789", "Destination Name #1");
    sms->addDestination("+33345678923", "Destination Name #2 testing some special chars.. #@1\"<>, ");
    sms->sender()->from7BitString("Sender: Sender Name #1 <+33345678923>, \"Sender 2\" <1234543>, \"Sender 3 with ,commas,\" <224123>, \"Sender 4 with \\\"quotes\\\"\" <1234567>");
    sms->setType( KMobileTools::SMS::Unread );

    out << endl;
    sms->assemble();
    out << "****************** SMS Serialization ******************\n"
        << sms->encodedContent()
    << "\n**************** SMS Serialization End ****************\n\n";
    if(deleteOnReturn) { out << "Deleting SMS..."; delete sms; sms=NULL; out << " Done" << endl; }
    return sms;
}

void TestLibKMobileToolsApp::testAkonadi() {
    Akonadi::Item i;
    i.setMimeType(SMS_MIMETYPE);
    KMobileTools::SMS *sms=checkSMS(false);
    Akonadi::SerializerPluginSMS *serializer=new Akonadi::SerializerPluginSMS();
    MessagePtr msg=MessagePtr(sms);
    i.setPayload<MessagePtr>(msg);
    delete serializer;
}

#include "testlibkmobiletools.moc"

int main(int argc, char** argv)
{
    KCmdLineArgs::init(argc, argv, "testlibkmobiletools", "testlibkmobiletools", "a little test tool", "0.1");
    TestLibKMobileToolsApp *app=new TestLibKMobileToolsApp();
    return app->exec();
}
