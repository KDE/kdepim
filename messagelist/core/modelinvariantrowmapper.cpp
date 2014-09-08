/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#include "core/modelinvariantrowmapper.h"
#include "core/modelinvariantrowmapper_p.h"
#include "core/modelinvariantindex_p.h"

#include <QTimer>
#include <QTime>

#include <QDebug>

namespace MessageList
{

namespace Core
{

class RowShift
{
public:
    int mMinimumRowIndex;
    int mShift;
    QHash< int, ModelInvariantIndex * > *mInvariantHash;

public:
    RowShift(int minRowIndex, int shift, QHash< int, ModelInvariantIndex * > *invariantHash)
        : mMinimumRowIndex(minRowIndex), mShift(shift), mInvariantHash(invariantHash)
    {
    }

    ~RowShift()
    {
        QHash< int, ModelInvariantIndex * >::ConstIterator end(mInvariantHash->constEnd());
        for (QHash< int, ModelInvariantIndex * >::ConstIterator it = mInvariantHash->constBegin(); it != end; ++it) {
            (*it)->d->setRowMapper(0);
        }
        delete mInvariantHash;
    }
};

} // namespace Core

} // namespace MessageList

using namespace MessageList::Core;

ModelInvariantRowMapper::ModelInvariantRowMapper()
    : d(new ModelInvariantRowMapperPrivate(this))
{
    d->mRowShiftList = new QList< RowShift * >();
    d->mCurrentShiftSerial = 0;
    d->mCurrentInvariantHash = new QHash< int, ModelInvariantIndex * >();
    d->mUpdateTimer = new QTimer(this);
    d->mUpdateTimer->setSingleShot(true);
    d->mLazyUpdateChunkInterval = 50;
    d->mLazyUpdateIdleInterval = 50;

    connect(d->mUpdateTimer, SIGNAL(timeout()),
            SLOT(slotPerformLazyUpdate()));
}

ModelInvariantRowMapper::~ModelInvariantRowMapper()
{
    if (d->mUpdateTimer->isActive()) {
        d->mUpdateTimer->stop();
    }

    // FIXME: optimize this (it CAN be optimized)
    QHash< int, ModelInvariantIndex * >::ConstIterator end(d->mCurrentInvariantHash->constEnd());
    for (QHash< int, ModelInvariantIndex * >::ConstIterator it = d->mCurrentInvariantHash->constBegin(); it != end; ++it) {
        (*it)->d->setRowMapper(0);
    }
    delete d->mCurrentInvariantHash;

    if (d->mRowShiftList) {
        while (!d->mRowShiftList->isEmpty()) {
            delete d->mRowShiftList->takeFirst();
        }

        delete d->mRowShiftList;
    }

    delete d;
}

void ModelInvariantRowMapperPrivate::killFirstRowShift()
{
    RowShift *shift = mRowShiftList->at(0);

    Q_ASSERT(shift->mInvariantHash->isEmpty());

    delete shift;
    mRowShiftList->removeAt(0);
    mRemovedShiftCount++;
    if (mRowShiftList->isEmpty()) {
        delete mRowShiftList;
        mRowShiftList = 0;
    }
}

void ModelInvariantRowMapperPrivate::indexDead(ModelInvariantIndex *invariant)
{
    Q_ASSERT(invariant->d->rowMapper() == q);

    if (invariant->d->rowMapperSerial() == mCurrentShiftSerial) {
        mCurrentInvariantHash->remove(invariant->d->modelIndexRow());
        return;
    }

    Q_ASSERT(invariant->d->rowMapperSerial() < mCurrentShiftSerial);

    if (!mRowShiftList) {
        return; // not found (not requested yet or invalid index at all)
    }

    uint invariantShiftIndex = invariant->d->rowMapperSerial() - mRemovedShiftCount;

    Q_ASSERT(invariantShiftIndex < static_cast< uint >(mRowShiftList->count()));

    RowShift *shift = mRowShiftList->at(invariantShiftIndex);

    Q_ASSERT(shift);

    shift->mInvariantHash->remove(invariant->d->modelIndexRow());

    if ((shift->mInvariantHash->isEmpty()) && (invariantShiftIndex == 0)) {
        // no more invariants with serial <= invariant->d->rowMapperSerial()
        killFirstRowShift();
    }
}

void ModelInvariantRowMapperPrivate::updateModelInvariantIndex(int modelIndexRow, ModelInvariantIndex *invariantToFill)
{
    // Here the invariant already belongs to this mapper. We ASSUME that it's somewhere
    // in the history and not in the hash belonging to the current serial.
    // modelIndexRow is the CURRENT model index row.
    Q_ASSERT(invariantToFill->d->rowMapper() == q);

    uint invariantShiftIndex = invariantToFill->d->rowMapperSerial() - mRemovedShiftCount;

    Q_ASSERT(invariantShiftIndex < static_cast< uint >(mRowShiftList->count()));

    RowShift *shift = mRowShiftList->at(invariantShiftIndex);

    int count = shift->mInvariantHash->remove(invariantToFill->d->modelIndexRow());

    Q_ASSERT(count > 0);
    Q_UNUSED(count);

    // update and make it belong to the current serial
    invariantToFill->d->setModelIndexRowAndRowMapperSerial(modelIndexRow, mCurrentShiftSerial);

    Q_ASSERT(!mCurrentInvariantHash->contains(invariantToFill->d->modelIndexRow()));

    mCurrentInvariantHash->insert(invariantToFill->d->modelIndexRow(), invariantToFill);

    if ((shift->mInvariantHash->isEmpty()) && (invariantShiftIndex == 0)) {
        // no more invariants with serial <= invariantToFill->rowMapperSerial()
        killFirstRowShift();
    }
}

ModelInvariantIndex *ModelInvariantRowMapperPrivate::modelIndexRowToModelInvariantIndexInternal(int modelIndexRow, bool updateIfNeeded)
{
    // First of all look it up in the current hash
    ModelInvariantIndex *invariant = mCurrentInvariantHash->value(modelIndexRow, 0);
    if (invariant) {
        return invariant;    // found: was up to date
    }

    // Go backward in history by unapplying changes
    if (!mRowShiftList) {
        return 0;    // not found (not requested yet or invalid index at all)
    }

    int idx = mRowShiftList->count();
    if (idx == 0) {
        Q_ASSERT(false);
        return 0; // should never happen (mRowShiftList should have been 0), but well...
    }
    idx--;

    int previousIndexRow = modelIndexRow;

    while (idx >= 0) {
        RowShift *shift = mRowShiftList->at(idx);

        // this shift has taken "previousModelIndexRow" in the historic state
        // and has executed:
        //
        //   if ( previousIndexRow >= shift->mMinimumRowIndex )
        //     previousIndexRow += shift->mShift;
        //
        // so inverting it
        //
        //   int potentialPreviousModelIndexRow = modelIndexRow - shift->mShift;
        //   if ( potentialPreviousModelIndexRow >= shift->mMinimumRowIndex )
        //     previousIndexRow = potentialPreviousModelIndexRow;
        //
        // or by simplyfying...

        int potentialPreviousModelIndexRow = previousIndexRow - shift->mShift;
        if (potentialPreviousModelIndexRow >= shift->mMinimumRowIndex) {
            previousIndexRow = potentialPreviousModelIndexRow;
        }

        invariant = shift->mInvariantHash->value(previousIndexRow, 0);
        if (invariant) {
            // found at this level in history
            if (updateIfNeeded) { // update it too
                updateModelInvariantIndex(modelIndexRow, invariant);
            }
            return invariant;
        }

        idx--;
    }

    qWarning() << "Requested invariant for storage row index "
               << modelIndexRow << " not found in history";
    return 0; // not found in history
}

void ModelInvariantRowMapper::setLazyUpdateChunkInterval(int chunkInterval)
{
    d->mLazyUpdateChunkInterval = chunkInterval;
}

void ModelInvariantRowMapper::setLazyUpdateIdleInterval(int idleInterval)
{
    d->mLazyUpdateIdleInterval = idleInterval;
}

int ModelInvariantRowMapper::modelInvariantIndexToModelIndexRow(ModelInvariantIndex *invariant)
{
    // the invariant shift serial is the serial this mapper
    // had at the time it emitted the invariant.
    // mRowShiftList at that time had at most invariantShiftSerial items.
    Q_ASSERT(invariant);

    if (invariant->d->rowMapper() != this) {
        return -1;
    }

    if (invariant->d->rowMapperSerial() == d->mCurrentShiftSerial) {
        Q_ASSERT(d->mCurrentInvariantHash->value(invariant->d->modelIndexRow()) == invariant);
        return invariant->d->modelIndexRow(); // this invariant was emitted very recently and isn't affected by any change
    }

    // If RowShift elements weren't removed from the list then
    // we should have mCurrentShiftSerial items in the list.
    // But RowShifts ARE removed sequentially from the beginning of the list
    // as the invariants are updated in the user's data.
    // We are making sure that if a RowShift belonging to a certain
    // serial is removed from the list then there are no more
    // ModelInvariantIndexinstances with that (or a lower) serial around.
    // Thus invariantShiftSerial is >= mRemovedShiftCount.

    // Example:
    //     Initial state, no shifts, current serial 0, removed shifts 0
    //     Emit ModelInvariantIndexfor model index row 6, with serial 0.
    //     User asks for model index row of invariant that has row index 10 and serial 0.
    //       The serial is equal to the current serial and we return the row index unchanged.
    //     A row arrives at position 4
    //       We add a RowShift with start index 5 and offset +1
    //       We increase current serial to 1
    //     User asks for model index row of invariant that has row index 6 with serial 0.
    //       We compute the first RowShift index as serial 0 - removed 0 = 0
    //       We apply the row shifts starting at that index.
    //         That is, since the requested row index is 6 >= 5
    //           We apply +1 shift and return row index 7 serial 1
    //     User asks for model index row of invariant that has row index 7 with serial 1
    //       The serial is equal to the current serial and we return the row index unchanged still with serial 1
    //     We update all the invariants in the user's data so that
    //     there are no more invariants with serial 0.
    //       We remove the RowShift and increase removed shift count to 1
    //     User asks for model index row of invariant that has row index 7
    //       The ModelInvariantIndex MUST have at least serial 1 because of the removal step above.
    //       The serial is equal to the current serial and we return the row index unchanged still with serial 1
    //     A row arrives at position 2
    //       We add a RowShift with start index 3 and offset +1
    //       We increase current serial to 2
    //     User asks for model index row of invariant that has row index 7 with serial 1.
    //       We compute the first RowShift index as serial 1 - removed 1 = 0
    //       We apply the row shifts starting at that index.
    //         That is, since the requested row index is 7 >= 3
    //           We apply +1 shift and return row index 8 serial 2
    //     User asks for model index row of invariant that has row index 8 and serial 2
    //       The serial is equal to the current serial and we return the row index unchanged still with serial 2
    //     Etc...

    // So if we can trust that the user doesn't mess up with serials
    // and the requested serial is not equal to the current serial
    // then we can be 100% sure that mRowShiftList is not null (it contains at least one item).
    // The requested serial is surely >= than mRemovedShiftCount too.

    // To find the starting index of the RowShifts that apply to this
    // serial we need to offset them by the removed rows.

    uint invariantShiftIndex = invariant->d->rowMapperSerial() - d->mRemovedShiftCount;

    Q_ASSERT(d->mRowShiftList);

    // For the reasoning above invariantShiftIndex is surely < than mRowShiftList.count()

    const uint count = static_cast< uint >(d->mRowShiftList->count());

    Q_ASSERT(invariantShiftIndex < count);

    int modelIndexRow = invariant->d->modelIndexRow();

    // apply shifts
    for (uint idx = invariantShiftIndex; idx < count; idx++) {
        RowShift *shift = d->mRowShiftList->at(idx);
        if (modelIndexRow >= shift->mMinimumRowIndex) {
            modelIndexRow += shift->mShift;
        }
    }

    // Update the invariant on-the-fly too...
    d->updateModelInvariantIndex(modelIndexRow, invariant);

    return modelIndexRow;
}

void ModelInvariantRowMapper::createModelInvariantIndex(int modelIndexRow, ModelInvariantIndex *invariantToFill)
{
    // The user is athemeg for the invariant of the item that is at the CURRENT modelIndexRow.
    Q_ASSERT(invariantToFill->d->rowMapper() == 0);

    // Plain new invariant. Fill it and add to the current hash.
    invariantToFill->d->setModelIndexRowAndRowMapperSerial(modelIndexRow, d->mCurrentShiftSerial);
    invariantToFill->d->setRowMapper(this);

    Q_ASSERT(!d->mCurrentInvariantHash->contains(modelIndexRow));

    d->mCurrentInvariantHash->insert(modelIndexRow, invariantToFill);
}

ModelInvariantIndex *ModelInvariantRowMapper::modelIndexRowToModelInvariantIndex(int modelIndexRow)
{
    return d->modelIndexRowToModelInvariantIndexInternal(modelIndexRow, false);
}

QList< ModelInvariantIndex * > *ModelInvariantRowMapper::modelIndexRowRangeToModelInvariantIndexList(int startIndexRow, int count)
{
    if (!d->mRowShiftList) {
        if (d->mCurrentInvariantHash->isEmpty()) {
            return 0;    // no invariants emitted, even if rows are changed, no invariant is affected.
        }
    }

    // Find the invariants in range.
    // It's somewhat impossible to split this in chunks.

    QList< ModelInvariantIndex * > *invariantList = new QList< ModelInvariantIndex * >();

    const int end = startIndexRow + count;
    for (int idx = startIndexRow; idx < end; idx++) {
        ModelInvariantIndex *invariant = d->modelIndexRowToModelInvariantIndexInternal(idx, true);
        if (invariant) {
            invariantList->append(invariant);
        }
    }

    if (invariantList->isEmpty()) {
        delete invariantList;
        return 0;
    }

    return invariantList;
}

void ModelInvariantRowMapper::modelRowsInserted(int modelIndexRowPosition, int count)
{
    // Some rows were added to the model at modelIndexRowPosition.

    // FIXME: If rows are added at the end then we don't need any mapping.
    //        The fact is that we don't know which is the model's end...
    //        But maybe we can consider the end being the greatest row
    //        index emitted until now...

    if (!d->mRowShiftList) {
        if (d->mCurrentInvariantHash->isEmpty()) {
            return;    // no invariants emitted, even if rows are changed, no invariant is affected.
        }
        // some invariants might be affected
        d->mRowShiftList = new QList< RowShift * >();
    }

    RowShift *shift;

    if (d->mCurrentInvariantHash->isEmpty()) {
        // No invariants updated (all existing are outdated)

        Q_ASSERT(d->mRowShiftList->count() > 0);   // must be true since it's not null

        // Check if we can attach to the last existing shift (very common for consecutive row additions)
        shift = d->mRowShiftList->at(d->mRowShiftList->count() - 1);
        Q_ASSERT(shift);

        if (shift->mShift > 0) { // the shift was positive (addition)
            if ((shift->mMinimumRowIndex + shift->mShift) == modelIndexRowPosition) {
                // Inserting contiguous blocks of rows, just extend this shift
                shift->mShift += count;
                Q_ASSERT(d->mUpdateTimer->isActive());
                return;
            }
        }
    }

    // FIXME: If we have few items, we can just shift the indexes now.

    shift = new RowShift(modelIndexRowPosition, count, d->mCurrentInvariantHash);
    d->mRowShiftList->append(shift);

    d->mCurrentShiftSerial++;
    d->mCurrentInvariantHash = new QHash< int, ModelInvariantIndex * >();

    if (d->mRowShiftList->count() > 7) { // 7 is heuristic
        // We start loosing performance as the stack is growing too much.
        // Start updating NOW and hope we can get it in few sweeps.

        if (d->mUpdateTimer->isActive()) {
            d->mUpdateTimer->stop();
        }

        d->slotPerformLazyUpdate();

    } else {
        // Make sure we'll get a lazy update somewhere in the future
        if (!d->mUpdateTimer->isActive()) {
            d->mUpdateTimer->start(d->mLazyUpdateIdleInterval);
        }
    }
}

QList< ModelInvariantIndex * > *ModelInvariantRowMapper::modelRowsRemoved(int modelIndexRowPosition, int count)
{
    // Some rows were added from the model at modelIndexRowPosition.

    // FIXME: If rows are removed from the end, we don't need any mapping.
    //        The fact is that we don't know which is the model's end...
    //        But maybe we can consider the end being the greatest row
    //        index emitted until now...

    if (!d->mRowShiftList) {
        if (d->mCurrentInvariantHash->isEmpty()) {
            return 0;    // no invariants emitted, even if rows are changed, no invariant is affected.
        }
        // some invariants might be affected
    }

    // FIXME: If we have few items, we can just shift the indexes now.

    // FIXME: Find a way to "merge" the shifts, if possible
    //        It OFTEN happens that we remove a lot of items at once (as opposed
    //        to item addition which is usually an incremental operation).

    // FIXME: HUGE PROBLEM
    //        When the items arent contiguous or are just out of order it's
    //        impossible to merge the shifts. Deleting many messages
    //        generates then a very deep delta stack. Since to delete the
    //        next message you need to traverse the whole stack, this method
    //        becomes very slow (maybe not as slow as updating all the indexes
    //        in the general case, but still *slow*).
    //
    //        So one needs to perform updates while rows are being removed
    //        but that tends to void all your efforts to not update the
    //        whole list of items every time...
    //
    //        Also deletions don't seem to be asynchronous (or at least
    //        they eat all the CPU power available for KMail) so the timers
    //        don't fire and we're not actually processing the model jobs...
    //
    //        It turns out that deleting many items is just slower than
    //        reloading the view...

    // Invalidate the invariants affected by the change
    // In most cases it's a relatively small sweep (and it's done once).
    // It's somewhat impossible to split this in chunks.

    QList< ModelInvariantIndex * > *deadInvariants = new QList< ModelInvariantIndex * >();

    const int end = modelIndexRowPosition + count;
    for (int idx = modelIndexRowPosition; idx < end; idx++) {
        // FIXME: One could optimize this by joining the retrieval and destruction functions
        //        that is by making a special indexDead( int modelIndex )..
        ModelInvariantIndex *dyingInvariant = d->modelIndexRowToModelInvariantIndexInternal(idx, false);
        if (dyingInvariant) {
            d->indexDead(dyingInvariant);   // will remove from this mapper hashes
            dyingInvariant->d->setRowMapper(0);   // invalidate!
            deadInvariants->append(dyingInvariant);
        } else {
            // got no dying invariant
            qWarning() << "Could not find invariant to invalidate at current row " << idx;
        }
    }

    if (!d->mRowShiftList) {
        // have no pending shifts, look if we are keeping other invariants
        if (d->mCurrentInvariantHash->isEmpty()) {
            // no more invariants in this mapper, even if rows are changed, no invariant is affected.
            if (deadInvariants->isEmpty()) {
                // should never happen, but well...
                delete deadInvariants;
                return 0;
            }
            return deadInvariants;
        }
        // still have some invariants inside, must add a shift for them
        d->mRowShiftList = new QList< RowShift * >();
    } // else already have shifts

    // add a shift for this row removal
    RowShift *shift = new RowShift(modelIndexRowPosition + count, -count, d->mCurrentInvariantHash);
    d->mRowShiftList->append(shift);

    d->mCurrentShiftSerial++;
    d->mCurrentInvariantHash = new QHash< int, ModelInvariantIndex * >();

    // trigger updates
    if (d->mRowShiftList->count() > 7) { // 7 is heuristic
        // We start loosing performance as the stack is growing too much.
        // Start updating NOW and hope we can get it in few sweeps.

        if (d->mUpdateTimer->isActive()) {
            d->mUpdateTimer->stop();
        }

        d->slotPerformLazyUpdate();

    } else {
        // Make sure we'll get a lazy update somewhere in the future
        if (!d->mUpdateTimer->isActive()) {
            d->mUpdateTimer->start(d->mLazyUpdateIdleInterval);
        }
    }

    if (deadInvariants->isEmpty()) {
        // should never happen, but well...
        delete deadInvariants;
        return 0;
    }

    return deadInvariants;
}

void ModelInvariantRowMapper::modelReset()
{
    // FIXME: optimize this (it probably can be optimized by providing a more complex user interface)
    QHash< int, ModelInvariantIndex * >::ConstIterator end(d->mCurrentInvariantHash->constEnd());

    for (QHash< int, ModelInvariantIndex * >::ConstIterator it = d->mCurrentInvariantHash->constBegin(); it != end; ++it) {
        (*it)->d->setRowMapper(0);
    }
    d->mCurrentInvariantHash->clear();

    if (d->mRowShiftList) {
        while (!d->mRowShiftList->isEmpty()) {
            delete d->mRowShiftList->takeFirst();
        }

        delete d->mRowShiftList;
        d->mRowShiftList = 0;
    }

    d->mCurrentShiftSerial = 0;
    d->mRemovedShiftCount = 0;
}

void ModelInvariantRowMapperPrivate::slotPerformLazyUpdate()
{
    // The drawback here is that when one row is removed from the middle (say position 500 of 1000)
    // then we require ALL the items to be updated...but:
    //
    // - We can do it very lazily in the background
    // - Optimizing this would mean to ALSO keep the indexes in lists or in a large array
    //   - The list approach would require to keep the indexes sorted
    //     so it would cost at least N log (N) / 2.. which is worse than N.
    //   - We could keep a single (or multiple) array as large as the model
    //     but then we'd have a large memory consumption and large overhead
    //     when inserting / removing items from the middle.
    //
    // So finally I think that the multiple hash approach is a "minimum loss" approach.

    QTime startTime = QTime::currentTime();

    int curIndex = 0;

    while (mRowShiftList) {
        // Have at least one row shift
        uint count = static_cast< uint >(mRowShiftList->count());

        // Grab it
        RowShift *shift = mRowShiftList->at(0);

        // and update the invariants that belong to it
        QHash< int, ModelInvariantIndex * >::Iterator it = shift->mInvariantHash->begin();
        QHash< int, ModelInvariantIndex * >::Iterator end = shift->mInvariantHash->end();

        while (it != end) {
            ModelInvariantIndex *invariant = *it;

            it = shift->mInvariantHash->erase(it);

            // apply shifts
            int modelIndexRow = invariant->d->modelIndexRow();

            for (uint idx = 0; idx < count; ++idx) {
                RowShift *thatShift = mRowShiftList->at(idx);
                if (modelIndexRow >= thatShift->mMinimumRowIndex) {
                    modelIndexRow += thatShift->mShift;
                }
            }

            // update and make it belong to the current serial
            invariant->d->setModelIndexRowAndRowMapperSerial(modelIndexRow, mCurrentShiftSerial);

            mCurrentInvariantHash->insert(modelIndexRow, invariant);

            // once in a while check if we ran out of time
            if ((curIndex % 15) == 0) {   // 15 is heuristic
                int elapsed = startTime.msecsTo(QTime::currentTime());
                if ((elapsed > mLazyUpdateChunkInterval) || (elapsed < 0)) {
                    // interrupt
                    //qDebug() << "Lazy update fixed " << curIndex << " invariants " << endl;
                    mUpdateTimer->start(mLazyUpdateIdleInterval);
                    return;
                }
            }

            curIndex++;
        }

        // no more invariants with serial <= invariantToFill->rowMapperSerial()
        killFirstRowShift();
    }

    //qDebug() << "Lazy update fixed " << curIndex << " invariants " << endl;

    // if we're here then no more work needs to be done.
}

#include "moc_modelinvariantrowmapper.cpp"
