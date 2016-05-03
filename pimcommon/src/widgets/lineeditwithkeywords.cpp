/*
  Copyright (c) 2016 Rebois Guillaume <guillaume.rebois@orange.fr>

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

#include "lineeditwithkeywords.h"
#include <KLocalizedString>

using namespace PimCommon;

class Q_DECL_HIDDEN LineEditWithKeywords::Private
{
public:
    Private(LineEditWithKeywords *owner)
        : q(owner)
    {
    }
    LineEditWithKeywords *const q;
    QStringList mKeywordList;
    void initKeywordList();
    void addKeywordList();

};

LineEditWithKeywords::LineEditWithKeywords(QWidget *parent)
    : LineEditWithCompleter(parent), d(new Private(this))
{
    d->initKeywordList();
    d->addKeywordList();
}

LineEditWithKeywords::~LineEditWithKeywords()
{
    delete d;
}

void LineEditWithKeywords::Private::initKeywordList()
{
    mKeywordList << i18n("in") << i18n("ago") << i18n("yesterday") << i18n("today") << i18n("first") << i18n("last") << i18n("next") << i18n("PM") << i18n("AM")
                << i18n("pm") << i18n("am") << i18n("at") << i18n("containing") << i18n("contains") << i18n("greater than") << i18n("bigger than") << i18n("more than")
                << i18n("at least") << i18n(">") << i18n("after") << i18n("since") << i18n("smaller than") << i18n("less than") << i18n("lesser than") << i18n("at most")
                << i18n("<") << i18n("Before") << i18n("until") << i18n("equal to") << i18n("equal") << i18n("equals") << i18n("=") << i18n("rated as") << i18n("rated")
                << i18n("score is") << i18n("score") << i18n("scored") << i18n("having") << i18n("stars") << i18n("star") << i18n("Comment") << i18n("described as")
                << i18n("description is") << i18n("comment is") << i18n("description") << i18n("described") << i18n("comment") << i18n("sent by") << i18n("from")
                << i18n("sender is") << i18n("sender") << i18n("title is") << i18n("subject is") << i18n("subject") << i18n("title") << i18n("titled") << i18n("sent to")
                << i18n("recipient is") << i18n("recipient") << i18n("sent at") << i18n("sent on") << i18n("sent") << i18n("received at") << i18n("received on")
                << i18n("received") << i18n("reception is") << i18n("written by") << i18n("created by") << i18n("composed by") << i18n("author is") << i18n("by")
                << i18n("size is") << i18n("size") << i18n("being") << i18n("large") << i18n("name is") << i18n("name") << i18n("named") << i18n("name is")
                << i18n("created at") << i18n("dated at") << i18n("created on") << i18n("dated on") << i18n("created in") << i18n("dated in") << i18n("created of")
                << i18n("dated of") << i18n("created") << i18n("dated") << i18n("creation date is") << i18n("creation time is") << i18n("creation datetime is")
                << i18n("modification date is") << i18n("modification time is") << i18n("modification datetime is") << i18n("edition date is") << i18n("edition time is")
                << i18n("edition datetime is") << i18n("edited") << i18n("modified") << i18n("modified at") << i18n("modified on") << i18n("edited on") << i18n("edited at")
                << i18n("tagged as") << i18n("has tag") << i18n("tag is") << i18n("#") << i18n("related to") << i18n("a") << i18n("year") << i18n("week") << i18n("month")
                << i18n("day") << i18n("hour") << i18n("of") << i18n("by") << i18n("and") << i18n("on") << i18n("in") << i18n("or") << i18n("third") << i18n("second") << i18n("fourth")
                << i18n("then");
}

void LineEditWithKeywords::Private::addKeywordList()
{
    Q_FOREACH(const QString &str, mKeywordList) {
        q->completionObject()->addItem(str);
    }
}

void LineEditWithKeywords::slotClearHistory()
{
    LineEditWithCompleter::slotClearHistory();
    d->addKeywordList();
}
