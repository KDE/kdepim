/*
    test_cryptoconfig.cpp

    This file is part of libkleopatra's test suite.
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "backends/qgpgme/qgpgmecryptoconfig.h"
#include "backends/qgpgme/qgpgmenewcryptoconfig.h"

#include <QCoreApplication>
#include <iostream>

using namespace std;

#include <gpgme++/global.h>
#include <gpgme++/error.h>

#include <stdlib.h>
#include <assert.h>

int main(int argc, char **argv)
{

    if (GpgME::initializeLibrary(0)) {
        return 1;
    }

    const bool newCryptoConfig = argc == 2 && qstrcmp(argv[1], "--new") == 0;
    if (newCryptoConfig) {
        argc = 1;    // hide from KDE
    }

    QCoreApplication::setApplicationName(QStringLiteral("test_cryptoconfig"));
    QCoreApplication app(argc, argv);

    Kleo::CryptoConfig *config = 0;
    if (newCryptoConfig) {
        config = new QGpgMENewCryptoConfig;
    } else {
        config = new QGpgMECryptoConfig;
    }

    // Dynamic querying of the options
    cout << "Components:" << endl;
    QStringList components = config->componentList();

    for (QStringList::Iterator compit = components.begin(); compit != components.end(); ++compit) {
        cout << "Component " << (*compit).toLocal8Bit().constData() << ":" << endl;
        const Kleo::CryptoConfigComponent *comp = config->component(*compit);
        assert(comp);
        QStringList groups = comp->groupList();
        for (QStringList::Iterator groupit = groups.begin(); groupit != groups.end(); ++groupit) {
            const Kleo::CryptoConfigGroup *group = comp->group(*groupit);
            assert(group);
            cout << " Group " << (*groupit).toLocal8Bit().constData() << ": descr=\"" << group->description().toLocal8Bit().constData() << "\""
                 << " level=" << group->level() << endl;
            QStringList entries = group->entryList();
            for (QStringList::Iterator entryit = entries.begin(); entryit != entries.end(); ++entryit) {
                const Kleo::CryptoConfigEntry *entry = group->entry(*entryit);
                assert(entry);
                cout << "  Entry " << (*entryit).toLocal8Bit().constData() << ":"
                     << " descr=\"" << entry->description().toLocal8Bit().constData() << "\""
                     << " " << (entry->isSet() ? "is set" : "is not set");
                if (!entry->isList())
                    switch (entry->argType()) {
                    case Kleo::CryptoConfigEntry::ArgType_None:
                        break;
                    case Kleo::CryptoConfigEntry::ArgType_Int:
                        cout << " int value=" << entry->intValue();
                        break;
                    case Kleo::CryptoConfigEntry::ArgType_UInt:
                        cout << " uint value=" << entry->uintValue();
                        break;
                    case Kleo::CryptoConfigEntry::ArgType_LDAPURL:
                    case Kleo::CryptoConfigEntry::ArgType_Path:
                    // fallthrough
                    case Kleo::CryptoConfigEntry::ArgType_DirPath:
                    // fallthrough
                    case Kleo::CryptoConfigEntry::ArgType_String:

                        cout << " string value=" << entry->stringValue().toLocal8Bit().constData();
                        break;
                    case Kleo::CryptoConfigEntry::NumArgType:
                        // just metadata and should never actually occur in the switch
                        break;
                    }
                else { // lists
                    switch (entry->argType()) {
                    case Kleo::CryptoConfigEntry::ArgType_None: {
                        cout << " set " << entry->numberOfTimesSet() << " times";
                        break;
                    }
                    case Kleo::CryptoConfigEntry::ArgType_Int: {
                        // (marc) if an entry isn't optional, you have to unset it for the default to take effect, so this assert is wrong:
                        // assert( entry->isOptional() ); // empty lists must be allowed (see https://www.intevation.de/roundup/aegypten/issue121)
                        std::vector<int> lst = entry->intValueList();
                        QString str;
                        for (std::vector<int>::const_iterator it = lst.begin(); it != lst.end(); ++it) {
                            str += QString::number(*it);
                        }
                        cout << " int values=" << str.toLocal8Bit().constData();
                        break;
                    }
                    case Kleo::CryptoConfigEntry::ArgType_UInt: {
                        // (marc) if an entry isn't optional, you have to unset it for the default to take effect, so this assert is wrong:
                        // assert( entry->isOptional() ); // empty lists must be allowed (see https://www.intevation.de/roundup/aegypten/issue121)
                        std::vector<uint> lst = entry->uintValueList();
                        QString str;
                        for (std::vector<uint>::const_iterator it = lst.begin(); it != lst.end(); ++it) {
                            str += QString::number(*it);
                        }
                        cout << " uint values=" << str.toLocal8Bit().constData();
                        break;
                    }
                    case Kleo::CryptoConfigEntry::ArgType_LDAPURL: {
                        // (marc) if an entry isn't optional, you have to unset it for the default to take effect, so this assert is wrong:
                        // assert( entry->isOptional() ); // empty lists must be allowed (see https://www.intevation.de/roundup/aegypten/issue121)
                        QList<QUrl> urls = entry->urlValueList();
                        cout << " url values ";
                        Q_FOREACH (const QUrl &url, urls) {
                            cout << url.toString().toLocal8Bit().constData() << " ";
                        }
                        cout << endl;
                    }
                    // fallthrough
                    case Kleo::CryptoConfigEntry::ArgType_Path:
                    // fallthrough
                    case Kleo::CryptoConfigEntry::ArgType_DirPath:
                    // fallthrough
                    case Kleo::CryptoConfigEntry::ArgType_String:
                    // fallthrough string value lists were removed from
                    // gpgconf in 2008
                    case Kleo::CryptoConfigEntry::NumArgType:
                        // just metadata and should never actually occur in the switch
                        break;
                    }
                }
                cout << endl;
            }
            // ...
        }
    }

    {
        // Static querying of a single boolean option
        static const char s_groupName[] = "Monitor";
        static const char s_entryName[] = "quiet";
        Kleo::CryptoConfigEntry *entry = config->entry(QStringLiteral("dirmngr"), s_groupName, s_entryName);
        if (entry) {
            assert(entry->argType() == Kleo::CryptoConfigEntry::ArgType_None);
            bool val = entry->boolValue();
            cout << "quiet option initially: " << (val ? "is set" : "is not set") << endl;

            entry->setBoolValue(!val);
            assert(entry->isDirty());
            config->sync(true);

            // Clear cached values!
            config->clear();

            // Check new value
            Kleo::CryptoConfigEntry *entry = config->entry(QStringLiteral("dirmngr"), s_groupName, s_entryName);
            assert(entry);
            assert(entry->argType() == Kleo::CryptoConfigEntry::ArgType_None);
            cout << "quiet option now: " << (val ? "is set" : "is not set") << endl;
            assert(entry->boolValue() == !val);

            // Set to default
            entry->resetToDefault();
            assert(entry->boolValue() == false);   // that's the default
            assert(entry->isDirty());
            assert(!entry->isSet());
            config->sync(true);
            config->clear();

            // Check value
            entry = config->entry(QStringLiteral("dirmngr"), s_groupName, s_entryName);
            assert(!entry->isDirty());
            assert(!entry->isSet());
            cout << "quiet option reset to default: " << (entry->boolValue() ? "is set" : "is not set") << endl;
            assert(entry->boolValue() == false);

            // Reset old value
            entry->setBoolValue(val);
            assert(entry->isDirty());
            config->sync(true);

            cout << "quiet option reset to initial: " << (val ? "is set" : "is not set") << endl;
        } else {
            cout << "Entry 'dirmngr/" << s_groupName << "/" << s_entryName << "' not found" << endl;
        }
    }

    {
        // Static querying and setting of a single int option
        static const char s_groupName[] = "LDAP";
        static const char s_entryName[] = "ldaptimeout";
        Kleo::CryptoConfigEntry *entry = config->entry(QStringLiteral("dirmngr"), s_groupName, s_entryName);
        if (entry) {
            assert(entry->argType() == Kleo::CryptoConfigEntry::ArgType_UInt);
            uint val = entry->uintValue();
            cout << "LDAP timeout initially: " << val << " seconds." << endl;

            // Test setting the option directly, then querying again
            //system( "echo 'ldaptimeout:0:101' | gpgconf --change-options dirmngr" );
            // Now let's do it with the C++ API instead
            entry->setUIntValue(101);
            assert(entry->isDirty());
            config->sync(true);

            // Clear cached values!
            config->clear();

            // Check new value
            Kleo::CryptoConfigEntry *entry = config->entry(QStringLiteral("dirmngr"), s_groupName, s_entryName);
            assert(entry);
            assert(entry->argType() == Kleo::CryptoConfigEntry::ArgType_UInt);
            cout << "LDAP timeout now: " << entry->uintValue() << " seconds." << endl;
            assert(entry->uintValue() == 101);

            // Set to default
            entry->resetToDefault();
            assert(entry->uintValue() == 100);
            assert(entry->isDirty());
            assert(!entry->isSet());
            config->sync(true);
            config->clear();

            // Check value
            entry = config->entry(QStringLiteral("dirmngr"), s_groupName, s_entryName);
            assert(!entry->isDirty());
            assert(!entry->isSet());
            cout << "LDAP timeout reset to default, " << entry->uintValue() << " seconds." << endl;
            assert(entry->uintValue() == 100);

            // Reset old value
            entry->setUIntValue(val);
            assert(entry->isDirty());
            config->sync(true);

            cout << "LDAP timeout reset to initial " << val << " seconds." << endl;
        } else {
            cout << "Entry 'dirmngr/" << s_groupName << "/" << s_entryName << "' not found" << endl;
        }
    }

    {
        // Static querying and setting of a single string option
        static const char s_groupName[] = "Debug";
        static const char s_entryName[] = "log-file";
        Kleo::CryptoConfigEntry *entry = config->entry(QStringLiteral("dirmngr"), s_groupName, s_entryName);
        if (entry) {
            assert(entry->argType() == Kleo::CryptoConfigEntry::ArgType_Path);
            QString val = entry->stringValue();
            cout << "Log-file initially: " << val.toLocal8Bit().constData() << endl;

            // Test setting the option, sync'ing, then querying again
            entry->setStringValue(QStringLiteral("/tmp/test:%e5ä"));
            assert(entry->isDirty());
            config->sync(true);

            // Let's see how it prints it
            system("gpgconf --list-options dirmngr | grep log-file");

            // Clear cached values!
            config->clear();

            // Check new value
            Kleo::CryptoConfigEntry *entry = config->entry(QStringLiteral("dirmngr"), s_groupName, s_entryName);
            assert(entry);
            assert(entry->argType() == Kleo::CryptoConfigEntry::ArgType_Path);
            cout << "Log-file now: " << entry->stringValue().toLocal8Bit().constData() << endl;
            assert(entry->stringValue() == QStringLiteral("/tmp/test:%e5ä"));     // (or even with %e5 decoded)

            // Reset old value
#if 0
            QString arg(val);
            if (!arg.isEmpty()) {
                arg.prepend('"');
            }
            Q3CString sys;
            sys.sprintf("echo 'log-file:%s' | gpgconf --change-options dirmngr", arg.local8Bit().data());
            system(sys.data());
#endif
            entry->setStringValue(val);
            assert(entry->isDirty());
            config->sync(true);

            cout << "Log-file reset to initial " << val.toLocal8Bit().constData() << endl;
        } else {
            cout << "Entry 'dirmngr/" << s_groupName << "/" << s_entryName << "' not found" << endl;
        }
    }

    {
        // Static querying and setting of the LDAP URL list option
        static const char s_groupName[] = "LDAP";
        static const char s_entryName[] = "LDAP Server";
        Kleo::CryptoConfigEntry *entry = config->entry(QStringLiteral("dirmngr"), s_groupName, s_entryName);
        if (entry) {
            assert(entry->argType() == Kleo::CryptoConfigEntry::ArgType_LDAPURL);
            assert(entry->isList());
            QList<QUrl> val = entry->urlValueList();
            cout << "URL list initially: ";
            Q_FOREACH (const QUrl &url, val) {
                cout << url.toString().toLocal8Bit().constData() << ", ";
            }
            cout << endl;

            // Test setting the option, sync'ing, then querying again
            QList<QUrl> lst;
            lst << QUrl(QStringLiteral("ldap://a:389?b"));
            lst << QUrl(QStringLiteral("ldap://foo:389?a:b c"));
            lst << QUrl(QStringLiteral("ldap://server:389?a=b,c=DE"));   // the query contains a literal ','
            //cout << " trying to set: " << lst.toStringList().join(", ").local8Bit() << endl;
            assert(lst[0].query() == "b");
            assert(lst[1].query() == "a:b c");   // see, the space got _not_escaped
            entry->setURLValueList(lst);
            assert(entry->isDirty());
            config->sync(true);

            // Let's see how it prints it
            system("gpgconf --list-options dirmngr | grep 'LDAP Server'");

            // Clear cached values!
            config->clear();

            // Check new value
            Kleo::CryptoConfigEntry *entry = config->entry(QStringLiteral("dirmngr"), s_groupName, s_entryName);
            assert(entry);
            assert(entry->argType() == Kleo::CryptoConfigEntry::ArgType_LDAPURL);
            assert(entry->isList());
            // Get QUrl form
            QList<QUrl> newlst = entry->urlValueList();
            cout << "URL list now: ";
            Q_FOREACH (const QUrl &url, newlst) {
                cout << url.toString().toLocal8Bit().constData() << endl;
            }
            cout << endl;
            assert(newlst.count() == 3);
            assert(newlst[0].url() == lst[0].url());
            assert(newlst[1].url() == lst[1].url());
            assert(newlst[2].url() == lst[2].url());

            // Reset old value
            entry->setURLValueList(val);
            assert(entry->isDirty());
            config->sync(true);

            cout << "URL list reset to initial: ";
            Q_FOREACH (const QUrl &url, newlst) {
                cout << url.toString().toLocal8Bit().constData() << ", ";
            }
            cout << endl;
        } else {
            cout << "Entry 'dirmngr/" << s_groupName << "/" << s_entryName << "' not found" << endl;
        }
    }

    cout << "Done." << endl;
}
