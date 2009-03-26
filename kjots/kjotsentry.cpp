//
//  kjots
//
//  Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
//  Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
//  Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
//  Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>
//  Copyright (C) 2007 Stephen Kelly <steveire@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

//Own Header
#include "kjotsentry.h"

#include <QTextCodec>
#include <QTextDocumentFragment>
#include <qdom.h>
#include <QCoreApplication>

#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <ksavefile.h>

#include <assert.h>

#include "kjotsedit.h"
#include "KJotsMain.h"
#include "KJotsSettings.h"

QSet<quint64> KJotsEntry::all_ids;

//
// KJotsEntry
//
KJotsEntry::KJotsEntry()
{
    m_id = 0;
}

void KJotsEntry::setTitle(const QString& title)
{
    setText(0, title);
}

/*!
    \brief Returns the parent book or 0 if there is none.
*/
KJotsBook* KJotsEntry::parentBook()
{
    return dynamic_cast<KJotsBook*>(QTreeWidgetItem::parent());
}

/*!
    \brief Returns the topmost level book. It could be you!
*/
KJotsBook* KJotsEntry::topLevelBook()
{
    KJotsBook *topBook = parentBook();
    if ( !topBook ) {
        topBook = static_cast<KJotsBook*>(this);
    } else {
        while ( topBook->parentBook() ) {
            topBook = topBook->parentBook();
        }
    }

    return topBook;
}

/*!
*    \brief Set the ID number.
*    \param id The ID to set, or 0 to pick a fresh one.
*
*    Set ourselves to a particular ID number, or pick a new one. The lastId used is
*    stored in the settings.
*/
void KJotsEntry::setId(quint64 id)
{
    if ( id && all_ids.contains(id) ) {
        //WOAH! This shouldn't happen. Some naughty user has been playing around with
        //the data files by hand. Since they caused the problem, ask if they want to
        //fix it themselves or let KJots handle it.
        KMessageBox::ButtonCode answer = (KMessageBox::ButtonCode) KMessageBox::warningYesNo(0,
            i18n("A duplicate ID was found in the book %1. This can happen if you "
            "manipulate the data files by hand, and will prevent KJots from working correctly."
            " KJots can attempt to fix this, or you can fix it yourself. In either "
            "case, bookmarks and links between pages may be broken.", title()),
            i18n("A duplicate ID was found"), KGuiItem(i18n("Exit and fix by hand")),
            KGuiItem(i18n("Fix it automatically")), "duplicateIdError",
            (KMessageBox::Notify | KMessageBox::Dangerous) );

        if ( answer == KMessageBox::Yes ) {
            QCoreApplication::exit(1);
        } else {
            id = 0;
            topLevelBook()->m_dirty = true; //Have to set manually; not open yet.
        }
    }

    //Find an unused id...
    if ( id == 0 ) {
        quint64 lastId = KJotsSettings::lastId();

        // Find an unused ID.
        do {
            id = ++lastId;
            if ( id == 0 ) {
                id = ++lastId; //Wrapping is OK, but zero is not.
            }
        } while ( all_ids.contains(id) );
        KJotsSettings::setLastId(id);
    }

    all_ids << id;
    m_id = id;
    setText(1, QString::number(id));
    return;
}

/*!
    \brief Performs functions necessary to save the entry to a KJots file.
    This function should ONLY be called when saving the file, as it performs
    other functions than generating XML.
*/
void KJotsEntry::generateXml( QDomDocument &doc, QDomElement &parent )
{
    QDomElement title = doc.createElement( "Title" );
    title.appendChild( doc.createTextNode( text(0) ));
    parent.appendChild( title );

    QDomElement id = doc.createElement( "ID" );
    QString id_string;
    id_string.setNum(m_id);
    id.appendChild( doc.createTextNode(id_string) );
    parent.appendChild( id );

    QColor currentColor = backgroundColor(0);
    if ( currentColor.isValid() ) {
        QDomElement color = doc.createElement( "Color" );
        color.appendChild( doc.createTextNode( currentColor.name() ));
        parent.appendChild( color );
    }

    return;
}

/*!
    Parses through XML data for settings inherent to the base class.
*/
void KJotsEntry::parseXml( QDomElement &e, bool )
{
    if ( !e.isNull() )
    {
        if ( e.tagName() == "Title" )
        {
            setTitle(e.text());
        }
        else
        if ( e.tagName() == "ID" )
        {
            setId(e.text().toULongLong());
        }
        else
        if ( e.tagName() == "Color" )
        {
            QColor color( e.text() );
            setBackgroundColor(0, color);
        }
    }

    return;
}


bool KJotsEntry::isKJotsLink(const QString &link)
{
    return link.startsWith(kjotsLinkStringPrefix());
}

QString KJotsEntry::kjotsLinkUrl()
{
    return kjotsLinkStringPrefix() + QString::number(id());
}

QString KJotsEntry::kjotsLinkUrlFromId(quint64 id)
{
    return kjotsLinkStringPrefix() + QString::number(id);
}

quint64 KJotsEntry::idFromLinkUrl(const QString &link)
{
    if (!link.startsWith(kjotsLinkStringPrefix()))
    {
        return 0;
    } else {
        QString s(link);
        QString idString = s.remove(kjotsLinkStringPrefix());
        bool ok;
        quint64 id = idString.toULongLong(&ok, 10);
        return ok == true ? id : 0;
    }
}


//
// KJotsBook
//
KJotsBook::KJotsBook()
{
    m_isBook = true;
    m_open = m_shouldBeOpened = m_dirty = false;
    setIcon(0, KIconLoader::global()->loadIcon(QString("x-office-address-book"),KIconLoader::Small));
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled
        | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
}

KJotsBook::~KJotsBook()
{
}

/*!
    \brief Sets the dirty flag.
*/
void KJotsBook::setDirty(bool dirty)
{
    //Don't count opening a book as dirtying it.
    if ( !m_open ) return;

    topLevelBook()->m_dirty = dirty;
}

/*!
    \brief Returns the dirty flag.
*/
bool KJotsBook::dirty()
{
    return topLevelBook()->m_dirty;
}

/*!
    \brief Reads a book in from a disk file.
    This function is only called for root-level books.
*/
bool KJotsBook::openBook(const QString& filename)
{
    //sanity check
    if ( m_open ) return true;

    m_fileName = filename;

    //new books names are empty.
    if ( m_fileName.isEmpty() ) {

        if (childCount() == 0){
            addPage();
        }
        m_open = m_dirty = true;
        setId(0);
    } else {
        QFile file(m_fileName);
        QDomDocument doc( "KJots" );
        bool oldBook = false;

        //TODO: Implement read-only mode?
        if ( !file.exists() || !file.open(QIODevice::ReadWrite) ) {
            return false;
        }

        //Determine if this is a KDE3.5 era book.
        QByteArray firstLine = file.readLine();
        file.reset();

        if ( !firstLine.startsWith( "<?xml" ) ) { // krazy:exclude=strings
            kDebug(0) << m_fileName << " is an old-style book." ;

            QTextStream st(&file);
            if (  KJotsSettings::unicode() ) {
                st.setCodec( "UTF-8" );
            } else {
                st.setCodec( QTextCodec::codecForLocale () );
            }

            doc.setContent( st.readAll() );
            oldBook = true;
        } else {
            doc.setContent(&file);
        }

        QDomElement docElem = doc.documentElement();

        if ( docElem.tagName() ==  "KJots" ) {
            QDomNode n = docElem.firstChild();
            while( !n.isNull() ) {
                QDomElement e = n.toElement(); // try to convert the node to an element.
                if( !e.isNull() && e.tagName() == "KJotsBook" ) {
                    parseXml(e, oldBook);
                }
                n = n.nextSibling();
            }

            m_open = true;
        }
    }

    return m_open;
}

/*!
    \brief Saves this book and everything in it to the data directory.
*/
void KJotsBook::saveBook(void)
{
    bool failed = false;

    if (!m_open) //sanity check
        return;

    // Are we a new book?
    if ( m_fileName.isEmpty() ) {
        KTemporaryFile file;
        file.setPrefix(KStandardDirs::locateLocal("data","kjots/"));
        file.setSuffix(".book");
        file.setAutoRemove(false);
        if ( file.open() ) {
            m_fileName = file.fileName();
        } else {
            kDebug() << "failed to open temporary file for saving";
            failed = true;
        }
    }

    if ( !failed ) {
        KSaveFile file(m_fileName);
        if ( file.open() ) {
            QDomDocument doc("KJots");
            QDomElement root = doc.createElement( "KJots" );
            doc.appendChild( root );

            this->generateXml( doc, root ); //recursive

            QTextStream st(&file);
            st.setCodec( "UTF-8" );
            st << "<?xml version='1.0' encoding='UTF-8' ?>\n";
            st << doc.toString();
            st.flush();

            setDirty(false);
        } else {
          kDebug() << "failed to open savefile" << m_fileName;
            failed = true;
        }
    }

    if ( failed ) {
        KMessageBox::error(0, i18n("<qt>KJots is having problems saving your data. " \
            "This might be a permissions problem, or you may be out of disk space.</qt>"));
    }
}

void KJotsBook::saveAndBackupBook()
{
    if (!m_fileName.isEmpty())
    {
      KSaveFile::numberedBackupFile(m_fileName);
    }
    saveBook();
}

/*!
    \brief Deletes a book by removing the data file.
    This does not affect the list display, and can be called for reasons other than
    choosing to delete an entry from the list. If you want the object to dissappear
    from the list, then you have to delete it.
*/
void KJotsBook::deleteBook ( void )
{
    if (!m_fileName.isEmpty())
    {
        QFile::remove(m_fileName);
    }
    m_fileName.clear();
}

/*!
    \brief Displays rename dialog.
*/
void KJotsBook::rename()
{
    bool ok;
    QString name = KInputDialog::getText(i18n( "Rename Book" ),
        i18n( "Book name:" ), title(), &ok, treeWidget());

    if (ok) {
        setTitle(name);
        topLevelBook()->setDirty(true);
    }
}

/*!
    \brief Add a new page to the end of this book.
*/
KJotsPage* KJotsBook::addPage(void)
{
    int pageCount = 1;

    //Only count pages
    for ( int i=0; i<childCount(); i++ ) {
        if ( dynamic_cast<KJotsEntry*>(QTreeWidgetItem::child(i)) ) {
            ++pageCount;
        }
    }

    KJotsPage *page = KJotsPage::createNewPage(pageCount);
    addChild(page);
    return page;
}

/*!
    \brief Creates XML code and performs necessary tasks to save file.
    This function should ONLY be called when saving the file.
*/
void KJotsBook::generateXml( QDomDocument &doc, QDomElement &parent )
{
    QDomElement book = doc.createElement( "KJotsBook" );
    parent.appendChild( book );

    KJotsEntry::generateXml(doc, book); //let the base class save important stuff

    QDomElement open = doc.createElement( "Open" );
    open.appendChild( treeWidget()->isItemExpanded(this) ? doc.createTextNode("1") : doc.createTextNode("0") );
    book.appendChild( open );

    for ( int i=0; i<childCount(); i++ ) {
        KJotsEntry *entry = dynamic_cast<KJotsEntry*>(QTreeWidgetItem::child(i));
        if ( entry ) {
            entry->generateXml( doc, book );
        }
    }

    if ( !m_fileName.isEmpty() && QTreeWidgetItem::parent() )
    {
        //Hmmmm... We were originally loaded from a file, but now we have a parent, so
        //we must have been moved into another tree. Remove the old file now that we
        //have saved ourselves into the new tree.
        deleteBook();
    }

    return;
}

/*!
    Parses through XML code from a file.
*/
void KJotsBook::parseXml( QDomElement &me, bool oldBook )
{
    if ( me.tagName() == "KJotsBook" ) {
        QDomNode n = me.firstChild();
        while( !n.isNull() ) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if ( !e.isNull() ) {
                if ( e.tagName() == "KJotsPage" ) {
                    KJotsPage *page = new KJotsPage();
                    addChild(page);
                    page->parseXml(e, oldBook);
                }
                else if ( e.tagName() == "KJotsBook" ) {
                    KJotsBook *book = new KJotsBook();
                    addChild(book);
                    book->parseXml(e, oldBook);
                }
                else if ( e.tagName() == "Open" ) {
                    if ( e.text() == "1" ) {
                        // We can't open ourselves here; we have to wait.
                        m_shouldBeOpened = true;
                    }
                } else {
                    //What was this? Send it to the base class and see if it can figure it out.
                    KJotsEntry::parseXml(e, oldBook);
                }
            }
            n = n.nextSibling();
        }
    }

    return;
}

/*!
    \brief Returns an HTML Table of contents for this book.
    This is a helper function for generateHtml().
*/
QString KJotsBook::getToc()
{
    QString toc;

    toc += "<ul>";

    for ( int i=0; i<childCount(); i++ ) {
        KJotsEntry *entry = dynamic_cast<KJotsEntry*>(QTreeWidgetItem::child(i));
        if ( entry ) {
            QString htmlSubject = Qt::escape(entry->title());
            toc += QString("<li><a href=\"#%1\">").arg(entry->id()) + htmlSubject + "</a></li>";

            KJotsBook *book = dynamic_cast<KJotsBook*>(entry);
            if ( book ) {
                toc += book->getToc();
            }
        }
    }

    toc += "</ul><br>";
    return toc;
}

/*!
    \brief Returns HTML for the read-only "book" view.
    \param top Pointer to the "starting point" of this tree.
    \param diskMode Files saved to disk have a slightly different format.
*/
void KJotsBook::generateHtml( KJotsEntry* top, bool diskMode, QTextCursor *cursorOut )
{
    QString toc;
    QString htmlTitle = Qt::escape(title());

    if ( top == this ) {
        toc = QString("<h1><a name=\"%1\">%2</a></h1>").arg(id()).arg(htmlTitle);
    } else {
        if ( diskMode ) {
            toc = QString("<h2><a name=\"%1\">%2</a></h2>").arg(id()).arg(htmlTitle);
        } else {
            //We need to fake QUrl in to thinking this is a real URL
            toc = QString("<h2><a name=\"%1\">&nbsp;</a><a href=\"%2\">%3</a></h2>").arg(id()).arg(kjotsLinkUrl()).arg(htmlTitle);
        }
    }

    toc += "<table width=\"100%\"><tr><td>";
    toc += "<h3>" + i18n("Table of Contents") + "</h3>";
    toc += getToc();
    toc += "<hr /></td></tr></table>";
    cursorOut->insertFragment(QTextDocumentFragment::fromHtml(toc));

    // Do the body text of the entries.
    foreach ( KJotsEntry *entry, children() ) {
        entry->generateHtml ( top, diskMode, cursorOut );
    }

    return;
}

/*!
    \brief Returns Text when saving to a file.

    This functions output moderately formatted text when the user chooses to save as
    a text file.
*/
QString KJotsBook::generateText( void )
{
    QString out;

    //Print Fancy Text header
    QString line, buf;
    line.fill('#', title().length() + 2);
    line += '\n';
    out = line + QString("# ") + title() + QString("\n") + line;

    foreach ( KJotsEntry *entry, children() ) {
        out += entry->generateText();
    }

    out += '\n';
    return out;
}

/*!
    \brief Inserts data for printing.
    \param cursor Cursor pointing to the printing document.

    This inserts data into a document for printing. There is data that is
    only used when printing, like the title bar across the top.
*/
void KJotsBook::generatePrintData ( QTextCursor *cursor )
{
    cursor->insertFragment(QTextDocumentFragment::fromHtml(
        QString ("<table border=1 width='100%'><tr><td><center>%1</center></td></tr></table>"
        ).arg(title()) ));

    foreach ( KJotsEntry *entry, children() ) {
        entry->generatePrintData ( cursor );
    }

    return;
}

/*!
    Returns QList of all entries in this book. THis is recursive, as
    opposed to KJotsBook::children which is not.
*/
QList<KJotsEntry*> KJotsBook::contents ( void )
{
    QList<KJotsEntry*> contents;

    for ( int i=0; i<childCount(); i++ ) {
        contents << static_cast<KJotsEntry*>(QTreeWidgetItem::child(i));
        KJotsBook *childBook = dynamic_cast<KJotsBook*>(QTreeWidgetItem::child(i));
        if ( childBook ) {
            contents << childBook->contents();
        }
    }

    return contents;
}

/*!
    Returns QList of centries in this book. This is not recursive, as
    opposed to KJotsBook::contents which is.
*/
QList<KJotsEntry*> KJotsBook::children ( void )
{
    QList<KJotsEntry*> children;

    for ( int i=0; i<childCount(); i++ ) {
        children << static_cast<KJotsEntry*>(QTreeWidgetItem::child(i));
    }

    return children;
}

KJotsBook *KJotsBook::createNewBook ( void )
{
    KJotsBook *book = 0;
    bool ok;

    QString name = KInputDialog::getText( i18n( "New Book" ),
        i18n( "Book name:" ), QString(), &ok );

    if ( ok ) {
        book = new KJotsBook();
        book->setTitle(name);
        book->openBook(QString());
    }

    return book;
}

//
// KJotsPage
//
KJotsPage::KJotsPage()
{
    m_isBook = false;
    setIcon(0, KIconLoader::global()->loadIcon(QString("text-x-generic"), KIconLoader::Small));
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled
        | Qt::ItemIsEnabled);

    // Don't have an action to set top and bottom margins on paragraphs.
    // Set the margins to 0 for all inserted html. Lists get some padding top and bottom.
    // See http://bugs.kde.org/show_bug.cgi?id=160600.
    document.setDefaultStyleSheet( "p,h1,h2,h3,h4,h5,h6,pre,br{margin-top:0px;margin-bottom:0px;}ul{margin-top:12px;margin-bottom:12px;}" );

    connect(&document, SIGNAL(modificationChanged(bool)), SLOT(documentModified(bool)));
}

KJotsPage::~KJotsPage()
{
}

/*!
    Creates a brand new page.
*/
KJotsPage *KJotsPage::createNewPage(int pageCount)
{
    QString title = i18n("Page %1", pageCount);

    if ( KJotsSettings::pageNamePrompt() ) {
        title = KInputDialog::getText( i18n( "New Page" ),
            i18n( "Page name:" ), title, 0 );
    }

    KJotsPage *page = new KJotsPage();
    page->setId(0);
    page->setTitle(title);
    return page;
}

/*!
    \brief Handles changes in the document
    Document edits are allowed to set the dirty flag to true, but it is
    NOT allowed to set it to clean, because other things affect the dirty
    flag than just editing,
*/
void KJotsPage::documentModified(bool modified)
{
    if ( modified ) {
        topLevelBook()->setDirty(true);
    }

    return;
}

/*!
    \brief Displays rename dialog.
*/
void KJotsPage::rename()
{
    bool ok;
    QString name = KInputDialog::getText(i18n( "Rename Page" ),
        i18n( "Page title:" ), title(), &ok, treeWidget() );

    if (ok) {
        setTitle(name);
        topLevelBook()->setDirty(true);
    }
}

/*!
    \brief Creates XML code and performs necessary tasks to save file.
    This function should ONLY be called when saving the file.
*/void KJotsPage::generateXml( QDomDocument &doc, QDomElement &parent )
{
    QDomElement page = doc.createElement( "KJotsPage" );
    parent.appendChild( page );

    KJotsEntry::generateXml(doc, page); //let the base class save important stuff

    QDomElement text = doc.createElement( "Text" );

    QString saveText = document.toHtml("UTF-8");
    text.appendChild( doc.createCDATASection( saveText ));
    page.appendChild( text );

    return;
}

/*!
    Parses through XML code from a file.
*/
void KJotsPage::parseXml( QDomElement &me, bool oldBook )
{
    if ( me.tagName() == "KJotsPage" )
    {
        QDomNode n = me.firstChild();
        while( !n.isNull() )
        {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if ( !e.isNull() )
            {
                if ( e.tagName() == "Text" )
                {
                    QString bodyText = e.text();

                    //This is for 3.5 era book support. Remove when appropriate.
                    if ( e.hasAttribute("fixed") ) {
                        bodyText.replace("]]&gt;", "]]>");
                    }

                    if ( oldBook ) {
                        // Ensure that whitespace is reproduced as in kjots of kde3.5.
                        // https://bugs.kde.org/show_bug.cgi?id=175100
                        document.setPlainText(bodyText);
                    } else {
                        document.setHtml(bodyText);
                    }
                }
                else
                {
                    //What was this? Send it to the base class and see if it can figure it out.
                    KJotsEntry::parseXml(e, oldBook);
                }
            }
            n = n.nextSibling();
        }
    }

    return;
}

/*!
    \brief Returns HTML for the read-only "book" overview.
    \param top This tells us that we are the toplevel.
    \param diskMode Files saved to disk have a slightly different format.
*/
void KJotsPage::generateHtml( KJotsEntry *top, bool diskMode, QTextCursor *cursorOut )
{
    QTextBlockFormat defaultBlockFormat = cursorOut->blockFormat();
    QTextCharFormat defaultCharFormat = cursorOut->charFormat();
    QString html;
    QString htmlSubject = Qt::escape(title());

    if ( diskMode ) {
        if ( top != this ) {
            html = "<table><tr><td>";
            html += QString("<h3><a name=\"%1\">%2</a></h3>").arg(id()).arg(htmlSubject);
            html += "</td></tr></table>";
        } else {
            html = "<table><tr><td>";
            html += QString("<h3>%1</h3>").arg(htmlSubject);
            html += "</td></tr></table>";
        }
    } else {
        //We need to fake QUrl in to thinking this is a real URL

        html = "<table><tr><td>";
        html += QString("<h3><a name=\"%1\">&nbsp;</a><a href=\"%2\" >%3</a></h3>").arg(id()).arg(kjotsLinkUrl()).arg(htmlSubject);
        html += "</td></tr></table>";
    }
    html += "<br>";

    cursorOut->insertFragment(QTextDocumentFragment::fromHtml(html));
    cursorOut->insertBlock(defaultBlockFormat, defaultCharFormat);
    html.clear(); //stop myself from making the same mistake over and over again.

    QTextCursor allCursor ( &document );
    allCursor.select( QTextCursor::Document );
    cursorOut->insertFragment(allCursor.selection());
    cursorOut->insertBlock(defaultBlockFormat, defaultCharFormat);

    if ( top != this ) {
        html = "<table width=\"100%\"><tr><td>";
        html += "<table border=1 cellpadding=\"10\"><tr>";
        html += QString("<td><a href=\"#%1\">%2</a></td>").arg(id()).arg(title());

        KJotsBook *parent = parentBook();
        while ( parent ) {
            html += QString("<td><a href=\"#%1\">%2</a></td>").arg(parent->id()).arg(parent->title());
            if ( parent == top ) break;
            parent = parent->parentBook();
        }

        html += QString("</tr></table>");
        html += "<hr /></td></tr></table>";
    }

    cursorOut->insertFragment(QTextDocumentFragment::fromHtml(html));

    return;
}

/*!
    \brief Returns Text when saving to a file.
    \param top This is a hint to specify if this book is the "starting point" or not.

    This functions output moderately formatted text when the user chooses to save as
    a text file.
*/
QString KJotsPage::generateText( void )
{
    QString out;

    //Print Fancy Text header
    QString line, buf;
    line.fill('#', title().length() + 2);
    line += '\n';
    out = line + QString("# ") + title() + QString("\n") + line;

    out += document.toPlainText();

    out += '\n';
    return out;
}

/*!
    \brief Inserts data for printing.
    \param cursor Cursor pointing to the printing document.

    This inserts data into a document for printing. There is data that is
    only used when printing, like the title bar across the top.
*/
void KJotsPage::generatePrintData ( QTextCursor *cursor )
{
    QString docName = QString ("%1: %2").arg(parentBook()->title()).arg(title());

    cursor->insertFragment(QTextDocumentFragment::fromHtml(
        QString ("<table border=1 width='100%'><tr><td><center>%1</center></td></tr></table>"
        ).arg(docName) ));

    QTextCursor allCursor ( &document );
    allCursor.select( QTextCursor::Document );
    cursor->insertFragment(allCursor.selection());

    return;
}

#include "kjotsentry.moc"
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
