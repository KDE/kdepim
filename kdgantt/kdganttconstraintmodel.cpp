/****************************************************************************
 ** Copyright (C) 2001-2006 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Gantt library.
 **
 ** This file may be distributed and/or modified under the terms of the
 ** GNU General Public License version 2 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL included in the
 ** packaging of this file.
 **
 ** Licensees holding valid commercial KD Gantt licenses may use this file in
 ** accordance with the KD Gantt Commercial License Agreement provided with
 ** the Software.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** See http://www.kdab.net/kdgantt for
 **   information about KD Gantt Commercial License Agreements.
 **
 ** Contact info@kdab.net if any conditions of this
 ** licensing are not clear to you.
 **
 **********************************************************************/
#include "kdganttconstraintmodel.h"
#include "kdganttconstraintmodel_p.h"

#include <QDebug>

#include <cassert>

using namespace KDGantt;

/*!\class KDGantt::ConstraintModel
 *\ingroup KDGantt
 * The ConstraintModel keeps track of the
 * interdependencies between gantt items in
 * a View.
 *
 */

ConstraintModel::Private::Private()
{
}

void ConstraintModel::Private::addConstraintToIndex( const QPersistentModelIndex& idx, const Constraint& c )
{
    QMultiHash<QPersistentModelIndex,Constraint>::iterator it = indexMap.find(idx);
    while (it != indexMap.end() && it.key() == idx) {
        // Check if we already have this
        if ( *it == c ) return;
        ++it;
    }

    indexMap.insert( idx, c );
}

void ConstraintModel::Private::removeConstraintFromIndex( const QPersistentModelIndex& idx,  const Constraint& c )
{
    QMultiHash<QPersistentModelIndex,Constraint>::iterator it = indexMap.find(idx);
    while (it != indexMap.end() && it.key() == idx) {
        if ( *it == c ) {
            it =indexMap.erase( it );
        } else {
            ++it;
        }
    }
}

/*! Constructor. Creates an empty ConstraintModel with parent \a parent
 */
ConstraintModel::ConstraintModel( QObject* parent )
    : QObject( parent ), _d( new Private )
{
    init();
}

/*!\internal*/
ConstraintModel::ConstraintModel( Private* d_ptr, QObject* parent )
    : QObject( parent ), _d( d_ptr )
{
    init();
}

/*! Destroys this ConstraintModel */
ConstraintModel::~ConstraintModel()
{
    delete _d;
}

#define d d_func()

void ConstraintModel::init()
{
}

/*! Adds the constraint \a c to this ConstraintModel
 *  If the Constraint \a c is already in this ConstraintModel,
 *  nothing happens.
 */
void ConstraintModel::addConstraint( const Constraint& c )
{
    int size = d->constraints.size();
    d->constraints.insert( c );
    if ( size != d->constraints.size() ) {
        d->addConstraintToIndex( c.startIndex(), c );
        d->addConstraintToIndex( c.endIndex(), c );
        emit constraintAdded( c );
    }
}

/*! Removes the Constraint \a c from this
 * ConstraintModel. If \a c was found and removed,
 * the signal constraintRemoved(const Constraint&) is emitted.
 *
 * \returns If \a c was found and removed, it returns true,
 * otherwise it returns false.
 */
bool ConstraintModel::removeConstraint( const Constraint& c )
{
    bool rc = d->constraints.remove( c );
    d->removeConstraintFromIndex( c.startIndex(), c );
    d->removeConstraintFromIndex( c.endIndex(), c );
    if ( rc ) emit constraintRemoved( c );
    return rc;
}

/*! Removes all Constraints from this model */
void ConstraintModel::clear()
{
    QList<Constraint> lst = constraints();
    Q_FOREACH( const Constraint& c, lst ) {
        removeConstraint( c );
    }
}

/*! \returns A list of all Constraints in this
 * ConstraintModel.
 */
QList<Constraint> ConstraintModel::constraints() const
{
    return d->constraints.toList();
}

/*! \returns A list of all Constraints in this ConstraintModel
 * that have an endpoint at \a idx.
 */
QList<Constraint> ConstraintModel::constraintsForIndex( const QModelIndex& idx ) const
{
    assert( d->indexMap.isEmpty() || idx.model() == d->indexMap.keys().front().model() );
    return d->indexMap.values( idx );
}

/*! Returns true if a Constraint with start \a s and end \a e
 * exists, otherwise false.
 */
bool ConstraintModel::hasConstraint( const Constraint& c ) const
{
    return d->constraints.contains( c );
}


#undef d

#ifndef KDAB_NO_UNIT_TESTS

#include <QStandardItemModel>

#include "unittest/test.h"

KDAB_SCOPED_UNITTEST_SIMPLE( KDGantt, ConstraintModel, "test" )
{
    QStandardItemModel dummyModel( 100, 100 );
    ConstraintModel model;

    assertEqual( model.constraints().count(), 0 );

    model.addConstraint( Constraint( QModelIndex(), QModelIndex() ) );
    assertEqual( model.constraints().count(), 1 );

    model.addConstraint( Constraint( QModelIndex(), QModelIndex() ) );
    assertEqual( model.constraints().count(), 1 );

    QModelIndex idx1 = dummyModel.index( 7, 17, QModelIndex() );
    QModelIndex idx2 = dummyModel.index( 42, 17, QModelIndex() );

    model.addConstraint( Constraint( idx1, idx2 ) );
    assertEqual( model.constraints().count(), 2 );
    assertTrue( model.hasConstraint( Constraint( idx1, idx2 ) ) );

    assertEqual( model.constraintsForIndex( QModelIndex() ).count(), 1 );

    model.removeConstraint( Constraint( QModelIndex(), QModelIndex() ) );
    assertEqual( model.constraints().count(), 1 );
    assertFalse( model.hasConstraint( Constraint( QModelIndex(), QModelIndex() ) ) );

    model.removeConstraint( Constraint( QModelIndex(), QModelIndex() ) );
    assertEqual( model.constraints().count(), 1 );

    model.removeConstraint( Constraint( idx1, idx2 ) );
    assertEqual( model.constraints().count(), 0 );
    assertFalse( model.hasConstraint( Constraint( idx1, idx2 ) ) );

    model.addConstraint( Constraint( idx1, idx2 ) );
}

#endif /* KDAB_NO_UNIT_TESTS */

#include "moc_kdganttconstraintmodel.cpp"
