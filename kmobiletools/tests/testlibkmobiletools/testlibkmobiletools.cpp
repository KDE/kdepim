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

// using namespace KMobileTools;

TestLibKMobileToolsApp::TestLibKMobileToolsApp()
    : KApplication(false)
{
    QTextStream out(stdout, QIODevice::WriteOnly);
    out << "LibKMobileTools tester application\n";
    SMS *sms=new SMS();
    out << "sms created\n";
    sms->setText("Testo di prova");
    out << "setting test to sms: " << sms->getText() << endl;
    out << "Raw content: " << sms->body() << endl;
    sms->setDateTime( KDateTime(QDate(2002,7,2), QTime(21,12,13) ) );
    out << "Set date time to " << sms->getDateTime().toString() << endl;
    sms->assemble();
    out << "****************** SMS Serialization ******************\n\n"
        << sms->encodedContent()
    << "\n\n**************** SMS Serialization End ****************\n\n";
    out << "Deleting SMS...";
    delete sms;
    out << " Done" << endl;
    quit();
}

#include "testlibkmobiletools.moc"

int main(int argc, char** argv)
{
    KCmdLineArgs::init(argc, argv, "testlibkmobiletools", "testlibkmobiletools", "a little test tool", "0.1");
    TestLibKMobileToolsApp *app=new TestLibKMobileToolsApp();
    return app->exec();
}
