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
#include <QItemSelectionModel>

#include <Tag>
#include <TagFetchJob>
#include <TagFetchScope>
#include <TagCreateJob>
#include <TagWidget>
#include <TagModel>
#include <Monitor>
#include <KCheckableProxyModel>
#include <KLineEdit>
#include <KDebug>

using namespace KPIM;

TagWidget::TagWidget(QWidget* parent)
:   QWidget(parent)
{
    mTagWidget = new Akonadi::TagWidget(this);
    connect(mTagWidget, SIGNAL(selectionChanged(Akonadi::Tag::List)), this, SLOT(onSelectionChanged(Akonadi::Tag::List)));
    QHBoxLayout *l = new QHBoxLayout;
    l->addWidget(mTagWidget);
    setLayout(l);
}

void TagWidget::onSelectionChanged(const Akonadi::Tag::List &tags)
{
    Q_UNUSED(tags);
    mCachedTagNames.clear();
    Q_FOREACH (const Akonadi::Tag &tag, mTagWidget->selection()) {
        mCachedTagNames << tag.name();
    }
    emit selectionChanged(mCachedTagNames);
}

void TagWidget::setSelection(const QStringList &tagNames)
{
    mTagList.clear();
    mCachedTagNames = tagNames;
    foreach (const QString &name, tagNames) {
      //TODO fetch by GID instead, we don't really want to create tags here
      Akonadi::TagCreateJob *tagCreateJob = new Akonadi::TagCreateJob(Akonadi::Tag(name), this);
      tagCreateJob->setMergeIfExisting(true);
      connect(tagCreateJob, SIGNAL(result(KJob*)), this, SLOT(onTagCreated(KJob*)));
    }
}

void TagWidget::onTagCreated(KJob *job)
{
    if (job->error()) {
        kWarning() << "Failed to create tag " << job->errorString();
        return;
    }
    Akonadi::TagCreateJob *createJob = static_cast<Akonadi::TagCreateJob*>(job);
    mTagList << createJob->tag();
    mTagWidget->setSelection(mTagList);
}

QStringList TagWidget::selection() const
{
    return mCachedTagNames;
}


TagSelectionDialog::TagSelectionDialog(QWidget* parent)
:   Akonadi::TagSelectionDialog(parent)
{

}

void TagSelectionDialog::setSelection(const QStringList &tagNames)
{
    mTagList.clear();
    foreach (const QString &name, tagNames) {
      //TODO fetch by GID instead, we don't really want to create tags here
      Akonadi::TagCreateJob *tagCreateJob = new Akonadi::TagCreateJob(Akonadi::Tag(name), this);
      tagCreateJob->setMergeIfExisting(true);
      connect(tagCreateJob, SIGNAL(result(KJob*)), this, SLOT(onTagCreated(KJob*)));
    }
}

void TagSelectionDialog::onTagCreated(KJob *job)
{
    if (job->error()) {
        kWarning() << "Failed to create tag " << job->errorString();
        return;
    }
    Akonadi::TagCreateJob *createJob = static_cast<Akonadi::TagCreateJob*>(job);
    mTagList << createJob->tag();
    Akonadi::TagSelectionDialog::setSelection(mTagList);
}

QStringList TagSelectionDialog::selection() const
{
    QStringList list;
    Q_FOREACH (const Akonadi::Tag &tag, Akonadi::TagSelectionDialog::selection()) {
        list << tag.name();
    }
    return list;
}

class MatchingCheckableProxyModel : public KCheckableProxyModel
{
public:
    MatchingCheckableProxyModel(QObject* parent = 0): KCheckableProxyModel(parent) {}
    virtual QModelIndexList match(const QModelIndex& start, int role, const QVariant& value, int hits = 1, Qt::MatchFlags flags = Qt::MatchExactly) const
    {
        if (role == Qt::CheckStateRole) {
            return selectionModel()->selectedRows();
        }
        return KCheckableProxyModel::match(start, role, value, hits, flags);
    }
};

TagSelectionCombo::TagSelectionCombo(QWidget* parent)
:   KPIM::KCheckComboBox(parent)
{
    Akonadi::Monitor *monitor = new Akonadi::Monitor(this);
    monitor->setTypeMonitored(Akonadi::Monitor::Tags);

    Akonadi::TagModel *model = new Akonadi::TagModel(monitor, this);

    QItemSelectionModel *selectionModel = new QItemSelectionModel(model, this);
    KCheckableProxyModel *checkableProxy = new MatchingCheckableProxyModel( this );
    checkableProxy->setSourceModel( model );
    checkableProxy->setSelectionModel( selectionModel );

    setModel(checkableProxy);

    //We need to reconnect from the constructor of KCheckComboBox to the new model
    connect(checkableProxy, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(updateCheckedItems(QModelIndex,QModelIndex)) );
}

TagCombo::TagCombo(QWidget* parent)
:   KComboBox(parent)
{
    Akonadi::Monitor *monitor = new Akonadi::Monitor(this);
    monitor->setTypeMonitored(Akonadi::Monitor::Tags);
    Akonadi::TagModel *model = new Akonadi::TagModel(monitor, this);
    setModel(model);
}

