/*
  progressmanager.cpp

  This file is part of libkdepim.

  Copyright (c) 2004 Till Adam <adam@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "progressmanager.h"

#include "libkdepim_debug.h"
#include <KLocalizedString>

namespace KPIM
{

unsigned int KPIM::ProgressManager::uID = 42;

ProgressItem::ProgressItem(ProgressItem *parent, const QString &id,
                           const QString &label, const QString &status,
                           bool canBeCanceled, CryptoStatus cryptoStatus)
    : mId(id),
      mLabel(label),
      mStatus(status),
      mParent(parent),
      mCanBeCanceled(canBeCanceled),
      mProgress(0),
      mTotal(0),
      mCompleted(0),
      mCryptoStatus(cryptoStatus),
      mType(0),
      mWaitingForKids(false),
      mCanceled(false),
      mUsesBusyIndicator(false),
      mCompletedCalled(false)
{
}

ProgressItem::~ProgressItem()
{
}

void ProgressItem::setComplete()
{
    //   qCDebug(LIBKDEPIM_LOG) << label();
    if (mChildren.isEmpty()) {
        if (mCompletedCalled) {
            return;
        }
        if (!mCanceled) {
            setProgress(100);
        }
        mCompletedCalled = true;
        if (parent()) {
            parent()->removeChild(this);
        }
        Q_EMIT progressItemCompleted(this);
    } else {
        mWaitingForKids = true;
    }
}

void ProgressItem::reset()
{
    setProgress(0);
    setStatus(QString());
    mCompleted = 0;
}

void ProgressItem::addChild(ProgressItem *kiddo)
{
    mChildren.insert(kiddo, true);
}

void ProgressItem::removeChild(ProgressItem *kiddo)
{
    if (mChildren.isEmpty()) {
        mWaitingForKids = false;
        return;
    }

    if (mChildren.remove(kiddo) == 0) {
        // do nothing if the specified item is not in the map
        return;
    }

    // in case we were waiting for the last kid to go away, now is the time
    if (mChildren.count() == 0 && mWaitingForKids) {
        Q_EMIT progressItemCompleted(this);
    }
}

bool ProgressItem::canceled() const
{
    return mCanceled;
}

unsigned int ProgressItem::typeProgressItem() const
{
    return mType;
}

void ProgressItem::setTypeProgressItem(unsigned int type)
{
    mType = type;
}

void ProgressItem::cancel()
{
    if (mCanceled || !mCanBeCanceled) {
        return;
    }

    qCDebug(LIBKDEPIM_LOG) << label();
    mCanceled = true;
    // Cancel all children.
    for (auto it = mChildren.cbegin(), end = mChildren.cend(); it != end; ++it) {
        ProgressItem *kid = it.key();
        if (kid->canBeCanceled()) {
            kid->cancel();
        }
    }
    setStatus(i18n("Aborting..."));
    Q_EMIT progressItemCanceled(this);
}

void ProgressItem::updateProgress()
{
    setProgress(mTotal ? mCompleted * 100 / mTotal : 0);
}

void ProgressItem::setProgress(unsigned int v)
{
    mProgress = v;
    // qCDebug(LIBKDEPIM_LOG) << label() << " :" << v;
    Q_EMIT progressItemProgress(this, mProgress);
}

const QString &ProgressItem::id() const
{
    return mId;
}

ProgressItem *ProgressItem::parent() const
{
    return mParent.data();
}

const QString &ProgressItem::label() const
{
    return mLabel;
}

void ProgressItem::setLabel(const QString &v)
{
    mLabel = v;
    Q_EMIT progressItemLabel(this, mLabel);
}

const QString &ProgressItem::status() const
{
    return mStatus;
}

void ProgressItem::setStatus(const QString &v)
{
    mStatus = v;
    Q_EMIT progressItemStatus(this, mStatus);
}

bool ProgressItem::canBeCanceled() const
{
    return mCanBeCanceled;
}

void ProgressItem::setCanBeCanceled(bool b)
{
    mCanBeCanceled = b;
}

ProgressItem::CryptoStatus ProgressItem::cryptoStatus() const
{
    return mCryptoStatus;
}

void ProgressItem::setCryptoStatus(ProgressItem::CryptoStatus v)
{
    mCryptoStatus = v;
    Q_EMIT progressItemCryptoStatus(this, v);
}

bool ProgressItem::usesBusyIndicator() const
{
    return mUsesBusyIndicator;
}

void ProgressItem::setUsesBusyIndicator(bool useBusyIndicator)
{
    mUsesBusyIndicator = useBusyIndicator;
    Q_EMIT progressItemUsesBusyIndicator(this, useBusyIndicator);
}

unsigned int ProgressItem::progress() const
{
    return mProgress;
}

// ======================================

struct ProgressManagerPrivate {
    ProgressManager instance;
};

Q_GLOBAL_STATIC(ProgressManagerPrivate, progressManagerPrivate)

ProgressManager::ProgressManager()
    : QObject()
{

}

ProgressManager::~ProgressManager()
{

}

ProgressManager *ProgressManager::instance()
{
    return progressManagerPrivate.isDestroyed() ? Q_NULLPTR : &progressManagerPrivate->instance;
}

QString ProgressManager::getUniqueID()
{
    return QString::number(++uID);
}

bool ProgressManager::isEmpty() const
{
    return mTransactions.isEmpty();
}

ProgressItem *ProgressManager::createProgressItem(const QString &id, const QString &label, const QString &status, bool canBeCanceled, ProgressItem::CryptoStatus cryptoStatus)
{
    return instance()->createProgressItemImpl(Q_NULLPTR, id, label, status,
            canBeCanceled, cryptoStatus);
}

ProgressItem *ProgressManager::createProgressItem(const QString &parent, const QString &id, const QString &label, const QString &status, bool canBeCanceled, ProgressItem::CryptoStatus cryptoStatus)
{
    return instance()->createProgressItemImpl(parent, id, label,
            status, canBeCanceled, cryptoStatus);
}

ProgressItem *ProgressManager::createProgressItem(ProgressItem *parent, const QString &id, const QString &label, const QString &status, bool canBeCanceled, ProgressItem::CryptoStatus cryptoStatus)
{
    return instance()->createProgressItemImpl(parent, id, label, status,
            canBeCanceled, cryptoStatus);
}

ProgressItem *ProgressManager::createProgressItem(const QString &label)
{
    return instance()->createProgressItemImpl(Q_NULLPTR, getUniqueID(), label,
            QString(), true, KPIM::ProgressItem::Unencrypted);
}

ProgressItem *ProgressManager::createProgressItem(unsigned int progressType, const QString &label)
{
    return instance()->createProgressItemImpl(Q_NULLPTR, getUniqueID(), label,
            QString(), true, KPIM::ProgressItem::Unencrypted, progressType);
}

ProgressItem *ProgressManager::createProgressItemImpl(ProgressItem *parent,
        const QString &id,
        const QString &label,
        const QString &status,
        bool cancellable,
        ProgressItem::CryptoStatus cryptoStatus,
        unsigned int progressType)
{
    ProgressItem *t = Q_NULLPTR;
    if (!mTransactions.value(id)) {
        t = new ProgressItem(parent, id, label, status, cancellable, cryptoStatus);
        t->setTypeProgressItem(progressType);
        mTransactions.insert(id, t);
        if (parent) {
            ProgressItem *p = mTransactions.value(parent->id());
            if (p) {
                p->addChild(t);
            }
        }
        // connect all signals
        connect(t, &ProgressItem::progressItemCompleted,
                this, &ProgressManager::slotTransactionCompleted);
        connect(t, &ProgressItem::progressItemProgress,
                this, &ProgressManager::progressItemProgress);
        connect(t, &ProgressItem::progressItemAdded,
                this, &ProgressManager::progressItemAdded);
        connect(t, &ProgressItem::progressItemCanceled,
                this, &ProgressManager::progressItemCanceled);
        connect(t, &ProgressItem::progressItemStatus,
                this, &ProgressManager::progressItemStatus);
        connect(t, &ProgressItem::progressItemLabel,
                this, &ProgressManager::progressItemLabel);
        connect(t, &ProgressItem::progressItemCryptoStatus,
                this, &ProgressManager::progressItemCryptoStatus);
        connect(t, &ProgressItem::progressItemUsesBusyIndicator,
                this, &ProgressManager::progressItemUsesBusyIndicator);

        Q_EMIT progressItemAdded(t);
    } else {
        // Hm, is this what makes the most sense?
        t = mTransactions.value(id);
    }
    return t;
}

ProgressItem *ProgressManager::createProgressItemImpl(const QString &parent,
        const QString &id,
        const QString &label,
        const QString &status,
        bool canBeCanceled,
        ProgressItem::CryptoStatus cryptoStatus,
        unsigned int progressType)
{
    ProgressItem *p = mTransactions.value(parent);
    return createProgressItemImpl(p, id, label, status, canBeCanceled, cryptoStatus, progressType);
}

void ProgressManager::emitShowProgressDialogImpl()
{
    Q_EMIT showProgressDialog();
}

// slots

void ProgressManager::slotTransactionCompleted(ProgressItem *item)
{
    mTransactions.remove(item->id());
    Q_EMIT progressItemCompleted(item);
}

void ProgressManager::slotStandardCancelHandler(ProgressItem *item)
{
    item->setComplete();
}

ProgressItem *ProgressManager::singleItem() const
{
    ProgressItem *item = Q_NULLPTR;
    QHash< QString, ProgressItem * >::const_iterator it = mTransactions.constBegin();
    QHash< QString, ProgressItem * >::const_iterator end = mTransactions.constEnd();
    while (it != end) {

        // No single item for progress possible, as one of them is a busy indicator one.
        if ((*it)->usesBusyIndicator()) {
            return Q_NULLPTR;
        }

        if (!(*it)->parent()) {               // if it's a top level one, only those count
            if (item) {
                return Q_NULLPTR; // we found more than one
            } else {
                item = (*it);
            }
        }
        ++it;
    }
    return item;
}

void ProgressManager::emitShowProgressDialog()
{
    instance()->emitShowProgressDialogImpl();
}

void ProgressManager::slotAbortAll()
{
    QHashIterator<QString, ProgressItem *> it(mTransactions);
    while (it.hasNext()) {
        it.next();
        it.value()->cancel();
    }

}

void KPIM::ProgressItem::setTotalItems(unsigned int v)
{
    mTotal = v;
}

unsigned int ProgressItem::totalItems() const
{
    return mTotal;
}

void ProgressItem::setCompletedItems(unsigned int v)
{
    mCompleted = v;
}

void ProgressItem::incCompletedItems(unsigned int v)
{
    mCompleted += v;
}

unsigned int ProgressItem::completedItems() const
{
    return mCompleted;
}

} // namespace

