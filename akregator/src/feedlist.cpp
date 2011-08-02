/*
    This file is part of Akregator.

    Copyright (C) 2004 Frank Osterfeld <osterfeld@kde.org>

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

#include "feedlist.h"
#include "storage.h"
#include "article.h"
#include "feed.h"
#include "folder.h"
#include "treenode.h"
#include "treenodevisitor.h"

#include "kernel.h"
#include "subscriptionlistjobs.h"

#include <kdebug.h>
#include <klocale.h>
#include <krandom.h>

#include <qdom.h>
#include <QHash>
#include <QSet>
#include <QTime>

#include <cassert>

using namespace boost;

namespace Akregator {

class FeedList::Private
{
    FeedList* const q;

public:
    Private( Backend::Storage* st, FeedList* qq );

    Akregator::Backend::Storage* storage;
    QList<TreeNode*> flatList;
    Folder* rootNode;
    QHash<int, TreeNode*> idMap;
    AddNodeVisitor* addNodeVisitor;
    RemoveNodeVisitor* removeNodeVisitor;
    QHash<QString, QList<Feed*> > urlMap;
    mutable int unreadCache;
};

class FeedList::AddNodeVisitor : public TreeNodeVisitor
{
    public:
        AddNodeVisitor(FeedList* list) : m_list(list) {}


        bool visitFeed(Feed* node)
        {
            m_list->d->idMap.insert(node->id(), node);
            m_list->d->flatList.append(node);
            m_list->d->urlMap[node->xmlUrl()].append(node);
            connect( node, SIGNAL(fetchStarted(Akregator::Feed*)),
                     m_list, SIGNAL(fetchStarted(Akregator::Feed*)) );
            connect( node, SIGNAL(fetched(Akregator::Feed*)),
                     m_list, SIGNAL(fetched(Akregator::Feed*)) );
            connect( node, SIGNAL(fetchAborted(Akregator::Feed*)),
                     m_list, SIGNAL(fetchAborted(Akregator::Feed*)) );
            connect( node, SIGNAL(fetchError(Akregator::Feed*)),
                     m_list, SIGNAL(fetchError(Akregator::Feed*)) );
            connect( node, SIGNAL(fetchDiscovery(Akregator::Feed*)),
                     m_list, SIGNAL(fetchDiscovery(Akregator::Feed*)) );


            visitTreeNode(node);
            return true;
        }

        void visit(TreeNode* node, bool preserveID)
        {
            m_preserveID = preserveID;
            TreeNodeVisitor::visit(node);
        }

        bool visitTreeNode(TreeNode* node)
        {
            if (!m_preserveID)
            	node->setId(m_list->generateID());
            m_list->d->idMap[node->id()] = node;
            m_list->d->flatList.append(node);

            connect(node, SIGNAL(signalDestroyed(Akregator::TreeNode*)), m_list, SLOT(slotNodeDestroyed(Akregator::TreeNode*)));
            connect( node, SIGNAL(signalChanged(Akregator::TreeNode*)), m_list, SIGNAL(signalNodeChanged(Akregator::TreeNode*)) );
            emit m_list->signalNodeAdded(node);

            return true;
        }

        bool visitFolder(Folder* node)
        {
            connect(node, SIGNAL(signalChildAdded(Akregator::TreeNode*)), m_list, SLOT(slotNodeAdded(Akregator::TreeNode*)));
            connect( node, SIGNAL(signalAboutToRemoveChild(Akregator::TreeNode*)), m_list, SIGNAL(signalAboutToRemoveNode(Akregator::TreeNode*)) );
            connect(node, SIGNAL(signalChildRemoved(Akregator::Folder*,Akregator::TreeNode*)), m_list, SLOT(slotNodeRemoved(Akregator::Folder*,Akregator::TreeNode*)));

            visitTreeNode(node);

            for (TreeNode* i = node->firstChild(); i && i != node; i = i->next() )
                m_list->slotNodeAdded(i);

            return true;
        }

    private:
        FeedList* m_list;
        bool m_preserveID;
};

class FeedList::RemoveNodeVisitor : public TreeNodeVisitor
{
    public:
        RemoveNodeVisitor(FeedList* list) : m_list(list) {}

        bool visitFeed(Feed* node)
        {
            visitTreeNode( node );
            m_list->d->urlMap[node->xmlUrl()].removeAll(node);
            return true;
        }

        bool visitTreeNode(TreeNode* node)
        {
            m_list->d->idMap.remove(node->id());
            m_list->d->flatList.removeAll(node);
            m_list->disconnect( node );
            return true;
        }

        bool visitFolder(Folder* node)
        {
            visitTreeNode(node);

            return true;
        }

    private:
        FeedList* m_list;
};

FeedList::Private::Private( Backend::Storage* st, FeedList* qq )
    : q( qq )
    , storage( st )
    , rootNode( 0 )
    , addNodeVisitor( new AddNodeVisitor( q ) )
    , removeNodeVisitor( new RemoveNodeVisitor( q ) )
    , unreadCache( -1 ) {
    Q_ASSERT( storage );
}

FeedList::FeedList( Backend::Storage* storage )
    : QObject( 0 ), d( new Private( storage, this ) ) {
    Folder* rootNode = new Folder( i18n("All Feeds") );
    rootNode->setId( 1 );
    setRootNode( rootNode );
    addNode( rootNode, true );
}

QVector<int> FeedList::feedIds() const
{
    QVector<int> ids;
    Q_FOREACH ( const Feed* const i, feeds() )
        ids += i->id();
    return ids;
}

QVector<const Feed*> FeedList::feeds() const
{
    QVector<const Feed*> constList;
    Q_FOREACH( const Feed* const i, d->rootNode->feeds() )
        constList.append( i );
    return constList;
}

QVector<Feed*> FeedList::feeds()
{
    return d->rootNode->feeds();
}

QVector<const Folder*> FeedList::folders() const
{
    QVector<const Folder*> constList;
    Q_FOREACH( const Folder* const i, d->rootNode->folders() )
        constList.append( i );
    return constList;
}

QVector<Folder*> FeedList::folders()
{
    return d->rootNode->folders();
}

void FeedList::addNode(TreeNode* node, bool preserveID)
{
    d->addNodeVisitor->visit(node, preserveID);
}

void FeedList::removeNode(TreeNode* node)
{
    d->removeNodeVisitor->visit(node);
}

void FeedList::parseChildNodes(QDomNode &node, Folder* parent)
{
    QDomElement e = node.toElement(); // try to convert the node to an element.

    if( !e.isNull() )
    {
        QString title = e.hasAttribute("text") ? e.attribute("text") : e.attribute("title");

        if (e.hasAttribute("xmlUrl") || e.hasAttribute("xmlurl") || e.hasAttribute("xmlURL") )
        {
            Feed* feed = Feed::fromOPML(e, d->storage);
            if (feed)
            {
                if (!d->urlMap[feed->xmlUrl()].contains(feed))
                    d->urlMap[feed->xmlUrl()].append(feed);
                parent->appendChild(feed);
            }
        }
        else
        {
            Folder* fg = Folder::fromOPML(e);
            parent->appendChild(fg);

            if (e.hasChildNodes())
            {
                QDomNode child = e.firstChild();
                while(!child.isNull())
                {
                    parseChildNodes(child, fg);
                    child = child.nextSibling();
                }
            }
        }
    }
}

bool FeedList::readFromOpml(const QDomDocument& doc)
{
    QDomElement root = doc.documentElement();

    kDebug() <<"loading OPML feed" << root.tagName().toLower();

    kDebug() <<"measuring startup time: START";
    QTime spent;
    spent.start();

    if (root.tagName().toLower() != "opml")
    {
        return false;
    }
    QDomNode bodyNode = root.firstChild();

    while (!bodyNode.isNull() && bodyNode.toElement().tagName().toLower() != "body")
        bodyNode = bodyNode.nextSibling();


    if (bodyNode.isNull())
    {
        kDebug() <<"Failed to acquire body node, markup broken?";
        return false;
    }

    QDomElement body = bodyNode.toElement();

    QDomNode i = body.firstChild();

    while( !i.isNull() )
    {
        parseChildNodes(i, allFeedsFolder());
        i = i.nextSibling();
    }

    for (TreeNode* i = allFeedsFolder()->firstChild(); i && i != allFeedsFolder(); i = i->next() )
        if (i->id() == 0)
    {
            uint id = generateID();
            i->setId(id);
            d->idMap.insert(id, i);
    }

    kDebug() <<"measuring startup time: STOP," << spent.elapsed() <<"ms";
    kDebug() <<"Number of articles loaded:" << allFeedsFolder()->totalCount();
    return true;
}

FeedList::~FeedList()
{
    emit signalDestroyed(this);
    setRootNode(0);
    delete d->addNodeVisitor;
    delete d->removeNodeVisitor;
    delete d;
}

const Feed* FeedList::findByURL(const QString& feedURL) const
{
    if ( !d->urlMap.contains( feedURL ) )
        return 0;
    const QList<Feed*>& v = d->urlMap[feedURL];
    return !v.isEmpty() ? v.front() : 0;
}

Feed* FeedList::findByURL(const QString& feedURL)
{
    if ( !d->urlMap.contains( feedURL ) )
        return 0;
    const QList<Feed*>& v = d->urlMap[feedURL];
    return !v.isEmpty() ? v.front() : 0;
}

const Article FeedList::findArticle(const QString& feedURL, const QString& guid) const
{
    const Feed* feed = findByURL(feedURL);
    return feed ? feed->findArticle(guid) : Article();
}

void FeedList::append(FeedList* list, Folder* parent, TreeNode* after)
{
    if ( list == this )
        return;

    if ( !d->flatList.contains(parent) )
        parent = allFeedsFolder();

    QList<TreeNode*> children = list->allFeedsFolder()->children();

    QList<TreeNode*>::ConstIterator end(  children.constEnd() );
    for (QList<TreeNode*>::ConstIterator it = children.constBegin(); it != end; ++it)
    {
        list->allFeedsFolder()->removeChild(*it);
        parent->insertChild(*it, after);
        after = *it;
    }
}

QDomDocument FeedList::toOpml() const
{
    QDomDocument doc;
    doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );

    QDomElement root = doc.createElement( "opml" );
    root.setAttribute( "version", "1.0" );
    doc.appendChild( root );

    QDomElement head = doc.createElement( "head" );
    root.appendChild( head );

    QDomElement ti = doc.createElement( "text" );
    head.appendChild( ti );

    QDomElement body = doc.createElement( "body" );
    root.appendChild( body );

    foreach( const TreeNode* const i, allFeedsFolder()->children() )
        body.appendChild( i->toOPML(body, doc) );

    return doc;
}

const TreeNode* FeedList::findByID(int id) const
{
    return d->idMap[id];
}

TreeNode* FeedList::findByID(int id)
{
    return d->idMap[id];
}

QList<const TreeNode*> FeedList::findByTitle(const QString& title ) const
{
    return allFeedsFolder()->namedChildren( title );
}

QList<TreeNode*> FeedList::findByTitle(const QString& title )
{
    return allFeedsFolder()->namedChildren( title );
}

const Folder* FeedList::allFeedsFolder() const
{
    return d->rootNode;
}

Folder* FeedList::allFeedsFolder()
{
    return d->rootNode;
}

bool FeedList::isEmpty() const
{
    return d->rootNode->firstChild() == 0;
}

void FeedList::rootNodeChanged() {
    assert( d->rootNode );
    const int newUnread = d->rootNode->unread();
    if ( newUnread == d->unreadCache )
        return;
    d->unreadCache = newUnread;
    emit unreadCountChanged( newUnread );
}

void FeedList::setRootNode(Folder* folder)
{
    if ( folder == d->rootNode )
        return;

    delete d->rootNode;
    d->rootNode = folder;
    d->unreadCache = -1;

    if ( d->rootNode ) {
        d->rootNode->setOpen(true);
        connect(d->rootNode, SIGNAL(signalChildAdded(Akregator::TreeNode*)), this, SLOT(slotNodeAdded(Akregator::TreeNode*)));
        connect(d->rootNode, SIGNAL(signalAboutToRemoveChild(Akregator::TreeNode*)), this, SIGNAL(signalAboutToRemoveNode(Akregator::TreeNode*)));
        connect(d->rootNode, SIGNAL(signalChildRemoved(Akregator::Folder*,Akregator::TreeNode*)), this, SLOT(slotNodeRemoved(Akregator::Folder*,Akregator::TreeNode*)));
        connect( d->rootNode, SIGNAL(signalChanged(Akregator::TreeNode*)), this, SIGNAL(signalNodeChanged(Akregator::TreeNode*)) );
        connect( d->rootNode, SIGNAL(signalChanged(Akregator::TreeNode*)), this, SLOT(rootNodeChanged()) );
    }
}

int FeedList::generateID() const
{
    return KRandom::random();
}

void FeedList::slotNodeAdded(TreeNode* node)
{
    if (!node) return;

    Folder* parent = node->parent();
    if ( !parent || !d->flatList.contains(parent) || d->flatList.contains(node) )
        return;

    addNode(node, false);
}

void FeedList::slotNodeDestroyed(TreeNode* node)
{
    if ( !node || !d->flatList.contains(node) )
        return;
    removeNode(node);
}

void FeedList::slotNodeRemoved(Folder* /*parent*/, TreeNode* node)
{
    if ( !node || !d->flatList.contains(node) )
        return;
    removeNode(node);
    emit signalNodeRemoved( node );
}

int FeedList::unread() const {
    if ( d->unreadCache == -1 )
        d->unreadCache = d->rootNode ? d->rootNode->unread() : 0;
    return d->unreadCache;
}

void FeedList::addToFetchQueue( FetchQueue* qu, bool intervalOnly ) {
    if ( d->rootNode )
        d->rootNode->slotAddToFetchQueue( qu, intervalOnly );
}

KJob* FeedList::createMarkAsReadJob() {
    return d->rootNode ? d->rootNode->createMarkAsReadJob() : 0;
}

FeedListManagementImpl::FeedListManagementImpl( const shared_ptr<FeedList>& list ) : m_feedList( list )
{

}

void FeedListManagementImpl::setFeedList( const shared_ptr<FeedList>& list )
{
    m_feedList = list;
}

static QString path_of_folder( const Folder* fol )
{
    assert( fol );
    QString path;
    const Folder* i = fol;
    while ( i ) {
        path = QString::number( i->id() ) + '/' + path;
        i = i->parent();
    }
    return path;
}

QStringList FeedListManagementImpl::categories() const
{
    if ( !m_feedList )
        return QStringList();
    QStringList cats;
    Q_FOREACH ( const Folder* const i, m_feedList->folders() )
        cats.append( path_of_folder( i ) );
    return cats;
}

QStringList FeedListManagementImpl::feeds( const QString& catId ) const
{
    if ( !m_feedList )
        return QStringList();

    uint lastcatid = catId.split('/',QString::SkipEmptyParts).last().toUInt();

    QSet<QString> urls;
    Q_FOREACH ( const Feed* const i, m_feedList->feeds() ) {
        if ( lastcatid == i->parent()->id() ) {
            urls.insert( i->xmlUrl() );
        }
    }
    return urls.toList();
}

void FeedListManagementImpl::addFeed( const QString& url, const QString& catId )
{
    if ( !m_feedList )
        return;

    kDebug() << "Name:" << url.left(20) << "Cat:" << catId;
    uint folder_id = catId.split('/',QString::SkipEmptyParts).last().toUInt();

    // Get the folder
    Folder * m_folder = 0;
    QVector<Folder*> vector = m_feedList->folders();
    for (int i = 0; i < vector.size(); ++i) {
        if (vector.at(i)->id() == folder_id) {
            m_folder = vector.at(i);
            i = vector.size();
        }
    }

    // Create new feed
    std::auto_ptr<FeedList> new_feedlist( new FeedList( Kernel::self()->storage() ) );
    Feed * new_feed = new Feed( Kernel::self()->storage() );
    new_feed->setXmlUrl(url);
    // new_feed->setTitle(url);
    new_feedlist->allFeedsFolder()->appendChild(new_feed);

    // Get last in the folder
    TreeNode* m_last = m_folder->childAt( m_folder->totalCount() );

    // Add the feed
    m_feedList->append(new_feedlist.get(), m_folder, m_last);
}

void FeedListManagementImpl::removeFeed( const QString& url, const QString& catId )
{
    kDebug() << "Name:" << url.left(20) << "Cat:" << catId;

    uint lastcatid = catId.split('/',QString::SkipEmptyParts).last().toUInt();

    Q_FOREACH ( const Feed* const i, m_feedList->feeds() ) {
        if ( lastcatid == i->parent()->id() ) {
            if (i->xmlUrl().compare(url)==0) {
                kDebug() << "id:" << i->id();
                DeleteSubscriptionJob* job = new DeleteSubscriptionJob;
                job->setSubscriptionId( i->id() );
                job->start();
            }
        }
    }
}

QString FeedListManagementImpl::addCategory( const QString& name, const QString& parentId ) const
{
    Q_UNUSED( parentId )

    if ( !m_feedList )
        return "";

    Folder * m_folder = new Folder(name);
    m_feedList->allFeedsFolder()->appendChild(m_folder);

    return QString::number(m_folder->id());
}

QString FeedListManagementImpl::getCategoryName( const QString& catId ) const
{
    QString catname;

    if ( !m_feedList )
        return catname;

    QStringList list = catId.split('/',QString::SkipEmptyParts);
    for (int i=0;i<list.size();i++) {
        int index = list.at(i).toInt();
        catname += m_feedList->findByID(index)->title() + '/';
    }

    return catname;
}

} // namespace Akregator

#include "feedlist.moc"
