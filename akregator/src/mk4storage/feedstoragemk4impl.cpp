/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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

#include "feedstoragemk4impl.h"
#include "storagemk4impl.h"

#include "../article.h"
#include "../utils.h"
#include "../librss/article.h"
#include "../librss/document.h"
#include <mk4.h>

#include <tqdom.h>
#include <tqfile.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>

namespace Akregator {
namespace Backend {

class FeedStorageMK4Impl::FeedStorageMK4ImplPrivate
{
    public:
        FeedStorageMK4ImplPrivate() :
            modified(false),
            pguid("guid"),
            ptitle("title"),
            pdescription("description"),
            plink("link"),
            pcommentsLink("commentsLink"),
            ptag("tag"),
            pEnclosureType("enclosureType"),
            pEnclosureUrl("enclosureUrl"),
            pcatTerm("catTerm"), 
            pcatScheme("catScheme"), 
            pcatName("catName"),
            pauthor("author"),
            phash("hash"),
            pguidIsHash("guidIsHash"),
            pguidIsPermaLink("guidIsPermaLink"),
            pcomments("comments"),
            pstatus("status"),
            ppubDate("pubDate"),
            pHasEnclosure("hasEnclosure"),
            pEnclosureLength("enclosureLength"),
            ptags("tags"),
            ptaggedArticles("taggedArticles"),
            pcategorizedArticles("categorizedArticles"),
            pcategories("categories")
        {}
            
        TQString url;
        c4_Storage* storage;
        StorageMK4Impl* mainStorage;
        c4_View archiveView;
        
        c4_Storage* catStorage;
        c4_View catView;
        c4_Storage* tagStorage;
        c4_View tagView;
        bool autoCommit;
        bool modified;
        bool taggingEnabled;
        bool convert;
        TQString oldArchivePath;
        c4_StringProp pguid, ptitle, pdescription, plink, pcommentsLink, ptag, pEnclosureType, pEnclosureUrl, pcatTerm, pcatScheme, pcatName, pauthor;
        c4_IntProp phash, pguidIsHash, pguidIsPermaLink, pcomments, pstatus, ppubDate, pHasEnclosure, pEnclosureLength;
        c4_ViewProp ptags, ptaggedArticles, pcategorizedArticles, pcategories;
};

void FeedStorageMK4Impl::convertOldArchive()
{
    if (!d->convert)
        return;

    d->convert = false;    
    TQFile file(d->oldArchivePath);

    if ( !file.open(IO_ReadOnly) )
        return;
    
    TQTextStream stream(&file);
    stream.setEncoding(TQTextStream::UnicodeUTF8);
    TQString data=stream.read();
    TQDomDocument xmldoc;
    
    if (!xmldoc.setContent(data))
        return;
    
    RSS::Document doc(xmldoc);

    RSS::Article::List d_articles = doc.articles();
    RSS::Article::List::ConstIterator it = d_articles.begin();
    RSS::Article::List::ConstIterator en = d_articles.end();
    
    int unr = 0;
    for (; it != en; ++it)
    {
        Article a(*it, this);
        if (a.status() != Article::Read)
            unr++;
    }
    
    setUnread(unr);
    markDirty();
    commit();
}

FeedStorageMK4Impl::FeedStorageMK4Impl(const TQString& url, StorageMK4Impl* main)
{
    d = new FeedStorageMK4ImplPrivate;
    d->autoCommit = main->autoCommit();
    d->url = url;
    d->mainStorage = main;
    d->taggingEnabled = main->taggingEnabled();
    
    TQString url2 = url;

    if (url.length() > 255)
    {
        url2 = url.left(200) + TQString::number(Akregator::Utils::calcHash(url), 16);
    }
    
    kdDebug() << url2 << endl;
    TQString t = url2;
    TQString t2 = url2;
    TQString filePath = main->archivePath() +"/"+ t.replace("/", "_").replace(":", "_");
    d->oldArchivePath = KGlobal::dirs()->saveLocation("data", "akregator/Archive/") + t2.replace("/", "_").replace(":", "_") + ".xml";

    d->convert = !TQFile::exists(filePath + ".mk4") && TQFile::exists(d->oldArchivePath);
    d->storage = new c4_Storage((filePath + ".mk4").local8Bit(), true);

    d->archiveView = d->storage->GetAs("articles[guid:S,title:S,hash:I,guidIsHash:I,guidIsPermaLink:I,description:S,link:S,comments:I,commentsLink:S,status:I,pubDate:I,tags[tag:S],hasEnclosure:I,enclosureUrl:S,enclosureType:S,enclosureLength:I,categories[catTerm:S,catScheme:S,catName:S],author:S]");

    c4_View hash = d->storage->GetAs("archiveHash[_H:I,_R:I]");
    d->archiveView = d->archiveView.Hash(hash, 1); // hash on guid

    d->tagStorage = 0;

    if (d->taggingEnabled)
    {
        d->tagStorage = new c4_Storage((filePath + ".mk4___TAGS").local8Bit(), true);
        d->tagView = d->tagStorage->GetAs("tagIndex[tag:S,taggedArticles[guid:S]]");
        hash = d->tagStorage->GetAs("archiveHash[_H:I,_R:I]");
        d->tagView = d->tagView.Hash(hash, 1); // hash on tag
    }
    //d->catStorage = new c4_Storage((filePath + ".mk4___CATEGORIES").local8Bit(), true);
    //d->catView = d->catStorage->GetAs("catIndex[catTerm:S,catScheme:S,catName:S,categorizedArticles[guid:S]]");
}


FeedStorageMK4Impl::~FeedStorageMK4Impl()
{
    delete d->storage;
    if (d->taggingEnabled)
        delete d->tagStorage;
//    delete d->catStorage;
    delete d; d = 0;
}

void FeedStorageMK4Impl::markDirty()
{
    if (!d->modified)
    {
        d->modified = true;
        // Tell this to mainStorage
        d->mainStorage->markDirty();
    }
}

void FeedStorageMK4Impl::commit()
{
    if (d->modified)
    {
        d->storage->Commit();
        if (d->taggingEnabled)
            d->tagStorage->Commit();
//        d->catStorage->Commit();
    }
    d->modified = false;
}

void FeedStorageMK4Impl::rollback()
{
    d->storage->Rollback();
    if (d->taggingEnabled)
        d->tagStorage->Rollback();
//    d->catStorage->Rollback();
}

void FeedStorageMK4Impl::close()
{
    if (d->autoCommit)
        commit();
}
int FeedStorageMK4Impl::unread()
{
    return d->mainStorage->unreadFor(d->url);
}
void FeedStorageMK4Impl::setUnread(int unread)
{
    d->mainStorage->setUnreadFor(d->url, unread);
}

int FeedStorageMK4Impl::totalCount()
{
    return d->mainStorage->totalCountFor(d->url);
}

void FeedStorageMK4Impl::setTotalCount(int total)
{
    d->mainStorage->setTotalCountFor(d->url, total);
}

int FeedStorageMK4Impl::lastFetch()
{
    return d->mainStorage->lastFetchFor(d->url);
}

void FeedStorageMK4Impl::setLastFetch(int lastFetch)
{
    d->mainStorage->setLastFetchFor(d->url, lastFetch);
}

TQStringList FeedStorageMK4Impl::articles(const TQString& tag)
{
    TQStringList list;
    if (tag.isNull()) // return all articles
    {
        int size = d->archiveView.GetSize();
        for (int i = 0; i < size; i++) // fill with guids
            list += TQString(d->pguid(d->archiveView.GetAt(i)));
    }
    else if (d->taggingEnabled)
    {
        c4_Row tagrow;
        d->ptag(tagrow) = tag.utf8().data();
        int tagidx = d->tagView.Find(tagrow);
        if (tagidx != -1)
        {
            tagrow = d->tagView.GetAt(tagidx);
            c4_View tagView = d->ptaggedArticles(tagrow);
            int size = tagView.GetSize();
            for (int i = 0; i < size; i++)
                list += TQString(d->pguid(tagView.GetAt(i)));
        }

    }
    return list;
}

TQStringList FeedStorageMK4Impl::articles(const Category& cat)
{
    TQStringList list;
    /*
    c4_Row catrow;
    d->pcatTerm(catrow) = cat.term.utf8().data();
    d->pcatScheme(catrow) = cat.scheme.utf8().data();

    int catidx = d->catView.Find(catrow);
    if (catidx != -1)
    {
        catrow = d->catView.GetAt(catidx);
        c4_View catView = d->pcategorizedArticles(catrow);
        int size = catView.GetSize();
        for (int i = 0; i < size; i++)
            list += TQString(d->pguid(catView.GetAt(i)));
    }
    */
    return list;
}

void FeedStorageMK4Impl::addEntry(const TQString& guid)
{
    c4_Row row;
    d->pguid(row) = guid.ascii();
    if (!contains(guid))
    {
        d->archiveView.Add(row);
        markDirty();
        setTotalCount(totalCount()+1);
    }
}

bool FeedStorageMK4Impl::contains(const TQString& guid)
{
    return findArticle(guid) != -1;
}

int FeedStorageMK4Impl::findArticle(const TQString& guid)
{
    c4_Row findrow;
    d->pguid(findrow) = guid.ascii();
    return d->archiveView.Find(findrow);
}

void FeedStorageMK4Impl::deleteArticle(const TQString& guid)
{

    int findidx = findArticle(guid);
    if (findidx != -1)
    {
        TQStringList list = tags(guid);
        for (TQStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
            removeTag(guid, *it);
        setTotalCount(totalCount()-1);
        d->archiveView.RemoveAt(findidx);
        markDirty();
    }   
}

int FeedStorageMK4Impl::comments(const TQString& guid)
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->pcomments(d->archiveView.GetAt(findidx)) : 0;
}

TQString FeedStorageMK4Impl::commentsLink(const TQString& guid)
{
   int findidx = findArticle(guid);
   return findidx != -1 ? TQString(d->pcommentsLink(d->archiveView.GetAt(findidx))) : "";
}

bool FeedStorageMK4Impl::guidIsHash(const TQString& guid)
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->pguidIsHash(d->archiveView.GetAt(findidx)) : false;
}

bool FeedStorageMK4Impl::guidIsPermaLink(const TQString& guid)
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->pguidIsPermaLink(d->archiveView.GetAt(findidx)) : false;
}

uint FeedStorageMK4Impl::hash(const TQString& guid)
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->phash(d->archiveView.GetAt(findidx)) : 0;
}


void FeedStorageMK4Impl::setDeleted(const TQString& guid)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;

    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    TQStringList list = tags(guid);
        for (TQStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
            removeTag(guid, *it);
    d->pdescription(row) = "";
    d->ptitle(row) = "";
    d->plink(row) = "";
    d->pauthor(row) = "";
    d->pcommentsLink(row) = "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

TQString FeedStorageMK4Impl::link(const TQString& guid)
{
    int findidx = findArticle(guid);
    return findidx != -1 ? TQString(d->plink(d->archiveView.GetAt(findidx))) : "";
}

uint FeedStorageMK4Impl::pubDate(const TQString& guid)
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->ppubDate(d->archiveView.GetAt(findidx)) : 0;
}

int FeedStorageMK4Impl::status(const TQString& guid)
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->pstatus(d->archiveView.GetAt(findidx)) : 0;
}

void FeedStorageMK4Impl::setStatus(const TQString& guid, int status)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pstatus(row) = status;
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

TQString FeedStorageMK4Impl::title(const TQString& guid)
{
    int findidx = findArticle(guid);
    return findidx != -1 ? TQString::fromUtf8(d->ptitle(d->archiveView.GetAt(findidx))) : "";
}

TQString FeedStorageMK4Impl::description(const TQString& guid)
{
    int findidx = findArticle(guid);
    return findidx != -1 ? TQString::fromUtf8(d->pdescription(d->archiveView.GetAt(findidx))) : "";
}


void FeedStorageMK4Impl::setPubDate(const TQString& guid, uint pubdate)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->ppubDate(row) = pubdate;
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::setGuidIsHash(const TQString& guid, bool isHash)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pguidIsHash(row) = isHash;
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::setLink(const TQString& guid, const TQString& link)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->plink(row) = !link.isEmpty() ? link.ascii() : "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::setHash(const TQString& guid, uint hash)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->phash(row) = hash;
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::setTitle(const TQString& guid, const TQString& title)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->ptitle(row) = !title.isEmpty() ? title.utf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::setDescription(const TQString& guid, const TQString& description)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pdescription(row) = !description.isEmpty() ? description.utf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::setAuthor(const TQString& guid, const TQString& author)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pauthor(row) = !author.isEmpty() ? author.utf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

TQString FeedStorageMK4Impl::author(const TQString& guid)
{
    int findidx = findArticle(guid);
    return findidx != -1 ? TQString::fromUtf8(d->pauthor(d->archiveView.GetAt(findidx))) : "";
}

        
void FeedStorageMK4Impl::setCommentsLink(const TQString& guid, const TQString& commentsLink)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pcommentsLink(row) = !commentsLink.isEmpty() ? commentsLink.utf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::setComments(const TQString& guid, int comments)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pcomments(row) = comments;
    d->archiveView.SetAt(findidx, row);
    markDirty();
}


void FeedStorageMK4Impl::setGuidIsPermaLink(const TQString& guid, bool isPermaLink)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pguidIsPermaLink(row) = isPermaLink;
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::addCategory(const TQString& /*guid*/, const Category& /*cat*/)
{
    return;
    /*
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    c4_View catView = d->pcategories(row);
    c4_Row findrow; 

    d->pcatTerm(findrow) = cat.term.utf8().data();
    d->pcatScheme(findrow) = cat.scheme.utf8().data();

    int catidx = catView.Find(findrow);
    if (catidx == -1)
    {
        d->pcatName(findrow) = cat.name.utf8().data();
        catidx = catView.Add(findrow);
        d->pcategories(row) = catView;
        d->archiveView.SetAt(findidx, row);

        // add to category->articles index
        c4_Row catrow;
        d->pcatTerm(catrow) = cat.term.utf8().data();
        d->pcatScheme(catrow) = cat.scheme.utf8().data();
        d->pcatName(catrow) = cat.name.utf8().data();

        int catidx2 = d->catView.Find(catrow);

        if (catidx2 == -1)
        {
            catidx2 = d->catView.Add(catrow);
        }

        c4_Row catrow2 = d->catView.GetAt(catidx2);
        c4_View catView2 = d->pcategorizedArticles(catrow2);

        c4_Row row3;
        d->pguid(row3) = guid.ascii();
        int guididx = catView2.Find(row3);
        if (guididx == -1)
        {
            guididx = catView2.Add(row3);
            catView2.SetAt(guididx, row3);
            d->pcategorizedArticles(catrow2) = catView2;
            d->catView.SetAt(catidx2, catrow2);
        }

        markDirty();
    } 
    */
}

TQValueList<Category> FeedStorageMK4Impl::categories(const TQString& /*guid*/)
{

    TQValueList<Category> list;
    return list;
    /*  
    if (!guid.isNull()) // return categories for an article
    {
        int findidx = findArticle(guid);
        if (findidx == -1)
            return list;
            
        c4_Row row;
        row = d->archiveView.GetAt(findidx);
        c4_View catView = d->pcategories(row);
        int size = catView.GetSize();
        
        for (int i = 0; i < size; ++i)
        {
            Category cat;

            cat.term = TQString::fromUtf8(d->pcatTerm(catView.GetAt(i)));
            cat.scheme = TQString::fromUtf8(d->pcatScheme(catView.GetAt(i)));
            cat.name = TQString::fromUtf8(d->pcatName(catView.GetAt(i)));

            list += cat;
        }
    }
    else // return all categories in the feed
    {
        int size = d->catView.GetSize();
        for (int i = 0; i < size; i++)
        {
            c4_Row row = d->catView.GetAt(i);

            Category cat;
            cat.term = TQString(d->pcatTerm(row));
            cat.scheme = TQString(d->pcatScheme(row));
            cat.name = TQString(d->pcatName(row));

            list += cat;
        }
    }
    
    return list;*/
}

void FeedStorageMK4Impl::addTag(const TQString& guid, const TQString& tag)
{
    if (!d->taggingEnabled)
        return;
    
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;

    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    c4_View tagView = d->ptags(row);
    c4_Row findrow; 
    d->ptag(findrow) = tag.utf8().data();
    int tagidx = tagView.Find(findrow);
    if (tagidx == -1)
    {
        tagidx = tagView.Add(findrow);
        d->ptags(row) = tagView;
        d->archiveView.SetAt(findidx, row);

        // add to tag->articles index
        c4_Row tagrow;
        d->ptag(tagrow) = tag.utf8().data();
        int tagidx2 = d->tagView.Find(tagrow);
        if (tagidx2 == -1)
            tagidx2 = d->tagView.Add(tagrow);
        tagrow = d->tagView.GetAt(tagidx2);
        c4_View tagView2 = d->ptaggedArticles(tagrow);

        c4_Row row3;
        d->pguid(row3) = guid.ascii();
        int guididx = tagView2.Find(row3);
        if (guididx == -1)
        {
            guididx = tagView2.Add(row3);
            tagView2.SetAt(guididx, row3);
            d->ptaggedArticles(tagrow) = tagView2;
            d->tagView.SetAt(tagidx2, tagrow);
        }
        markDirty();
    } 
}

void FeedStorageMK4Impl::removeTag(const TQString& guid, const TQString& tag)
{
    if (!d->taggingEnabled)
        return;
    
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;

    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    c4_View tagView = d->ptags(row);
    c4_Row findrow; 
    d->ptag(findrow) = tag.utf8().data();
    int tagidx = tagView.Find(findrow);
    if (tagidx != -1)
    {
        tagView.RemoveAt(tagidx);
        d->ptags(row) = tagView;
        d->archiveView.SetAt(findidx, row);

        // remove from tag->articles index
        c4_Row tagrow;
        d->ptag(tagrow) = tag.utf8().data();
        int tagidx2 = d->tagView.Find(tagrow);
        if (tagidx2 != -1)
        {
            tagrow = d->tagView.GetAt(tagidx2);
            c4_View tagView2 = d->ptaggedArticles(tagrow);

            c4_Row row3;
            d->pguid(row3) = guid.ascii();
            int guididx = tagView2.Find(row3);
            if (guididx != -1)
            {
                tagView2.RemoveAt(guididx);
                d->ptaggedArticles(tagrow) = tagView2;
                d->tagView.SetAt(tagidx2, tagrow);
            }
        }
        
        markDirty();
    }
}

TQStringList FeedStorageMK4Impl::tags(const TQString& guid)
{
    TQStringList list;
    
    if (!d->taggingEnabled)
        return list;
    
    if (!guid.isNull()) // return tags for an articles
    {
        int findidx = findArticle(guid);
        if (findidx == -1)
            return list;
            
        c4_Row row;
        row = d->archiveView.GetAt(findidx);
        c4_View tagView = d->ptags(row);
        int size = tagView.GetSize();
        
        for (int i = 0; i < size; ++i)
            list += TQString::fromUtf8(d->ptag(tagView.GetAt(i)));
    }
    else // return all tags in the feed
    {
        int size = d->tagView.GetSize();
        for (int i = 0; i < size; i++)
             list += TQString(d->ptag(d->tagView.GetAt(i)));
    }
    
    return list;
}

void FeedStorageMK4Impl::add(FeedStorage* source)
{
    TQStringList articles = source->articles();
    for (TQStringList::ConstIterator it = articles.begin(); it != articles.end(); ++it)
        copyArticle(*it, source);
    setUnread(source->unread());
    setLastFetch(source->lastFetch());
    setTotalCount(source->totalCount());
}

void FeedStorageMK4Impl::copyArticle(const TQString& guid, FeedStorage* source)
{
    if (!contains(guid))
        addEntry(guid);
    setComments(guid, source->comments(guid));
    setCommentsLink(guid, source->commentsLink(guid));
    setDescription(guid, source->description(guid));
    setGuidIsHash(guid, source->guidIsHash(guid));
    setGuidIsPermaLink(guid, source->guidIsPermaLink(guid));
    setHash(guid, source->hash(guid));
    setLink(guid, source->link(guid));
    setPubDate(guid, source->pubDate(guid));
    setStatus(guid, source->status(guid));
    setTitle(guid, source->title(guid));
    setAuthor(guid, source->author(guid));

    TQStringList tags = source->tags(guid);
    for (TQStringList::ConstIterator it = tags.begin(); it != tags.end(); ++it)
        addTag(guid, *it);
}

void FeedStorageMK4Impl::setEnclosure(const TQString& guid, const TQString& url, const TQString& type, int length)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pHasEnclosure(row) = true;
    d->pEnclosureUrl(row) = !url.isEmpty() ? url.utf8().data() : "";
    d->pEnclosureType(row) = !type.isEmpty() ? type.utf8().data() : "";
    d->pEnclosureLength(row) = length;

    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::removeEnclosure(const TQString& guid)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pHasEnclosure(row) = false;
    d->pEnclosureUrl(row) = "";
    d->pEnclosureType(row) = "";
    d->pEnclosureLength(row) = -1;

    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::enclosure(const TQString& guid, bool& hasEnclosure, TQString& url, TQString& type, int& length)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
    {
        hasEnclosure = false;
        url = TQString::null;
        type = TQString::null;
        length = -1;
        return;
    }
    c4_Row row = d->archiveView.GetAt(findidx);
    hasEnclosure = d->pHasEnclosure(row);
    url = d->pEnclosureUrl(row);
    type = d->pEnclosureType(row);
    length = d->pEnclosureLength(row);
}

void FeedStorageMK4Impl::clear()
{
    d->storage->RemoveAll();
    if (d->taggingEnabled)
        d->tagStorage->RemoveAll();
    setUnread(0);
    markDirty();
}

} // namespace Backend
} // namespace Akregator
