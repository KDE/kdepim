/*
    This file is part of Akregator2.

    Copyright (C) 2004 Teemu Rytilahti <tpr@d5k.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

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

#include "articleviewer.h"
#include "akregator2config.h"
#include "aboutdata.h"
#include "actionmanager.h"
#include "actions.h"
#include "articleformatter.h"
#include "articlematcher.h"
#include "utils.h"
#include "openurlrequest.h"

#include <kpimutils/kfileio.h>

#include <krss/item.h>
#include <krss/feeditemmodel.h>
#include <krss/feedcollection.h>

#include <QAbstractItemModel>


#include <Akonadi/Collection>
#include <Akonadi/ItemFetchScope>

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <khtmlview.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kshell.h>
#include <kstandarddirs.h>
#include <kstandardaction.h>
#include <ktoolinvocation.h>
#include <kurl.h>
#include <kglobalsettings.h>
#include <kparts/browserextension.h>
#include <kparts/browserrun.h>
#include <KDateTime>
#include "kdepim-version.h"

#include <QClipboard>
#include <QKeySequence>
#include <QGridLayout>

#include <boost/bind.hpp>

#include <memory>
#include <cassert>

using namespace boost;
using namespace Akregator2;
using namespace Akregator2::Filters;
using namespace KRss;

namespace Akregator2 {

ArticleViewer::ArticleViewer(QWidget *parent)
    : QWidget(parent),
      m_url(0),
      m_htmlFooter(),
      m_currentText(),
      m_viewMode(NormalView),
      m_part( new ArticleViewerPart( this ) ),
      m_model( 0 ),
      m_normalViewFormatter( new DefaultNormalViewFormatter( m_part->view() ) ),
      m_combinedViewFormatter( new DefaultCombinedViewFormatter( m_part->view() ) )
{
    QGridLayout* layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->addWidget(m_part->widget(), 0, 0);

    setFocusProxy( m_part->widget() );

    m_part->setFontScaleFactor(100);
    m_part->setZoomFactor(100);
    m_part->setJScriptEnabled(false);
    m_part->setJavaEnabled(false);
    m_part->setMetaRefreshEnabled(false);
    m_part->setPluginsEnabled(false);
    m_part->setDNDEnabled(true);
    m_part->setAutoloadImages(true);
    m_part->setStatusMessagesEnabled(false);
    m_part->view()->setAttribute(Qt::WA_InputMethodEnabled, true); //workaround to fix 216878
    m_part->view()->setFrameStyle( QFrame::StyledPanel|QFrame::Sunken );

    // change the cursor when loading stuff...
    connect( m_part, SIGNAL(started(KIO::Job*)),
             this, SLOT(slotStarted(KIO::Job*)));
    connect( m_part, SIGNAL(completed()),
             this, SLOT(slotCompleted()));

    KParts::BrowserExtension* ext = m_part->browserExtension();
    connect(ext, SIGNAL(popupMenu(QPoint,KUrl,mode_t,KParts::OpenUrlArguments,KParts::BrowserArguments,KParts::BrowserExtension::PopupFlags,KParts::BrowserExtension::ActionGroupMap)),
             this, SLOT(slotPopupMenu(QPoint,KUrl,mode_t,KParts::OpenUrlArguments,KParts::BrowserArguments,KParts::BrowserExtension::PopupFlags))); // ActionGroupMap argument removed, unused by slot

    connect( ext, SIGNAL(openUrlRequestDelayed(KUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)),
             this, SLOT(slotOpenUrlRequestDelayed(KUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)) );

    connect(ext, SIGNAL(createNewWindow(KUrl,
            KParts::OpenUrlArguments,
            KParts::BrowserArguments,
            KParts::WindowArgs,
            KParts::ReadOnlyPart**)),
            this, SLOT(slotCreateNewWindow(KUrl,
                         KParts::OpenUrlArguments,
                         KParts::BrowserArguments,
                         KParts::WindowArgs,
                         KParts::ReadOnlyPart**)));

    KAction* action = 0;
    action = KStandardAction::print(this, SLOT(slotPrint()), m_part->actionCollection());
    m_part->actionCollection()->addAction("viewer_print", action);

    action = KStandardAction::copy(this, SLOT(slotCopy()), m_part->actionCollection());
    m_part->actionCollection()->addAction("viewer_copy", action);

    action = m_part->actionCollection()->addAction("copylinkaddress");
    action->setText(i18n("Copy &Link Address"));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotCopyLinkAddress()));

    action = m_part->actionCollection()->addAction("savelinkas");
    action->setText(i18n("&Save Link As..."));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotSaveLinkAs()));

    action = m_part->actionCollection()->addAction("articleviewer_scroll_up");
    action->setText(i18n("&Scroll Up"));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotScrollUp()));
    action->setShortcuts(KShortcut( "Up" ));

    action = m_part->actionCollection()->addAction("articleviewer_scroll_down");
    action->setText(i18n("&Scroll Down"));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotScrollDown()));
    action->setShortcuts(KShortcut( "Down" ));

    updateCss();

    connect(this, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));

    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SLOT(slotPaletteOrFontChanged()) );
    connect(KGlobalSettings::self(), SIGNAL(kdisplayFontChanged()), this, SLOT(slotPaletteOrFontChanged()) );

    m_htmlFooter = "</body></html>";

    m_updateTimer.setInterval( 50 );
    m_updateTimer.setSingleShot( true );
    connect( &m_updateTimer, SIGNAL(timeout()), this, SLOT(slotUpdateCombinedView()) );
}

ArticleViewer::~ArticleViewer()
{
}

KParts::ReadOnlyPart* ArticleViewer::part() const
{
    return m_part;
}

int ArticleViewer::pointsToPixel(int pointSize) const
{
    return ( pointSize * m_part->view()->logicalDpiY() + 36 ) / 72 ;
}

void ArticleViewer::slotOpenUrlRequestDelayed(const KUrl& url, const KParts::OpenUrlArguments& args, const KParts::BrowserArguments& browserArgs)
{
    OpenUrlRequest req(url);
    req.setArgs(args);
    req.setBrowserArgs(browserArgs);
    if (req.options() == OpenUrlRequest::None)		// no explicit new window,
        req.setOptions(OpenUrlRequest::NewTab);		// so must open new tab

    if (m_part->button() == Qt::LeftButton)
    {
        switch (Settings::lMBBehaviour())
        {
            case Settings::EnumLMBBehaviour::OpenInExternalBrowser:
                req.setOptions(OpenUrlRequest::ExternalBrowser);
                break;
            case Settings::EnumLMBBehaviour::OpenInBackground:
                req.setOpenInBackground(true);
                break;
            default:
                break;
        }
    }
    else if (m_part->button() == Qt::MidButton)
    {
        switch (Settings::mMBBehaviour())
        {
            case Settings::EnumMMBBehaviour::OpenInExternalBrowser:
                req.setOptions(OpenUrlRequest::ExternalBrowser);
                break;
            case Settings::EnumMMBBehaviour::OpenInBackground:
                req.setOpenInBackground(true);
                break;
            default:
                break;
        }
    }

    emit signalOpenUrlRequest(req);
}

void ArticleViewer::slotCreateNewWindow(const KUrl& url,
                                       const KParts::OpenUrlArguments& args,
                                       const KParts::BrowserArguments& browserArgs,
                                       const KParts::WindowArgs& /*windowArgs*/,
                                       KParts::ReadOnlyPart** part)
{
    OpenUrlRequest req;
    req.setUrl(url);
    req.setArgs(args);
    req.setBrowserArgs(browserArgs);
    req.setOptions(OpenUrlRequest::NewTab);

    emit signalOpenUrlRequest(req);
    if ( part )
        *part = req.part();
}

void ArticleViewer::slotPopupMenu(const QPoint& p, const KUrl& kurl, mode_t, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&, KParts::BrowserExtension::PopupFlags kpf)
{
    const bool isLink = (kpf & KParts::BrowserExtension::ShowNavigationItems) == 0; // ## why not use kpf & IsLink ?
    const bool isSelection = (kpf & KParts::BrowserExtension::ShowTextSelectionItems) != 0;

    QString url = kurl.url();

    m_url = url;
    KMenu popup;

    if (isLink && !isSelection)
    {
        popup.addAction( createOpenLinkInNewTabAction( kurl, this, SLOT(slotOpenLinkInForegroundTab()), &popup ) );
        popup.addAction( createOpenLinkInExternalBrowserAction( kurl, this, SLOT(slotOpenLinkInBrowser()), &popup ) );
        popup.addSeparator();
        popup.addAction( m_part->action("savelinkas") );
        popup.addAction( m_part->action("copylinkaddress") );
    }
    else
    {
        if (isSelection)
        {
            popup.addAction( ActionManager::getInstance()->action("viewer_copy") );

            popup.addSeparator();
        }
        popup.addAction( ActionManager::getInstance()->action("viewer_print") );
       //KAction *ac = action("setEncoding");
       //if (ac)
       //     ac->plug(&popup);
        popup.addSeparator();
        popup.addAction( ActionManager::getInstance()->action("inc_font_sizes") );
        popup.addAction( ActionManager::getInstance()->action("dec_font_sizes") );

    }
    popup.exec(p);
}

// taken from KDevelop
void ArticleViewer::slotCopy()
{
    QString text = m_part->selectedText();
    text.replace( QChar( 0xa0 ), ' ' );
    QClipboard* const cb = QApplication::clipboard();
    assert( cb );
    cb->setText( text, QClipboard::Clipboard );
}

void ArticleViewer::slotCopyLinkAddress()
{
    if(m_url.isEmpty()) return;
    QClipboard *cb = QApplication::clipboard();
    cb->setText(m_url.prettyUrl(), QClipboard::Clipboard);
    // don't set url to selection as it's a no-no according to a fd.o spec
    // which spec? Nobody seems to care (tested Firefox (3.5.10) Konqueror,and KMail (4.2.3)), so I re-enable the following line unless someone gives
    // a good reason to remove it again (bug 183022) --Frank
    cb->setText(m_url.prettyUrl(), QClipboard::Selection);
}

void ArticleViewer::slotSelectionChanged()
{
    ActionManager::getInstance()->action("viewer_copy")->setEnabled(!m_part->selectedText().isEmpty());
}

void ArticleViewer::slotOpenLinkInternal()
{
    openUrl(m_url);
}

void ArticleViewer::slotOpenLinkInForegroundTab()
{
    OpenUrlRequest req(m_url);
    req.setOptions(OpenUrlRequest::NewTab);
    emit signalOpenUrlRequest(req);
}

void ArticleViewer::slotOpenLinkInBackgroundTab()
{
    OpenUrlRequest req(m_url);
    req.setOptions(OpenUrlRequest::NewTab);
    req.setOpenInBackground(true);
    emit signalOpenUrlRequest(req);
}

void ArticleViewer::slotOpenLinkInBrowser()
{
    OpenUrlRequest req(m_url);
    req.setOptions(OpenUrlRequest::ExternalBrowser);
    emit signalOpenUrlRequest(req);
}

void ArticleViewer::slotSaveLinkAs()
{
    KUrl tmp( m_url );

    if ( tmp.fileName(KUrl::ObeyTrailingSlash).isEmpty() )
        tmp.setFileName( "index.html" );
    KParts::BrowserRun::simpleSave(tmp, tmp.fileName());
}

void ArticleViewer::slotStarted(KIO::Job* job)
{
    m_part->widget()->setCursor( Qt::WaitCursor );
    emit started(job);
}

void ArticleViewer::slotCompleted()
{
    m_part->widget()->unsetCursor();
    emit completed();
}

void ArticleViewer::slotScrollUp()
{
    m_part->view()->scrollBy(0,-10);
}

void ArticleViewer::slotScrollDown()
{
    m_part->view()->scrollBy(0,10);
}

void ArticleViewer::slotZoomIn(int id)
{
    if (id != 0)
      return;	
    int zf = m_part->fontScaleFactor();
    if (zf < 100)
    {
        zf = zf - (zf % 20) + 20;
        m_part->setFontScaleFactor(zf);
    }
    else
    {
        zf = zf - (zf % 50) + 50;
        m_part->setFontScaleFactor(zf < 300 ? zf : 300);
    }
}

void ArticleViewer::slotZoomOut(int id)
{
    if (id != 0)
     return;

    int zf = m_part->fontScaleFactor();
    if (zf <= 100)
    {
        zf = zf - (zf % 20) - 20;
        m_part->setFontScaleFactor(zf > 20 ? zf : 20);
    }
    else
    {
        zf = zf - (zf % 50) - 50;
        m_part->setFontScaleFactor(zf);
    }
}

void ArticleViewer::slotSetZoomFactor(int percent)
{
    m_part->setFontScaleFactor(percent);
}

// some code taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void ArticleViewer::slotPrint( )
{
    m_part->view()->print();
}

void ArticleViewer::renderContent(const QString& text)
{
    m_part->closeUrl();
    m_currentText = text;
    beginWriting();
    //kDebug() << text;
    m_part->write(text);
    endWriting();
}

void ArticleViewer::beginWriting()
{
    QString head = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n <html><head><title>.</title>");

    if (m_viewMode == CombinedView)
        head += m_combinedModeCSS;
    else
        head += m_normalModeCSS;

    head += "</style></head><body>";
    m_part->view()->setContentsPos(0,0);

    //pass link to the KHTMLPart to make relative links work
    //add a bogus query item to distinguish from m_link
    //fixes the Complete Story link if the url has an anchor (e.g. #reqRSS) in it
    //See bug 177754

    KUrl url(m_link);
    url.addQueryItem("akregator2PreviewMode", "true");
    m_part->begin(url);
    m_part->write(head);
}

void ArticleViewer::endWriting()
{
    m_part->write(m_htmlFooter);
    //kDebug() << m_htmlFooter;
    m_part->end();
}


void ArticleViewer::slotShowSummary( const Akonadi::Collection& c, int unread )
{
    m_viewMode = SummaryView;

    if ( !c.isValid() )
    {
        slotClear();
        return;
    }

    QString summary = m_normalViewFormatter->formatSummary( c, unread );
    m_link.clear();
    renderContent(summary);

    setArticleActionsEnabled(false);
}

#ifdef KRSS_PORT_DISABLED
class PreferredLinkVisitor : public ConstFeedVisitor {
public:

    void visitNetFeed( const shared_ptr<const NetFeed>& f ) {
        link = f->preferItemLinkForDisplay() ? KUrl( item.link() ) : KUrl();
    }

    KUrl getPreferredLink( const shared_ptr<const Feed>& f, const Item& i ) {
        item = i;
        link.clear();
        if ( !f )
            return link;
        f->accept( this );
        return link;
    }

    Item item;
    KUrl link;
};
#endif

void ArticleViewer::showItem( const Akonadi::Collection& storageCollection, const Akonadi::Item& aitem )
{
    if ( !aitem.isValid() || KRss::Item::isDeleted( aitem ) )
    {
        slotClear();
        return;
    }

    const KRss::Item item = aitem.payload<KRss::Item>();
    m_viewMode = NormalView;
#ifdef KRSS_PORT_DISABLED
    disconnectFromNode(m_node);
#endif // KRSS_PORT_DISABLED

    m_link = KUrl( item.link() );

    const KRss::FeedCollection fc( storageCollection );

    if ( fc.preferItemLinkForDisplay() && !item.link().isEmpty() )
        openUrl( m_link );
    else
      renderContent( m_normalViewFormatter->formatItem( aitem ) );

    setArticleActionsEnabled(true);
}

bool ArticleViewer::openUrl(const KUrl& url)
{
#ifdef KRSS_PORT_DISABLED

    const shared_ptr<const FeedList> fl = m_feedList.lock();
    const shared_ptr<const Feed> f = fl ? fl->constFeedById( m_item.sourceFeedId() ) : shared_ptr<const Feed>();
    PreferredLinkVisitor visitor;
    const KUrl url2 = visitor.getPreferredLink( f, m_item );

    if ( url2.isValid() )
        return m_part->openUrl(url);
#endif
    reload();
    return true;
}

void ArticleViewer::slotUpdateCombinedView()
{
    if (m_viewMode != CombinedView)
        return;


   Akonadi::Item::List items;

   const int rows = m_model->rowCount();
   items.reserve( rows );
   for ( int i = 0; i < rows; ++i ) {
       const Akonadi::Item item = m_model->index( i, 0 ).data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
       items.append( item );
   }

   QString text;
   Q_FOREACH( const Akonadi::Item& i, items )
       text += "<p><div class=\"article\">"+m_combinedViewFormatter->formatItem( i )+"</div><p>";

   renderContent(text);
}


void ArticleViewer::slotClear()
{
    if ( m_model ) {
        m_model->disconnect( this );
        m_model = 0;
    }
    renderContent(QString());
}

void ArticleViewer::triggerUpdate() {
    if ( !m_updateTimer.isActive() )
        m_updateTimer.start();
}

void ArticleViewer::showNode( QAbstractItemModel* m )
{
    m_viewMode = CombinedView;

    if ( m_model == m )
        return;

    if ( m_model )
        m_model->disconnect( this );

    m_model = m;

    connect( m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
             this, SLOT(triggerUpdate()) );
    connect( m_model, SIGNAL(layoutChanged()),
             this, SLOT(triggerUpdate()) );
    connect( m_model, SIGNAL(rowsInserted(QModelIndex,int,int)),
             this, SLOT(triggerUpdate()) );
    connect( m_model, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
             this, SLOT(triggerUpdate()) );
    connect( m_model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
             this, SLOT(triggerUpdate()) );

    //PENDING(frank): don't repaint synchronously for each signal fired
    slotUpdateCombinedView();
}


static bool lessByDate( const KRss::Item& lhs, const KRss::Item& rhs ) {
    return lhs.dateUpdated() < rhs.dateUpdated();
}

void ArticleViewer::keyPressEvent(QKeyEvent* e)
{
    e->ignore();
}

void ArticleViewer::slotPaletteOrFontChanged()
{
    updateCss();
    reload();
}

void ArticleViewer::reload()
{
    beginWriting();
    m_part->write(m_currentText);
    endWriting();
}

QSize ArticleViewer::sizeHint() const
{
    // Increase height a bit so that we can (roughly) read 25 lines of text
    QSize sh = QWidget::sizeHint();
    sh.setHeight(qMax(sh.height(), 25 * fontMetrics().height()));
    return sh;
}

void ArticleViewer::displayAboutPage()
{
    QString location = KStandardDirs::locate("data", "akregator2/about/main.html");

    m_part->begin(KUrl::fromPath( location ));
    QString info =
            i18nc("%1: Akregator2 version; %2: homepage URL; "
            "--- end of comment ---",
    "<h2 style='margin-top: 0px;'>Welcome to Akregator2 %1</h2>"
            "<p>Akregator2 is a feed reader for the K Desktop Environment. "
            "Feed readers provide a convenient way to browse different kinds of "
            "content, including news, blogs, and other content from online sites. "
            "Instead of checking all your favorite web sites manually for updates, "
            "Akregator2 collects the content for you.</p>"
            "<p>For more information about using Akregator2, check the "
            "<a href=\"%2\">Akregator2 website</a>. If you do not want to see this page "
            "anymore, <a href=\"config:/disable_introduction\">click here</a>.</p>"
            "<p>We hope that you will enjoy Akregator2.</p>\n"
            "<p>Thank you,</p>\n"
            "<p style='margin-bottom: 0px'>&nbsp; &nbsp; The Akregator2 Team</p>\n",
    KDEPIM_VERSION, // Akregator2 version
    "http://akregator2.kde.org/"); // Akregator2 homepage URL

    QString fontSize = QString::number( pointsToPixel( Settings::mediumFontSize() ));
    QString appTitle = i18n("Akregator2");
    QString catchPhrase = ""; //not enough space for a catch phrase at default window size i18n("Part of the Kontact Suite");
    QString quickDescription = i18n("An RSS feed reader for the K Desktop Environment.");

    QString content = KPIMUtils::kFileToByteArray(location);

    QString infocss = KStandardDirs::locate( "data", "kdeui/about/kde_infopage.css" );
    QString rtl = kapp->isRightToLeft() ? QString("@import \"%1\";" ).arg( KStandardDirs::locate( "data", "kdeui/about/kde_infopage_rtl.css" )) : QString();

    m_part->write( content.arg( infocss, rtl, fontSize, appTitle, catchPhrase, quickDescription, info ) );
    m_part->end();
}

ArticleViewerPart::ArticleViewerPart(QWidget* parent) : KHTMLPart(parent),
     m_button(-1)
{
    setXMLFile(KStandardDirs::locate("data", "akregator2/articleviewer.rc"), true);
}

int ArticleViewerPart::button() const
{
    return m_button;
}

bool ArticleViewerPart::closeUrl()
{
    emit browserExtension()->loadingProgress(-1);
    emit canceled(QString());
    return KHTMLPart::closeUrl();
}

bool ArticleViewerPart::urlSelected(const QString &url, int button, int state, const QString &_target,
                                    const KParts::OpenUrlArguments& args,
                                    const KParts::BrowserArguments& browserArgs)
{
    m_button = button;
    if(url == "config:/disable_introduction")
    {
        KGuiItem yesButton(KStandardGuiItem::yes());
        yesButton.setText(i18n("Disable"));
        KGuiItem noButton(KStandardGuiItem::no());
        noButton.setText(i18n("Keep Enabled"));
        if(KMessageBox::questionYesNo( widget(), i18n("Are you sure you want to disable this introduction page?"), i18n("Disable Introduction Page"), yesButton, noButton) == KMessageBox::Yes)
        {
            KConfigGroup conf(Settings::self()->config(), "General");
            conf.writeEntry("Disable Introduction", "true");
            conf.sync();
            return true;
        }

        return false;
    }
    else
        return KHTMLPart::urlSelected(url,button,state,_target,args,browserArgs);
}

void ArticleViewer::updateCss()
{
    m_normalModeCSS =  m_normalViewFormatter->getCss();
    m_combinedModeCSS = m_combinedViewFormatter->getCss();
}

void ArticleViewer::setNormalViewFormatter( const shared_ptr<ArticleFormatter>& formatter )
{
    assert( formatter );
    m_normalViewFormatter = formatter;
    m_normalViewFormatter->setPaintDevice(m_part->view());
}

void ArticleViewer::setCombinedViewFormatter( const shared_ptr<ArticleFormatter>& formatter )
{
    assert( formatter );
    m_combinedViewFormatter = formatter;
    m_combinedViewFormatter->setPaintDevice(m_part->view());
}

void ArticleViewer::setArticleActionsEnabled(bool enabled)
{
    ActionManager::getInstance()->setArticleActionsEnabled(enabled);
}

} // namespace Akregator2
