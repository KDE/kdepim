/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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
#include "parseuserjobtest.h"
#include "ksieveui/managescriptsjob/parseuserscriptjob.h"
#include <qtest.h>


QTEST_MAIN( ParseUserTest )

void ParseUserTest::testParseEmptyUserJob()
{
    const QString script;
    bool result;
    const QStringList lst = KSieveUi::ParseUserScriptJob::parsescript(script, result);
    QCOMPARE(lst.count(), 0);
    QCOMPARE(result, true);
}

void ParseUserTest::testParseUserTwoActiveScriptJob()
{
    const QString script = QLatin1String("# USER Management Script\n"
                                         "#\n"
                                         "# This script includes the various active sieve scripts\n"
                                         "# it is AUTOMATICALLY GENERATED. DO NOT EDIT MANUALLY!\n"
                                         "# \n"
                                         "# For more information, see http://wiki.kolab.org/KEP:14#USER\n"
                                         "#\n"
                                         "\n"
                                         "require [\"include\"];\n"
                                         "include :personal \"file1\";\n"
                                         "include :personal \"file2\";\n");
    bool result;
    const QStringList lst = KSieveUi::ParseUserScriptJob::parsescript(script, result);
    QCOMPARE(lst.count(), 2);
    QCOMPARE(result, true);
}

void ParseUserTest::testParseUserNoActiveScriptJob()
{
    const QString script = QLatin1String("# USER Management Script\n"
                                         "#\n"
                                         "# This script includes the various active sieve scripts\n"
                                         "# it is AUTOMATICALLY GENERATED. DO NOT EDIT MANUALLY!\n"
                                         "# \n"
                                         "# For more information, see http://wiki.kolab.org/KEP:14#USER\n"
                                         "#\n"
                                         "\n"
                                         "require [\"include\"];\n");
    bool result;
    const QStringList lst = KSieveUi::ParseUserScriptJob::parsescript(script, result);
    QCOMPARE(lst.count(), 0);
    QCOMPARE(result, true);
}

void ParseUserTest::testParseUserDuplicateActiveScriptJob()
{
    const QString script = QLatin1String("# USER Management Script\n"
                                         "#\n"
                                         "# This script includes the various active sieve scripts\n"
                                         "# it is AUTOMATICALLY GENERATED. DO NOT EDIT MANUALLY!\n"
                                         "# \n"
                                         "# For more information, see http://wiki.kolab.org/KEP:14#USER\n"
                                         "#\n"
                                         "\n"
                                         "require [\"include\"];\n"
                                         "include :personal \"file1\";\n"
                                         "include :personal \"file1\";\n");
    bool result;
    const QStringList lst = KSieveUi::ParseUserScriptJob::parsescript(script, result);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(result, true);
}

void ParseUserTest::testParseUserErrorScriptJob()
{
    const QString script = QLatin1String("# USER Management Script\n"
                                         "#\n"
                                         "# This script includes the various active sieve scripts\n"
                                         "# it is AUTOMATICALLY GENERATED. DO NOT EDIT MANUALLY!\n"
                                         "# \n"
                                         "# For more information, see http://wiki.kolab.org/KEP:14#USER\n"
                                         "#\n"
                                         "\n"
                                         "errorscript\n");
    bool result;
    const QStringList lst = KSieveUi::ParseUserScriptJob::parsescript(script, result);
    QCOMPARE(lst.count(), 0);
    QCOMPARE(result, false);
}


