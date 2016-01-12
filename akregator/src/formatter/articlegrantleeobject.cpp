/*
  Copyright (c) 2016 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "articlegrantleeobject.h"
#include "utils.h"
#include <KLocale>
#include <KGlobal>
#include <QDateTime>
using namespace Akregator;

ArticleGrantleeObject::ArticleGrantleeObject(const Article &article, ArticleFormatter::IconOption iconOption, QObject *parent)
    : QObject(parent),
      mArticle(article),
      mArticleFormatOption(iconOption)
{

}

ArticleGrantleeObject::~ArticleGrantleeObject()
{

}

QString ArticleGrantleeObject::strippedTitle() const
{
    return Akregator::Utils::stripTags(mArticle.title());
}

QString ArticleGrantleeObject::author() const
{
    return mArticle.authorAsHtml();
}

QString ArticleGrantleeObject::content() const
{
    return mArticle.content(Article::DescriptionAsFallback);
}

QString ArticleGrantleeObject::articleLinkUrl() const
{
    return mArticle.link().url();
}

QString ArticleGrantleeObject::articlePubDate() const
{
    if (mArticle.pubDate().isValid()) {
        return KLocale::global()->formatDateTime(mArticle.pubDate(), KLocale::FancyLongDate);
    }
    return {};
}
