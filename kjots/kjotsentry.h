//
//  kjots
//
//  Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
//  Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
//  Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
//  Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>
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

#ifndef __KJOTSENTRY_H
#define __KJOTSENTRY_H

#include <QTreeWidgetItem>
#include <QTextDocument>
#include <QTextCursor>
#include <QSet>

//
// class KJotsEntry
//

class KJotsPage;
class QDomDocument;
class QDomElement;
class KJotsBook;

class KJotsEntry : public QObject, public QTreeWidgetItem
{
    Q_OBJECT
    public:
        KJotsEntry();

    public:
        virtual void setTitle(const QString&);
        QString title() { return text(0); }
        virtual void rename() = 0;
        virtual void generateXml( QDomDocument&, QDomElement& );
        virtual void parseXml( QDomElement&, bool );
        virtual void generateHtml( KJotsEntry*, bool, QTextCursor* ) = 0;
        virtual QString generateText( void ) = 0;
        virtual void generatePrintData ( QTextCursor * ) = 0;

        quint64 id() const { return m_id; }

        KJotsBook *parentBook();
        KJotsBook *topLevelBook();

        bool isBook() const { return m_isBook; }
        bool isPage() const { return !m_isBook; }

        static bool isKJotsLink(const QString &link);
        QString kjotsLinkUrl();
        static quint64 idFromLinkUrl(const QString &link);
        static QString kjotsLinkUrlFromId(quint64 id);

        virtual void setEditable(bool editable) = 0;    //items can be made read-only with this. implemented in children classes
        virtual bool isEditable() = 0;                  //check if an item is read-only

    protected:
        void setId(quint64);
        bool m_isBook; //!< used for speed and code clarity.

    private:
        quint64 m_id; //!< unique ID for this entry
        static QSet<quint64> all_ids;
        static QString kjotsLinkStringPrefix() { return QString("kjots://0.0.0.0/"); }
};

//
// class KjotsBook
//

class KJotsBook : public KJotsEntry
{
friend class KJotsEntry;

    Q_OBJECT
    public:
        KJotsBook ();
        ~KJotsBook();

        bool openBook(const QString&);
        void saveBook();
        void saveAndBackupBook();
        void deleteBook();
        void rename();
        KJotsPage* addPage(void);
        void generateXml( QDomDocument&, QDomElement& );
        void parseXml( QDomElement&, bool );
        void generateHtml( KJotsEntry*, bool, QTextCursor* );
        QString generateText( void );
        void generatePrintData ( QTextCursor * );

        bool shouldBeOpened ( void ) const { return m_shouldBeOpened; }

        void setDirty(bool);
        bool dirty();
        QString fileName() const { return m_fileName; }

        QList<KJotsEntry*> contents ( void );
        QList<KJotsEntry*> children ( void );

        static KJotsBook *createNewBook( void );

        void setEditable(bool editable);    //items can be made read-only with this.
        bool isEditable();                  //check if an item is read-only

    private:
        QString getToc();

        bool m_open, m_shouldBeOpened;
        QString m_fileName;
        bool m_dirty; //!< Set when this book needs saving.

        bool m_editable; //used for making items read-only
};

//
// class KJotsPage
//
class KJotsPage : public KJotsEntry
{
    Q_OBJECT
    public:
        KJotsPage();
        ~KJotsPage();

    public:
        QTextDocument *body ( void ) { return &document; }
        void setCursor ( const QTextCursor &c ) { cursor = c; }
        QTextCursor& getCursor ( void ) { return cursor; }
        void rename();

        void generateXml( QDomDocument&, QDomElement& );
        void parseXml( QDomElement&, bool );
        void generateHtml( KJotsEntry*, bool, QTextCursor* );
        QString generateText( void );
        void generatePrintData ( QTextCursor * );

        static KJotsPage *createNewPage( int );

        void setEditable(bool editable);    //items can be made read-only with this.
        bool isEditable();                  //check if an item is read-only

    protected slots:
        void documentModified(bool);

    private:
        QTextDocument document;
        QTextCursor cursor;

        bool m_editable; //used for making items read-only
};

#endif // __KJOTSENTRY_H
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
