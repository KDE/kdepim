/*
  Copyright (c) 2014 Christian Mollekopf <mollekopf@kolabsys.com>

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

#include "tagwidgets.h"
#include <KJob>
#include <QLayout>

#include <AkonadiCore/Tag>
#include <AkonadiCore/TagFetchJob>
#include <AkonadiCore/TagFetchScope>
#include <AkonadiCore/TagCreateJob>
#include <AkonadiWidgets/TagWidget>
#include <AkonadiCore/TagModel>
#include <AkonadiCore/Monitor>
#include "libkdepim_debug.h"

using namespace KPIM;

class KPIM::TagWidgetPrivate
{
public:
    TagWidgetPrivate()
        : mTagWidget(Q_NULLPTR)
    {

    }

    Akonadi::TagWidget *mTagWidget;
    Akonadi::Tag::List mTagList;
    QStringList mCachedTagNames;
};

TagWidget::TagWidget(QWidget *parent)
    : QWidget(parent),
      d(new KPIM::TagWidgetPrivate)
{
    d->mTagWidget = new Akonadi::TagWidget(this);
    connect(d->mTagWidget, &Akonadi::TagWidget::selectionChanged, this, &TagWidget::onSelectionChanged);
    QHBoxLayout *l = new QHBoxLayout;
    l->setMargin(0);
    l->setSpacing(0);
    l->addWidget(d->mTagWidget);
    setLayout(l);
}

TagWidget::~TagWidget()
{
    delete d;
}

void TagWidget::onSelectionChanged(const Akonadi::Tag::List &tags)
{
    Q_UNUSED(tags);
    d->mCachedTagNames.clear();
    Q_FOREACH (const Akonadi::Tag &tag, d->mTagWidget->selection()) {
        d->mCachedTagNames << tag.name();
    }
    Q_EMIT selectionChanged(d->mCachedTagNames);
    Q_EMIT selectionChanged(tags);
}

void TagWidget::setSelection(const QStringList &tagNames)
{
    d->mTagList.clear();
    d->mCachedTagNames = tagNames;
    foreach (const QString &name, tagNames) {
        //TODO fetch by GID instead, we don't really want to create tags here
        Akonadi::TagCreateJob *tagCreateJob = new Akonadi::TagCreateJob(Akonadi::Tag::genericTag(name), this);
        tagCreateJob->setMergeIfExisting(true);
        connect(tagCreateJob, &Akonadi::TagCreateJob::result, this, &TagWidget::onTagCreated);
    }
}

void TagWidget::onTagCreated(KJob *job)
{
    if (job->error()) {
        qCWarning(LIBKDEPIM_LOG) << "Failed to create tag " << job->errorString();
        return;
    }
    Akonadi::TagCreateJob *createJob = static_cast<Akonadi::TagCreateJob *>(job);
    d->mTagList << createJob->tag();
    d->mTagWidget->setSelection(d->mTagList);
}

QStringList TagWidget::selection() const
{
    return d->mCachedTagNames;
}

class KPIM::TagSelectionDialogPrivate
{
public:
    TagSelectionDialogPrivate()
    {

    }

    Akonadi::Tag::List mTagList;
};
TagSelectionDialog::TagSelectionDialog(QWidget *parent)
    : Akonadi::TagSelectionDialog(parent),
      d(new KPIM::TagSelectionDialogPrivate)
{

}

TagSelectionDialog::~TagSelectionDialog()
{
    delete d;
}

void TagSelectionDialog::setSelection(const QStringList &tagNames)
{
    d->mTagList.clear();
    foreach (const QString &name, tagNames) {
        //TODO fetch by GID instead, we don't really want to create tags here
        Akonadi::TagCreateJob *tagCreateJob = new Akonadi::TagCreateJob(Akonadi::Tag::genericTag(name), this);
        tagCreateJob->setMergeIfExisting(true);
        connect(tagCreateJob, &Akonadi::TagCreateJob::result, this, &TagSelectionDialog::onTagCreated);
    }
}

void TagSelectionDialog::onTagCreated(KJob *job)
{
    if (job->error()) {
        qCWarning(LIBKDEPIM_LOG) << "Failed to create tag " << job->errorString();
        return;
    }
    Akonadi::TagCreateJob *createJob = static_cast<Akonadi::TagCreateJob *>(job);
    d->mTagList << createJob->tag();
    Akonadi::TagSelectionDialog::setSelection(d->mTagList);
}

QStringList TagSelectionDialog::selection() const
{
    QStringList list;
    const Akonadi::Tag::List lst = Akonadi::TagSelectionDialog::selection();
    list.reserve(lst.count());
    Q_FOREACH (const Akonadi::Tag &tag, lst) {
        list << tag.name();
    }
    return list;
}

Akonadi::Tag::List TagSelectionDialog::tagSelection() const
{
    return Akonadi::TagSelectionDialog::selection();
}

