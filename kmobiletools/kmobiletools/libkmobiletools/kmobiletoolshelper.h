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
#ifndef KMOBILETOOLSHELPER_H
#define KMOBILETOOLSHELPER_H

#include <libkmobiletools/kmobiletools_export.h>

#include <QtCore/QObject>
#include <QtCore/QDir>
#include <QtCore/QThread>

class KSystemTrayIcon;

/**
    @author Marco Gulino <marco@kmobiletools.org>
*/
class KMobiletoolsHelperPrivate;
namespace KMobileTools
{
class ContactsList;

class KMOBILETOOLS_EXPORT Thread : public QThread //krazy:exclude=dpointer
// We don't need d-pointer since it's just a wrapper class, and it contains static-only members.
{
    public:
        static void usleep(ulong);
        static void msleep(ulong);
        static void sleep(ulong);
};

    class KMOBILETOOLS_EXPORT KMobiletoolsHelper : public QObject
    {
    Q_OBJECT
    public:
        KMobiletoolsHelper(QObject *parent = 0);
        enum Connection {
            USB=0x1,
            Serial=0x2,
            IRDA=0x4,
            Bluetooth=0x8
        };
        enum SlotSource {
            PhoneBook=0x1,
            SMS=0x2
        };

        ~KMobiletoolsHelper();
        static KMobiletoolsHelper *instance();
        static QStringList getStdDevices(int connectionFlags);
        static QStringList getStdDevices(int connectionFlags, const QStringList &oldDevices);
        /**
         * Checks if number1 and number2 are equals.
         * */
        static bool compareNumbers(const QString &number1, const QString &number2);
            /** Checks if the number is stored in the addressbook and returns the
        * owner if found.
            */
        static QString translateNumber( const QString &s_number );
        static QString translateNumber( const QString &s_number, ContactsList *phoneBook );
        /** This method removes the international call prefix and normalizes.
        * This is used to compare two numbers with different prefix
        * shemes
        */
        static QString removeIntPrefix( const QString &number );
        /** This method can give us a standard template for all KMobileTools item details, i.e. SMS, PhoneBook, and.. we'll see ;-)
         * */
        static QString getTemplate();
        /** This method parses arguments so that it can give us a HTML link
         * */
        static QString getFooterLink(const QString &text, const QString &iconName, const QString &url);
        static void createMailDir(const QString &dirname);
        static QString mkMailDir(const QString &dirname, bool isParent=false);
        static QString shortMonthNameEng(int month);
        static QString shortWeekDayNameEng(int day);
        KSystemTrayIcon *systray();
        void setSystray(KSystemTrayIcon *s);
        static QString memorySlotsDescriptions(const QString &slot, int type);
    private:
        KMobiletoolsHelperPrivate *const d;
    };
}
#endif
