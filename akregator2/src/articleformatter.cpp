/*
    This file is part of Akregator2.

    Copyright (C) 2006 Frank Osterfeld <osterfeld@kde.org>

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

#include "akregator2config.h"
#include "articleformatter.h"
#include "utils.h"

#include <Akonadi/Collection>
#include <Akonadi/CollectionStatistics>

#include <krss/feedcollection.h>
#include <krss/enclosure.h>
#include <krss/item.h>

#include <KDateTime>
#include <KGlobal>
#include <KLocale>

#include <QApplication>
#include <QPaintDevice>
#include <QPalette>
#include <QString>

#include <QTextDocument> //Qt::escape
using namespace boost;
using namespace Akregator2;
using namespace Akonadi;
using namespace KRss;

namespace {
    static QString formatEnclosure( const KRss::Enclosure& enclosure )
    {
        if ( enclosure.isNull() )
            return QString();

        const QString title = !enclosure.title().isEmpty() ? enclosure.url() : enclosure.url();
        const uint length = enclosure.length();
        const QString type = enclosure.type();
        QString inf;
        if ( !type.isEmpty() && length > 0 )
            inf = i18n( "(%1, %2)", type, KGlobal::locale()->formatByteSize( length ) );
        else if ( !type.isNull() )
            inf = type;
        else if ( length > 0 )
            inf = KGlobal::locale()->formatByteSize( length );
        QString str = i18n( "<a href=\"%1\">%2</a> %3", enclosure.url(), title, inf );
        return str;
    }

    static QString formatEnclosures( const QList<KRss::Enclosure>& enclosures ) {
        QStringList list;
        Q_FOREACH( const KRss::Enclosure& i, enclosures )
            list.append( formatEnclosure( i ) );
        return list.join( QLatin1String("<br/>") );
    }
}

static QString imageLink( const Akonadi::Collection& c ) {
    KRss::FeedCollection fc( c );
    if ( fc.imageUrl().isEmpty() )
        return QString();

    const QString imageTag = QString::fromLatin1("<img class=\"headimage\" alt=\"%2\" src=\"%3\"/>").arg( Qt::escape( fc.imageTitle() ), Qt::escape( fc.imageUrl() ) );
    const QString linkUrl = !fc.imageLink().isEmpty() ? fc.imageLink() : fc.htmlUrl();
    if ( linkUrl.isEmpty() )
        return imageTag;
    else
        return QString::fromLatin1("<a href=\"%1\">%2</a>").arg( Qt::escape( linkUrl ), imageTag );
}

class ArticleFormatter::Private
{
    public:
        explicit Private( QPaintDevice* device_ );
        QPaintDevice* device;
        class SummaryVisitor;
};

ArticleFormatter::Private::Private( QPaintDevice* device_ ) : device( device_ )
{
}

ArticleFormatter::ArticleFormatter( QPaintDevice* device ) : d( new Private( device ) )
{
}

ArticleFormatter::~ArticleFormatter()
{
    delete d;
}

void ArticleFormatter::setPaintDevice(QPaintDevice* device)
{
    d->device = device;
}

int ArticleFormatter::pointsToPixel(int pointSize) const
{
    return ( pointSize * d->device->logicalDpiY() + 36 ) / 72 ;
}

static QString formatFolderSummary( const Collection& c, int unread ) {
    const QString title = FeedCollection( c ).title();
    QString text = QString::fromLatin1("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");
    text += QString::fromLatin1("<div class=\"headertitle\" dir=\"%1\">%2").arg(Utils::directionOf(Utils::stripTags(title)), title);

    if(unread == 0)
        text += i18n(" (no unread articles)");
    else
        text += i18np(" (1 unread article)", " (%1 unread articles)", unread);
    text += QString("</div>\n");
    text += "</div>\n"; // /headerbox
    return text;
}

static QString formatFeedSummary( const FeedCollection& feed, int unread ) {

    QString text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");

    text += QString("<div class=\"headertitle\" dir=\"%1\">").arg(Utils::directionOf(Utils::stripTags(feed.title())));
    text += feed.title();

    if(unread == 0)
        text += i18n(" (no unread articles)");
    else
        text += i18np(" (1 unread article)", " (%1 unread articles)", unread);

    text += "</div>\n"; // headertitle
    text += "</div>\n"; // /headerbox
    text += imageLink( feed );

    text += "<div class=\"body\">";


    if( !feed.description().isEmpty() )
    {
        text += QString("<div dir=\"%1\">").arg(Utils::stripTags(Utils::directionOf(feed.description())));
        text += i18n("<b>Description:</b> %1<br /><br />", feed.description());
        text += "</div>\n"; // /description
    }

    if ( !feed.htmlUrl().isEmpty() )
    {
        text += QString("<div dir=\"%1\">").arg(Utils::directionOf(feed.htmlUrl()));
        text += i18n("<b>Homepage:</b> <a href=\"%1\">%2</a>", feed.htmlUrl(), feed.htmlUrl());
        text += "</div>\n"; // / link
    }

//text += i18n("<b>Unread articles:</b> %1").arg(node->unread());
    text += "</div>"; // /body
    return text;
}

static QString formatCollectionSummary( const Collection& c, int unread ) {
    FeedCollection fc( c );
    if ( fc.isFolder() )
        return formatFolderSummary( c, unread );
    else
        return formatFeedSummary( fc, unread );
}

QString DefaultNormalViewFormatter::formatItem( const Akonadi::Item& aitem, const Akonadi::Collection& storageCollection, IconOption icon) const
{
    const KRss::Item item = aitem.payload<KRss::Item>();

    QString text;
    text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");
    const QString enc = formatEnclosures( item.enclosures() );

    const KUrl link( item.link() );
    if (!item.title().isEmpty())
    {
        text += QString("<div class=\"headertitle\" dir=\"%1\">\n").arg(Utils::directionOf(Utils::stripTags(item.title())));
        if (link.isValid())
            text += "<a href=\""+link.url()+"\">";
        text += item.title().replace('<', "&lt;").replace('>', "&gt;"); // TODO: better leave things escaped in the parser
        if (link.isValid())
            text += "</a>";
        text += "</div>\n";
    }
    if (item.dateUpdated().isValid())
    {
        text += QString("<span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Date")));
        text += QString ("%1:").arg(i18n("Date"));
        text += "</span><span class=\"headertext\">";
        text += KGlobal::locale()->formatDateTime(item.dateUpdated(), KLocale::FancyLongDate) +"</span>\n"; // TODO: might need RTL?
    }

    const QString author = item.authorsAsHtml();
    if (!author.isEmpty())
    {
        text += QString("<br/><span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Author")));
        text += QString ("%1:").arg(i18n("Author"));
        text += "</span><span class=\"headertext\">";
        text += author+"</span>\n"; // TODO: might need RTL?
    }

    if (!enc.isEmpty())
    {
        text += QString("<br/><span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Enclosure")));
        text += QString ("%1:").arg(i18n("Enclosure"));
        text += "</span><span class=\"headertext\">";
        text += enc+"</span>\n"; // TODO: might need RTL?
    }

    text += "</div>\n"; // end headerbox

    if (icon == ShowIcon )
    {
        KRss::FeedCollection fc( storageCollection );
        text += imageLink( fc );
    }

    const QString content = item.contentWithDescriptionAsFallback();

    if (!content.isEmpty())
    {
        text += QString("<div dir=\"%1\">").arg(Utils::directionOf(Utils::stripTags(content)) );
        text += "<span class=\"content\">"+content+"</span>";
        text += "</div>";
    }

    text += "<div class=\"body\">";

    if (KUrl( item.commentsLink() ).isValid())
    {
        text += "<a class=\"contentlink\" href=\"";
        text += KUrl( item.commentsLink() ).url();
        text += "\">" + i18n( "Comments");
        if ( item.commentsCount() > 0 )
            text += " ("+ QString::number(item.commentsCount()) +')';
        text += "</a>";
    }

    if (!enc.isEmpty())
        text += QString("<p><em>%1</em> %2</p>").arg(i18n("Enclosure:")).arg(enc);

    if (link.isValid())
        text += QString( "<p><a class=\"contentlink\" href=\"%1\">%2</a></p>" ).arg( link.url(), i18n( "Complete Story" ) );

    text += "</div>";

    return text;
}

QString DefaultNormalViewFormatter::getCss() const
{
    const QPalette & pal = QApplication::palette();

    // from kmail::headerstyle.cpp
    QString css = QString (
            "<style type=\"text/css\">\n"
            "@media screen, print {"
            "body {\n"
            "  font-family: \"%1\" ! important;\n"
            "  font-size: %2 ! important;\n"
            "  color: %3 ! important;\n"
            "  background: %4 ! important;\n"
            "}\n\n")
            .arg( Settings::standardFont(),
                  QString::number(pointsToPixel(Settings::mediumFontSize()))+"px",
                  pal.color( QPalette::Text ).name(),
                  pal.color( QPalette::Base ).name() );
    css += QString(
            "a {\n"
            + QString("  color: %1 ! important;\n")
            + QString(!Settings::underlineLinks() ? " text-decoration: none ! important;\n" : "")
            +       "}\n\n"
            +".headerbox {\n"
            +"  background: %2 ! important;\n"
            +"  color: %3 ! important;\n"
            +"  border:1px solid #000;\n"
            +"  margin-bottom: 10pt;\n"
            +        "}\n\n")
            .arg( pal.color( QPalette::Link ).name(),
                  pal.color( QPalette::Background ).name(),
                  pal.color( QPalette::Text ).name() );
    css += QString(".headertitle a:link { color: %1 ! important;\n text-decoration: none ! important;\n }\n"
            ".headertitle a:visited { color: %1 ! important;\n text-decoration: none ! important;\n }\n"
            ".headertitle a:hover{ color: %1 ! important;\n text-decoration: none ! important;\n }\n"
            ".headertitle a:active { color: %1 ! important;\n  text-decoration: none ! important;\n }\n" )
            .arg( pal.color( QPalette::HighlightedText ).name() );
    css += QString(
            ".headertitle {\n"
            "  background: %1 ! important;\n"
            "  padding:2px;\n"
            "  color: %2 ! important;\n"
            "  font-weight: bold;\n"
            "  text-decoration: none ! important;\n"
            "}\n\n"
            ".header {\n"
            "  font-weight: bold;\n"
            "  padding:2px;\n"
            "  margin-right: 5px;\n"
            "  text-decoration: none ! important;\n"
            "}\n\n"
            ".headertext a {\n"
            "  text-decoration: none ! important;\n"
            "}\n\n"
            ".headimage {\n"
            "  float: right;\n"
            "  margin-left: 5px;\n"
            "}\n\n").arg(
                    pal.color( QPalette::Highlight ).name(),
                    pal.color( QPalette::HighlightedText ).name() );

    css += QString(
            "body { clear: none; }\n\n"
            ".content {\n"
            "  display: block;\n"
            "  margin-bottom: 6px;\n"
            "}\n\n"
    // these rules make sure that there is no leading space between the header and the first of the text
            ".content > P:first-child {\n margin-top: 1px; }\n"
            ".content > DIV:first-child {\n margin-top: 1px; }\n"
            ".content > BR:first-child {\n display: none;  }\n"
    //".contentlink {\n display: block; }\n"
            "}\n\n" // @media screen, print
    // Why did we need that, bug #108187?
    //"@media screen { body { overflow: auto; } }\n"
            "\n\n");

    return css;
}

DefaultCombinedViewFormatter::DefaultCombinedViewFormatter( QPaintDevice* device )
    : ArticleFormatter( device )
{
}

DefaultNormalViewFormatter::DefaultNormalViewFormatter( QPaintDevice* device )
    : ArticleFormatter( device )
{
}

DefaultNormalViewFormatter::~DefaultNormalViewFormatter()
{
}

QString DefaultCombinedViewFormatter::formatItem( const Akonadi::Item& aitem, const Akonadi::Collection& storageCollection, IconOption icon ) const
{
    const KRss::Item item = aitem.payload<KRss::Item>();

    QString text;
    const QString enc = formatEnclosures( item.enclosures() );
    text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");

    const KUrl link( item.link() );
    if (!item.title().isEmpty())
    {
        text += QString("<div class=\"headertitle\" dir=\"%1\">\n").arg(Utils::directionOf(Utils::stripTags(item.title())));
        if (link.isValid())
            text += "<a href=\""+link.url()+"\">";
        text += item.title().replace('<', "&lt;").replace('>', "&gt;"); // TODO: better leave things escaped in the parser
        if (link.isValid())
            text += "</a>";
        text += "</div>\n";
    }
    if (item.datePublished().isValid())
    {
        text += QString("<span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Date")));
        text += QString ("%1:").arg(i18n("Date"));
        text += "</span><span class=\"headertext\">";
        text += KGlobal::locale()->formatDateTime(item.datePublished(), KLocale::FancyLongDate) + "</span>\n"; // TODO: might need RTL?
    }

    const QString author = item.authorsAsHtml();
    if (!author.isEmpty())
    {
        text += QString("<br/><span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Author")));
        text += QString ("%1:").arg(i18n("Author"));
        text += "</span><span class=\"headertext\">";
        text += author+"</span>\n"; // TODO: might need RTL?
    }
    if (!enc.isEmpty())
    {
        text += QString("<br/><span class=\"header\" dir=\"%1\">").arg(Utils::directionOf(i18n("Enclosure")));
        text += QString ("%1:").arg(i18n("Enclosure"));
        text += "</span><span class=\"headertext\">";
        text += enc+"</span>\n"; // TODO: might need RTL?
    }

    text += "</div>\n"; // end headerbox

    if (icon == ShowIcon )
    {
        KRss::FeedCollection fc( storageCollection );
        text += imageLink( fc );
    }

    const QString content = item.contentWithDescriptionAsFallback();
    if (!content.isEmpty())
    {
        text += QString("<div dir=\"%1\">").arg(Utils::directionOf(Utils::stripTags(content)) );
        text += "<span class=\"content\">"+content+"</span>";
        text += "</div>";
    }

    text += "<div class=\"body\">";

    if ( KUrl(item.commentsLink()).isValid())
    {
        text += "<a class=\"contentlink\" href=\"";
        text += KUrl( item.commentsLink() ).url();
        text += "\">" + i18n( "Comments");
        if ( item.commentsCount() > 0 )
            text += " ("+ QString::number(item.commentsCount()) +')';
        text += "</a>";
    }


    if (!enc.isEmpty())
        text += QString("<p><em>%1</em> %2</p>").arg(i18n("Enclosure:")).arg(enc);

    if (link.isValid())
        text += QString( "<p><a class=\"contentlink\" href=\"%1\">%2</a></p>" ).arg( link.url(), i18n( "Complete Story" ) );

    text += "</div>";
    //kDebug() << text;
    return text;
}

QString DefaultCombinedViewFormatter::getCss() const
{
    const QPalette &pal = QApplication::palette();

    // from kmail::headerstyle.cpp
    QString css = QString (
            "<style type=\"text/css\">\n"
            "@media screen, print {"
            "body {\n"
            "  font-family: \"%1\" ! important;\n"
            "  font-size: %2 ! important;\n"
            "  color: %3 ! important;\n"
            "  background: %4 ! important;\n"
            "}\n\n").arg(Settings::standardFont(),
                         QString::number(pointsToPixel(Settings::mediumFontSize()))+"px",
                         pal.color( QPalette::Text ).name(),
                         pal.color( QPalette::Base ).name() );
    css += QString(
            "a {\n"
            + QString("  color: %1 ! important;\n")
            + QString(!Settings::underlineLinks() ? " text-decoration: none ! important;\n" : "")
            +       "}\n\n"
            +".headerbox {\n"
            +"  background: %2 ! important;\n"
            +"  color: %3 ! important;\n"
            +"  border:1px solid #000;\n"
            +"  margin-bottom: 10pt;\n"
//    +"  width: 99%;\n"
            +        "}\n\n")
            .arg( pal.color( QPalette::Link ).name(),
                  pal.color( QPalette::Background ).name(),
                  pal.color( QPalette::Text ).name() );

    css += QString(".headertitle a:link { color: %1  ! important; text-decoration: none ! important;\n }\n"
            ".headertitle a:visited { color: %1 ! important; text-decoration: none ! important;\n }\n"
            ".headertitle a:hover{ color: %1 ! important; text-decoration: none ! important;\n }\n"
            ".headertitle a:active { color: %1 ! important; text-decoration: none ! important;\n }\n")
            .arg( pal.color( QPalette::HighlightedText ).name() );
    css += QString(
            ".headertitle {\n"
            "  background: %1 ! important;\n"
            "  padding:2px;\n"
            "  color: %2 ! important;\n"
            "  font-weight: bold;\n"
            "  text-decoration: none ! important;\n"
            "}\n\n"
            ".header {\n"
            "  font-weight: bold;\n"
            "  padding:2px;\n"
            "  margin-right: 5px;\n"
            "  text-decoration: none ! important;\n"
            "}\n\n"
            ".headertext {\n"
            "  text-decoration: none ! important;\n"
            "}\n\n"
            ".headimage {\n"
            "  float: right;\n"
            "  margin-left: 5px;\n"
            "}\n\n").arg( pal.color( QPalette::Highlight ).name(),
                          pal.color( QPalette::HighlightedText ).name() );

    css += QString(
            "body { clear: none; }\n\n"
            ".content {\n"
            "  display: block;\n"
            "  margin-bottom: 6px;\n"
            "}\n\n"
    // these rules make sure that there is no leading space between the header and the first of the text
            ".content > P:first-child {\n margin-top: 1px; }\n"
            ".content > DIV:first-child {\n margin-top: 1px; }\n"
            ".content > BR:first-child {\n display: none;  }\n"
    //".contentlink {\n display: block; }\n"
            "}\n\n" // @media screen, print
    // Why did we need that, bug #108187?
    //"@media screen { body { overflow: auto; } }\n"
            "\n\n");

    return css;
}

QString DefaultNormalViewFormatter::formatSummary( const Akonadi::Collection& c, int unread ) const
{
    return formatCollectionSummary( c, unread );
}

QString DefaultCombinedViewFormatter::formatSummary( const Akonadi::Collection&, int ) const
{
    return QString();
}
