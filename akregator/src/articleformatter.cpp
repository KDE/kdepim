/*
    This file is part of Akregator.

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

#include "akregatorconfig.h"
#include "articleformatter.h"
#include "article.h"
#include "feed.h"
#include "folder.h"
#include "treenode.h"
#include "treenodevisitor.h"
#include "utils.h"

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <KDebug>

#include <QApplication>
#include <QPaintDevice>
#include <QPalette>
#include <QString>

// Grantlee
#include <grantlee/template.h>
#include <grantlee/context.h>
#include <grantlee/engine.h>

#include "grantlee_paths.h"

using namespace boost;
using namespace Syndication;
using namespace Akregator;

namespace {
    QString formatEnclosure( const Enclosure& enclosure )
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
    mEngine = new Grantlee::Engine();
    Grantlee::FileSystemTemplateLoader::Ptr loader(new Grantlee::FileSystemTemplateLoader());

    mEngine->addTemplateLoader( loader );
    loader->setTemplateDirs( QStringList() << KStandardDirs::locate("data","akregator/themes/") );

    mEngine->setPluginPaths( QStringList() << GRANTLEE_PLUGIN_PATH );
    mThemeName = Settings::self()->theme();
}

ArticleFormatter::~ArticleFormatter()
{
    Settings::self()->setTheme( mThemeName );
    
    delete d;
    delete mEngine;
}

void ArticleFormatter::setPaintDevice(QPaintDevice* device)
{
    d->device = device;
}

int ArticleFormatter::pointsToPixel(int pointSize) const
{
    return ( pointSize * d->device->logicalDpiY() + 36 ) / 72 ;
}

void ArticleFormatter::setThemeName( const QString &themeName )
{
    mThemeName = themeName;
}

QString ArticleFormatter::renderTheme( const QString &themeName, const Article& article, IconOption icon, const QString &formatter ) const {

    Grantlee::Template t = mEngine->loadByName( themeName + "/default.html" );
    QVariantHash data;

    QString dir = QApplication::isRightToLeft() ? "rtl" : "ltr";
    const QString enc = formatEnclosure( *article.enclosure() );

    data.insert( QLatin1String( "dir" ) , dir );

    if ( formatter == "Normal" )
        data.insert( QLatin1String( "NormalView" ) , "" );

    const QString strippedTitle = Utils::stripTags( article.title() );
    if (!strippedTitle.isEmpty())
    {
        data.insert( QLatin1String( "directionOfstrippedTitle" ) , Utils::directionOf(strippedTitle) );

        if (article.link().isValid()) {
          data.insert( QLatin1String( "article.link" ) , article.link().url() );
          data.insert( QLatin1String( "strippedTitle" ) , strippedTitle );
        }
    }

    if (article.pubDate().isValid())
    {
        data.insert( QLatin1String( "directionOfDate" ) , Utils::directionOf(i18n("Date")) );
        data.insert( QLatin1String( "date" ) , KGlobal::locale()->formatDateTime(article.pubDate(), KLocale::FancyLongDate) );
    }

    const QString author = article.authorAsHtml();
    if (!author.isEmpty())
    {
        data.insert( QLatin1String( "directionOfAuthor" ) , Utils::directionOf(i18n("Author")) );
        data.insert( QLatin1String( "author" ) , author );
    }

    if (!enc.isEmpty())
    {
        data.insert( QLatin1String( "directionOfEnclosure" ) , Utils::directionOf(i18n("Enclosure")) );
        data.insert( QLatin1String( "enc" ) , enc );
    }

    if (icon == ShowIcon && article.feed() && !article.feed()->image().isNull())
    {
        const Feed* feed = article.feed();
        QString file = Utils::fileNameForUrl(feed->xmlUrl());
        KUrl m_imageDir;
        KUrl u(m_imageDir);
        u.setFileName(file);

        data.insert( QLatin1String( "imageUrl" ) , feed->htmlUrl() );
        data.insert( QLatin1String( "imageName" ) , u.url() );
    }

    const QString content = article.content( Article::DescriptionAsFallback );
    if (!content.isEmpty())
    {
        data.insert( QLatin1String( "directionOfContent" ) , Utils::directionOf(Utils::stripTags(content)) );
        data.insert( QLatin1String( "content" ) , content );
    }

    if (article.commentsLink().isValid())
    {
        data.insert( QLatin1String( "article.commentsLink" ) , article.commentsLink().url() );

        if (article.comments())
        {
            data.insert( QLatin1String( "article.comments" ) , QString::number(article.comments()) );
        }
    }

    if (article.link().isValid() || (article.guidIsPermaLink() && KUrl(article.guid()).isValid()))
    {
        // in case link isn't valid, fall back to the guid permaLink.
        if (article.link().isValid())
        {
            data.insert( QLatin1String( "article.linkComplete" ) , article.link().url() );
        }
        else
        {
            data.insert( QLatin1String( "article.linkComplete" ) , article.guid() );

        }
    }

    Grantlee::Context c( data );
    c.setRelativeMediaPath("images/");
    QString viewStr = t->render( &c );

    data.insert( QLatin1String( "font-family" ) , Settings::standardFont() );
    data.insert( QLatin1String( "font-size" ) , QString::number(pointsToPixel(Settings::mediumFontSize()))+"px" );


    return viewStr;
}  
  

class DefaultNormalViewFormatter::SummaryVisitor : public TreeNodeVisitor
{
    public:
        SummaryVisitor(DefaultNormalViewFormatter* p) : parent(p) {}
        virtual bool visitFeed(Feed* node)
        {
            text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");
            const QString strippedTitle = Utils::stripTags(node->title());
            text += QString("<div class=\"headertitle\" dir=\"%1\">").arg(Utils::directionOf(strippedTitle));
            text += strippedTitle;
            if(node->unread() == 0)
                text += i18n(" (no unread articles)");
            else
                text += i18np(" (1 unread article)", " (%1 unread articles)", node->unread());
            text += "</div>\n"; // headertitle
            text += "</div>\n"; // /headerbox

            if (!node->image().isNull()) // image
            {
                text += QString("<div class=\"body\">");
                QString file = Utils::fileNameForUrl(node->xmlUrl());
                KUrl u(parent->m_imageDir);
                u.setFileName(file);
                text += QString("<a href=\"%1\"><img class=\"headimage\" src=\"%2.png\"></a>\n").arg(node->htmlUrl(), u.url());
            }
            else text += "<div class=\"body\">";


            if( !node->description().isEmpty() )
            {
                text += QString("<div dir=\"%1\">").arg(Utils::stripTags(Utils::directionOf(node->description())));
                text += i18n("<b>Description:</b> %1<br /><br />", node->description());
                text += "</div>\n"; // /description
            }

            if ( !node->htmlUrl().isEmpty() )
            {
                text += QString("<div dir=\"%1\">").arg(Utils::directionOf(node->htmlUrl()));
                text += i18n("<b>Homepage:</b> <a href=\"%1\">%2</a>", node->htmlUrl(), node->htmlUrl());
                text += "</div>\n"; // / link
            }

        //text += i18n("<b>Unread articles:</b> %1").arg(node->unread());
            text += "</div>"; // /body

            return true;
        }

        virtual bool visitFolder(Folder* node)
        {
            text = QString("<div class=\"headerbox\" dir=\"%1\">\n").arg(QApplication::isRightToLeft() ? "rtl" : "ltr");
            text += QString("<div class=\"headertitle\" dir=\"%1\">%2").arg(Utils::directionOf(Utils::stripTags(node->title())), node->title());
            if(node->unread() == 0)
                text += i18n(" (no unread articles)");
            else
                text += i18np(" (1 unread article)", " (%1 unread articles)", node->unread());
            text += QString("</div>\n");
            text += "</div>\n"; // /headerbox

            return true;
        }

        QString formatSummary(TreeNode* node)
        {
            text.clear();
            visit(node);
            return text;
        }

        QString text;
        DefaultNormalViewFormatter* parent;
};

QString DefaultNormalViewFormatter::formatArticle(const Article& article, IconOption icon) const
{
    QString formatter = "Normal";
    return renderTheme( mThemeName, article, icon, formatter );
}

DefaultCombinedViewFormatter::DefaultCombinedViewFormatter(const KUrl& imageDir, QPaintDevice* device ) : ArticleFormatter( device ), m_imageDir(imageDir)
{
}

DefaultNormalViewFormatter::DefaultNormalViewFormatter(const KUrl& imageDir, QPaintDevice* device )
    : ArticleFormatter( device ),
    m_imageDir( imageDir ),
    m_summaryVisitor( new SummaryVisitor( this ) )
{
}

DefaultNormalViewFormatter::~DefaultNormalViewFormatter()
{
    delete m_summaryVisitor;
}

QString DefaultCombinedViewFormatter::formatArticle(const Article& article, IconOption icon) const
{
    QString formatter =  "Combined";
    return renderTheme( mThemeName, article, icon, formatter );
}

QString DefaultNormalViewFormatter::formatSummary(TreeNode* node) const
{
    return m_summaryVisitor->formatSummary(node);
}

QString DefaultCombinedViewFormatter::formatSummary(TreeNode*) const
{
    return QString();
}