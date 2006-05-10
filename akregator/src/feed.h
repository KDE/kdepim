/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#ifndef AKREGATORFEED_H
#define AKREGATORFEED_H

#include "treenode.h"
#include "librss/librss.h"

class QDomElement;
class QPixmap;
class QString;
class QStringList;
class KURL;


namespace KPIM {
    class ProgressItem;
}

// needed for slot fetchCompleted()
using RSS::Document;
using RSS::Loader;
using RSS::Status;

namespace Akregator
{
    class Article;
    class FetchQueue;
    class Folder;
    class TreeNodeVisitor;

    namespace Backend
    {
        class FeedStorage;
    }
    /**
        represents a feed
     */
    class Feed : public TreeNode
    {
        friend class Article;
        
        Q_OBJECT
        public:
            /** the archiving modes:
                - globalDefault: use default from Settings (default)
                - keepAllArticles: Don't delete any articles
                - disableArchiving: Don't save any articles except articles with keep flag set (equal to maxArticleNumber() == 0)
                - limitArticleNumber: Save maxArticleNumber() articles, plus the ones with keep flag set
                - limitArticleAge: Save articles not older than maxArticleAge() (or keep flag set)
             */
            enum ArchiveMode { globalDefault, keepAllArticles, disableArchiving, limitArticleNumber, limitArticleAge };

            // class methods
            /** converts strings to ArchiveMode value
             if parsing fails, it returns ArchiveMode::globalDefault
             */
            static ArchiveMode stringToArchiveMode(const QString& str);

            /** converts ArchiveMode values to corresponding strings */
            static QString archiveModeToString(ArchiveMode mode);

            /** creates a Feed object from a description in OPML format */
            static Feed* fromOPML(QDomElement e);

            /** default constructor */
            Feed();

            virtual ~Feed();

            virtual bool accept(TreeNodeVisitor* visitor);

            /** exports the feed settings to OPML */
            virtual QDomElement toOPML( QDomElement parent, QDomDocument document ) const;

            /**
              returns whether this feed uses its own fetch interval or the global setting
              @return @c true iff this feed has a custom fetch interval
             */
            bool useCustomFetchInterval() const;

            /** set if the feed has its custom fetch interval or uses the
                global setting
                @param enabled @c true: use custom interval, @c false: use global default
             */
            void setCustomFetchIntervalEnabled(bool enabled);

            // FIXME is it -1 or 0 to disable interval fetching?
            /** Returns custom auto fetch interval of this feed.
            @return custom fetch interval in minutes, 0 if disabled */
            int fetchInterval() const;

            /** Sets custom auto fetch interval.
            @param interval interval in minutes, -1 for disabling auto fetching */
            void setFetchInterval(int interval);

            /** returns the archiving mode which is used for this feed */
            ArchiveMode archiveMode() const;

            /** sets the archiving mode for this feed */
            void setArchiveMode(ArchiveMode archiveMode);

            /** returns the maximum age of articles used for expiration by age (used in @c limitArticleAge archive mode)
            @return expiry age in days */
            int maxArticleAge() const;

            /** sets the maximum age of articles used for expiration by age (used in @c limitArticleAge archive mode)
            @param maxArticleAge expiry age in days */
            void setMaxArticleAge(int maxArticleAge);
            

            /** returns the article count limit used in @c limitArticleNumber archive mode **/
            int maxArticleNumber() const;

            /** sets the article count limit used in @c limitArticleNumber archive mode **/
            void setMaxArticleNumber(int maxArticleNumber);

            /** if @c true, new articles are marked immediately as read instead of new/unread. Useful for high-traffic feeds. */
            bool markImmediatelyAsRead() const;

            void setMarkImmediatelyAsRead(bool enabled);

            void setUseNotification(bool enabled);

            bool useNotification() const;

            /** if true, the linked URL is loaded directly in the article viewer instead of showing the description */
            void setLoadLinkedWebsite(bool enabled);

            bool loadLinkedWebsite() const;
            
            /** returns the favicon */
            const QPixmap& favicon() const;

            /** sets the favicon (used in the tree view) */
            void setFavicon(const QPixmap& p);

            /** returns the feed image */
            const QPixmap& image() const;

            /** returns the url of the actual feed source (rss/rdf/atom file) */
            const QString& xmlUrl() const;
            /** sets the url of the actual feed source (rss/rdf/atom file) */
            void setXmlUrl(const QString& s);

            /** returns the URL of the HTML page of this feed */
            const QString& htmlUrl() const;
            /** sets the URL of the HTML page of this feed */
            void setHtmlUrl(const QString& s);

            /** returns the description of this feed */
            const QString& description() const;

            /** sets the description of this feed */
            void setDescription(const QString& s);

            virtual QValueList<Article> articles(const QString& tag=QString::null);

            /** returns the article with the given @c guid, or a null article if it not exists */
            virtual Article findArticle(const QString& guid) const;
            
            virtual QStringList tags() const;
            
            /** returns whether a fetch error has occurred */
            bool fetchErrorOccurred();

            /** returns the unread count for this feed */
            virtual int unread() const;

            /** returns the number of total articles in this feed
            @return number of articles */

            virtual int totalCount() const;

            /** returns if the article archive of this feed is loaded */
            bool isArticlesLoaded() const;

           /** returns if this node is a feed group (@c false here) */
            virtual bool isGroup() const { return false; }

            /** returns the next node in the tree.
            Calling next() unless it returns 0 iterates through the tree in pre-order
            */
            virtual TreeNode* next();

            /** downloads the favicon */
            void loadFavicon();
            
            /** load the image from the cache if it is in there */
            void loadImage();
            
        public slots:
            /** starts fetching */
            void fetch(bool followDiscovery=false);

            void slotAbortFetch();

            /** deletes expired articles */
            virtual void slotDeleteExpiredArticles();

            /** mark all articles in this feed as read */
            virtual void slotMarkAllArticlesAsRead();

            /** add this feed to the fetch queue @c queue */
            virtual void slotAddToFetchQueue(FetchQueue* queue, bool intervalFetchOnly=false);

        signals:
            /** emitted when fetching started */
            void fetchStarted(Feed*);
            /** emitted when feed finished fetching */
            void fetched(Feed *);
            /** emitted when a fetch error occurred */
            void fetchError(Feed *);
            /** emitted when a feed URL was found by auto discovery */
            void fetchDiscovery(Feed *);
            /** emitted when a fetch is aborted */
            void fetchAborted(Feed *);
           
        protected:
            /** loads articles from archive **/
            void loadArticles();

            void recalcUnreadCount();

            virtual void doArticleNotification();

            /** sets the unread count for this feed */
            void setUnread(int unread);

            
        private slots:

            void fetchCompleted(Loader *loader, Document doc, Status status);
            void slotImageFetched(const QPixmap& image);

        private:
            
            /** notifies that article @c mya was set to "deleted".
                To be called by @ref Article
             */
            void setArticleDeleted(Article& a);

            /** notifies that article @c mya was changed
                @param oldStatus if the status was changed, it contains the old status, -1 otherwise
                To be called by @ref Article
             */
            void setArticleChanged(Article& a, int oldStatus=-1);
            
            void enforceLimitArticleNumber();

            void appendArticles(const RSS::Document &d);
            /** appends article @c a to the article list */
            void appendArticle(const Article& a);

            /** checks whether article @c a is expired (considering custom and global archive mode settings) */
            bool isExpired(const Article& a) const;

            /** returns @c true if either this article uses @c limitArticleAge as custom setting or uses the global default, which is @c limitArticleAge */
            bool usesExpiryByAge() const;

            /** executes the actual fetch action */
            void tryFetch();

            class FeedPrivate;
            FeedPrivate* d;
    };
}

#endif
