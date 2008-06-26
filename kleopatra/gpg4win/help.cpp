/* -*- mode: c++; c-basic-offset:4 -*-
    gpg4win/help.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
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

#include "help.h"

#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KMessageBox>
#include <KStandardDirs>
#include <KToolInvocation>
#include <KUrl>

#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QWidget>

using namespace Kleo::Gpg4Win;

namespace {
    //from kdelibs/kdoctools/kio_help.cpp
    QString langLookup(const QString &fname)
    {
        QStringList search;

        // assemble the local search paths
        const QStringList localDoc = KGlobal::dirs()->resourceDirs("html");

        QStringList langs = KGlobal::locale()->languageList();
        langs.append( "en" );
        langs.removeAll( "C" );

        // this is kind of compat hack as we install our docs in en/ but the
        // default language is en_US
        for (QStringList::Iterator it = langs.begin(); it != langs.end(); ++it)
            if ( *it == "en_US" )
                *it = "en";

        // look up the different languages
        int ldCount = localDoc.count();
        for (int id=0; id < ldCount; id++)
        {
            QStringList::ConstIterator lang;
            for (lang = langs.begin(); lang != langs.end(); ++lang)
                search.append(QString("%1%2/%3").arg(localDoc[id], *lang, fname));
        }

        // try to locate the file
        for (QStringList::Iterator it = search.begin(); it != search.end(); ++it)
        {
            kDebug( 7119 ) << "Looking for help in: " << *it;

            QFileInfo info(*it);
            if (info.exists() && info.isFile() && info.isReadable())
                return *it;
        }
        return QString();
    }

    void showError( QWidget* parent, const QString & text ) {
        KMessageBox::error( parent, text, i18n( "No Help Available" ) );
    }
}

void Kleo::Gpg4Win::showHelp( QWidget* parent, const QString & appname ) {
    const QString file = langLookup( appname + "/index.html" );
    if ( file.isEmpty() ) {
        showError( parent, i18n( "No documentation for application %1 installed.", appname ) );
        return;
    }
    KToolInvocation::invokeBrowser( KUrl::fromPath( file ).url() );
}

