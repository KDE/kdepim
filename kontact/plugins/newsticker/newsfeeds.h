/*
    This file is part of Kontact.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef NEWSFEEDS_H
#define NEWSFEEDS_H

#include <tqvaluelist.h>

#define DEFAULT_NEWSSOURCES 60

class NewsSourceData
{
  public:
    typedef TQValueList<NewsSourceData> List;

    enum Category { Arts, Business, Computers, Misc,
                    Recreation, Society };

    NewsSourceData( const TQString &name = I18N_NOOP( "Unknown" ),
                    const TQString &url = TQString::null,
                    const TQString &icon = TQString::null,
                    const Category category= Computers )
      : mName( name ), mURL( url ), mIcon( icon ), mCategory( category )
    {
    }

    TQString name() const { return mName; }
    TQString url() const { return mURL; }
    TQString icon() const { return mIcon; }
    Category category() const { return mCategory; }

    TQString mName;
    TQString mURL;
    TQString mIcon;
    Category mCategory;
};

static NewsSourceData NewsSourceDefault[DEFAULT_NEWSSOURCES] = {
  // Arts ---------------
    NewsSourceData(
    TQString::fromLatin1("Bureau 42"),
    TQString::fromLatin1("http://www.bureau42.com/rdf/"),
    TQString::fromLatin1("http://www.bureau42.com/favicon.ico"),
    NewsSourceData::Arts ),
    NewsSourceData(
    TQString::fromLatin1("eFilmCritic"),
    TQString::fromLatin1("http://efilmcritic.com/fo.rdf"),
    TQString::fromLatin1("http://efilmcritic.com/favicon.ico"),
    NewsSourceData::Arts ),
  // Business -----------
    NewsSourceData(
    TQString::fromLatin1("Internet.com Business"),
    TQString::fromLatin1("http://headlines.internet.com/internetnews/bus-news/news.rss"),
    TQString::null,
    NewsSourceData::Business ),
    NewsSourceData(
    TQString::fromLatin1("TradeSims"),
    TQString::fromLatin1("http://www.tradesims.com/AEX.rdf"),
    TQString::null,
    NewsSourceData::Business ),
  // Computers ----------
    NewsSourceData(
    TQString::fromLatin1("KDE Deutschland"),
    TQString::fromLatin1("http://www.kde.de/nachrichten/nachrichten.rdf"),
    TQString::fromLatin1("http://www.kde.de/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("KDE France"),
    TQString::fromLatin1("http://www.kde-france.org/backend-breves.php3"),
    TQString::null,
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("FreeBSD Project News"),
    TQString::fromLatin1("http://www.freebsd.org/news/news.rdf"),
    TQString::fromLatin1("http://www.freebsd.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("dot.kde.org"),
    TQString::fromLatin1("http://www.kde.org/dotkdeorg.rdf"),
    TQString::fromLatin1("http://www.kde.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData( TQString::fromLatin1("KDE-Look.org"),
                    TQString::fromLatin1("http://www.kde.org/kde-look-content.rdf"),
                    TQString::fromLatin1("http://kde-look.org/img/favicon-1-1.ico"),
                    NewsSourceData::Computers ),
    NewsSourceData( TQString::fromLatin1("KDE-Apps.org"),
                    TQString::fromLatin1("http://www.kde.org/dot/kde-apps-content.rdf"),
                    TQString::fromLatin1("http://kde-apps.org/img/favicon-1-1.ico"),
                    NewsSourceData::Computers ),
    NewsSourceData( TQString::fromLatin1("DesktopLinux"),
                    TQString::fromLatin1("http://www.desktoplinux.com/backend/index.html"),
                    TQString::fromLatin1("http://www.desktoplinux.com/images/favicon.ico"),
                    NewsSourceData::Computers ),
    NewsSourceData( TQString::fromLatin1("DistroWatch"),
                    TQString::fromLatin1("http://distrowatch.com/news/dw.xml"),
                    TQString::fromLatin1("http://distrowatch.com/favicon.ico"),
                    NewsSourceData::Computers ),
    /*URL changed*/
    NewsSourceData(
    TQString::fromLatin1("GNOME News"),
    TQString::fromLatin1("http://www.gnomedesktop.org/node/feed"),
    TQString::null,
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("Slashdot"),
    TQString::fromLatin1("http://slashdot.org/slashdot.rdf"),
    TQString::fromLatin1("http://slashdot.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("Ask Slashdot"),
    TQString::fromLatin1("http://slashdot.org/askslashdot.rdf"),
    TQString::fromLatin1("http://slashdot.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("Slashdot: Features"),
    TQString::fromLatin1("http://slashdot.org/features.rdf"),
    TQString::fromLatin1("http://slashdot.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("Slashdot: Apache"),
    TQString::fromLatin1("http://slashdot.org/apache.rdf"),
    TQString::fromLatin1("http://slashdot.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("Slashdot: Books"),
    TQString::fromLatin1("http://slashdot.org/books.rdf"),
    TQString::fromLatin1("http://slashdot.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("Jabber News"),
    TQString::fromLatin1("http://www.jabber.org/news/rss.xml"),
    TQString::null,
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("Freshmeat"),
    TQString::fromLatin1("http://freshmeat.net/backend/fm.rdf"),
    TQString::fromLatin1("http://freshmeat.net/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("Linux Weekly News"),
    TQString::fromLatin1("http://www.lwn.net/headlines/rss"),
    TQString::fromLatin1("http://www.lwn.net/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("heise online news"),
    TQString::fromLatin1("http://www.heise.de/newsticker/heise.rdf"),
    TQString::fromLatin1("http://www.heise.de/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("RUS-CERT Ticker"),
    TQString::fromLatin1("http://cert.uni-stuttgart.de/ticker/rus-cert.rdf"),
    TQString::fromLatin1("http://cert.uni-stuttgart.de/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("RUS-CERT Elsewhere"),
    TQString::fromLatin1("http://cert.uni-stuttgart.de/ticker/rus-cert-elsewhere.rdf"),
    TQString::fromLatin1("http://cert.uni-stuttgart.de/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("Kuro5hin"),
    TQString::fromLatin1("http://kuro5hin.org/backend.rdf"),
    TQString::fromLatin1("http://kuro5hin.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("Prolinux"),
    TQString::fromLatin1("http://www.pl-forum.de/backend/pro-linux.rdf"),
    TQString::fromLatin1("http://www.prolinux.de/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("LinuxSecurity.com"),
    TQString::fromLatin1("http://www.linuxsecurity.com/linuxsecurity_hybrid.rdf"),
    TQString::fromLatin1("http://www.linuxsecurity.com/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("Linux Game Tome"),
    TQString::fromLatin1("http://happypenguin.org/html/news.rdf"),
    TQString::null,
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("Mozilla"),
    TQString::fromLatin1("http://www.mozilla.org/news.rdf"),
    TQString::fromLatin1("http://www.mozillazine.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("MozillaZine"),
    TQString::fromLatin1("http://www.mozillazine.org/contents.rdf"),
    TQString::fromLatin1("http://www.mozillazine.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("Daemon News"),
    TQString::fromLatin1("http://daily.daemonnews.org/ddn.rdf.php3"),
    TQString::null,
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("use Perl;"),
    TQString::fromLatin1("http://use.perl.org/useperl.rdf"),
    TQString::null,
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("Root prompt"),
    TQString::fromLatin1("http://www.rootprompt.org/rss/"),
    TQString::fromLatin1("http://www.rootprompt.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("SecurityFocus"),
    TQString::fromLatin1("http://www.securityfocus.com/topnews-rdf.html"),
    TQString::fromLatin1("http://www.securityfocus.com/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("Arstechnica"),
    TQString::fromLatin1("http://arstechnica.com/etc/rdf/ars.rdf"),
    TQString::fromLatin1("http://arstechnica.com/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("amiga-news.de - deutschsprachige Amiga Nachrichten"),
    TQString::fromLatin1("http://www.amiga-news.de/de/backends/news/index.rss"),
    TQString::fromLatin1("http://www.amiga-news.de/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("amiga-news.de - english Amiga news"),
    TQString::fromLatin1("http://www.amiga-news.de/en/backends/news/index.rss"),
    TQString::fromLatin1("http://www.amiga-news.de/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("FreshPorts - the place for ports"),
    TQString::fromLatin1("http://www.freshports.org/news.php3"),
    TQString::fromLatin1("http://www.freshports.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("zez.org - about code "),
    TQString::fromLatin1("http://zez.org/article/rssheadlines"),
    TQString::null,
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("BSDatwork.com"),
    TQString::fromLatin1("http://BSDatwork.com/backend.php"),
    TQString::fromLatin1("http://BSDatwork.com/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("FreshSource - the place for source"),
    TQString::fromLatin1("http://www.freshsource.org/news.php"),
    TQString::fromLatin1("http://www.freshsource.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    TQString::fromLatin1("The FreeBSD Diary"),
    TQString::fromLatin1("http://www.freebsddiary.org/news.php"),
    TQString::fromLatin1("http://www.freebsddiary.org/favicon.ico"),
    NewsSourceData::Computers ),
  // Miscellaneous ------
    NewsSourceData(
    TQString::fromLatin1("tagesschau.de"),
    TQString::fromLatin1("http://www.tagesschau.de/newsticker.rdf"),
    TQString::fromLatin1("http://www.tagesschau.de/favicon.ico"),
    NewsSourceData::Misc ),
    NewsSourceData(
    TQString::fromLatin1("CNN Top Stories"),
    TQString::fromLatin1("http://rss.cnn.com/rss/cnn_topstories.rss"),
    TQString::fromLatin1("http://www.cnn.com/favicon.ico"),
    NewsSourceData::Misc ),
    /*feed URL changed*/
    NewsSourceData(
    TQString::fromLatin1("HotWired"),
    TQString::fromLatin1("http://www.wired.com/news/feeds/rss2/0,2610,,00.xml"),
    TQString::fromLatin1("http://www.hotwired.com/favicon.ico"),
    NewsSourceData::Misc ),
    NewsSourceData(
    TQString::fromLatin1("The Register"),
    TQString::fromLatin1("http://www.theregister.co.uk/headlines.rss"),
    TQString::fromLatin1("http://www.theregister.co.uk/favicon.ico"),
    NewsSourceData::Misc ),
    NewsSourceData(
    TQString::fromLatin1( "Christian Science Monitor" ),
    TQString::fromLatin1( "http://www.csmonitor.com/rss/csm.rss"),
    TQString::fromLatin1( "http://www.csmonitor.com/favicon.ico"),
    NewsSourceData::Misc ),
  // Recreation
   // Society
    NewsSourceData(
    TQString::fromLatin1("nippon.it"),
    TQString::fromLatin1("http://www.nippon.it/backend.it.php"),
    TQString::fromLatin1("http://www.nippon.it/favicon.ico"),
    NewsSourceData::Society ),
    NewsSourceData(
    TQString::fromLatin1( "gflash" ),
    TQString::fromLatin1( "http://www.gflash.de/backend.php"),
    TQString::fromLatin1( "http://www.gflash.de/favicon.ico"),
    NewsSourceData::Society ),
    NewsSourceData(
    TQString::fromLatin1( "Quintessenz" ),
    TQString::fromLatin1( "http://quintessenz.at/cgi-bin/rdf"),
    TQString::fromLatin1( "http://quintessenz.at/favicon.ico"),
    NewsSourceData::Society )
};

#endif
