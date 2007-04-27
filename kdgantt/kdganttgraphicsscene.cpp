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
#include "kdganttgraphicsscene.h"
#include "kdganttgraphicsscene_p.h"
#include "kdganttgraphicsitem.h"
#include "kdganttconstraint.h"
#include "kdganttconstraintgraphicsitem.h"
#include "kdganttitemdelegate.h"
#include "kdganttabstractrowcontroller.h"
#include "kdganttdatetimegrid.h"
#include "kdganttsummaryhandlingproxymodel.h"

#include <QToolTip>
#include <QGraphicsSceneHelpEvent>
#include <QTextDocument>

#include <QDebug>

#include <functional>
#include <algorithm>
#include <cassert>

using namespace KDGantt;

GraphicsScene::Private::Private( GraphicsScene* _q )
    : q( _q ),
      dragSource( 0 ),
      itemDelegate( new ItemDelegate( _q ) ),
      rowController( 0 ),
      grid( &default_grid ),
      readOnly( false ),
      summaryHandlingModel( new SummaryHandlingProxyModel( _q ) ),
      selectionModel( 0 )
{
    default_grid.setStartDateTime( QDateTime::currentDateTime().addDays( -1 ) );
}

void GraphicsScene::Private::resetConstraintItems()
{
    q->clearConstraintItems();
    if ( constraintModel.isNull() ) return;
    QList<Constraint> clst = constraintModel->constraints();
    Q_FOREACH( Constraint c, clst ) {
        createConstraintItem( c );
    }
    q->updateItems();
}

void GraphicsScene::Private::createConstraintItem( const Constraint& c )
{
    GraphicsItem* sitem = q->findItem( summaryHandlingModel->mapFromSource( c.startIndex() ) );
    GraphicsItem* eitem = q->findItem( summaryHandlingModel->mapFromSource( c.endIndex() ) );

    if ( sitem && eitem ) {
        ConstraintGraphicsItem* citem = new ConstraintGraphicsItem( c );
        sitem->addStartConstraint( citem );
        eitem->addEndConstraint( citem );
        q->addItem( citem );
    }



    //q->insertConstraintItem( c, citem );
}

void GraphicsScene::Private::deleteConstraintItem( const Constraint& c )
{
    ConstraintGraphicsItem* citem = findConstraintItem( c );
    GraphicsItem* item = items.value( c.startIndex(), 0 );
    if ( item ) item->removeStartConstraint( citem );
    item = items.value( c.endIndex(), 0 );
    if ( item ) item->removeEndConstraint( citem );
    if ( citem ) delete citem;
}

namespace {
    /* Very useful functional to compose functions.
     * Unfortunately an SGI extension and not standard
     */
    template<typename Operation1,typename Operation2>
    struct unary_compose : public std::unary_function<typename Operation1::result_type,typename Operation2::argument_type> {
        unary_compose( const Operation1& f, const Operation2& g ) : _f( f ), _g( g ) {}

        inline typename Operation1::result_type operator()( const typename Operation2::argument_type& arg ) {
            return _f( _g( arg ) );
        }

        Operation1 _f;
        Operation2 _g;
    };

    template<typename Operation1,typename Operation2>
    inline unary_compose<Operation1,Operation2> compose1( const Operation1& f, const Operation2& g )
    {
        return unary_compose<Operation1,Operation2>( f, g );
    }
}

ConstraintGraphicsItem* GraphicsScene::Private::findConstraintItem( const Constraint& c ) const
{
    GraphicsItem* item = items.value( c.startIndex(), 0 );
    if ( !item ) return 0;
    QList<ConstraintGraphicsItem*> clst = item->startConstraints();
    QList<ConstraintGraphicsItem*>::iterator it = clst.begin();
	for( ; it != clst.end() ; ++it )
		if ((*it)->constraint() == c )
			break;
    if (  it != clst.end() ) {
        return *it;
    } else return 0;
}

GraphicsScene::GraphicsScene( QObject* parent )
    : QGraphicsScene( parent ), _d( new Private( this ) )
{
    init();
}

GraphicsScene::~GraphicsScene()
{
    clearConstraintItems();
    clearItems();
}

#define d d_func()

void GraphicsScene::init()
{
    setConstraintModel( new ConstraintModel( this ) );
}

/* NOTE: The delegate should really be a property
 * of the view, but that doesn't really fit at
 * this time
 */
void GraphicsScene::setItemDelegate( ItemDelegate* delegate )
{
    if ( !d->itemDelegate.isNull() && d->itemDelegate->parent()==this ) delete d->itemDelegate;
    d->itemDelegate = delegate;
    update();
}

ItemDelegate* GraphicsScene::itemDelegate() const
{
    return d->itemDelegate;
}

QAbstractItemModel* GraphicsScene::model() const
{
    assert(!d->summaryHandlingModel.isNull());
    return d->summaryHandlingModel->sourceModel();
}

void GraphicsScene::setModel( QAbstractItemModel* model )
{
    assert(!d->summaryHandlingModel.isNull());
    d->summaryHandlingModel->setSourceModel(model);
    d->grid->setModel( d->summaryHandlingModel );
    setSelectionModel( new QItemSelectionModel( model, this ) );
}

QAbstractProxyModel* GraphicsScene::summaryHandlingModel() const
{
    return d->summaryHandlingModel;
}

void GraphicsScene::setSummaryHandlingModel( QAbstractProxyModel* proxyModel )
{
    proxyModel->setSourceModel( model() );
    d->summaryHandlingModel = proxyModel;
}

void GraphicsScene::setRootIndex( const QModelIndex& idx )
{
    d->grid->setRootIndex( idx );
}

QModelIndex GraphicsScene::rootIndex() const
{
    return d->grid->rootIndex();
}

ConstraintModel* GraphicsScene::constraintModel() const
{
    return d->constraintModel;
}

void GraphicsScene::setConstraintModel( ConstraintModel* cm )
{
    if ( !d->constraintModel.isNull() ) {
        disconnect( d->constraintModel );
    }
    d->constraintModel = cm;

    connect( cm, SIGNAL( constraintAdded( const Constraint& ) ), this, SLOT( slotConstraintAdded( const Constraint& ) ) );
    connect( cm, SIGNAL( constraintRemoved( const Constraint& ) ), this, SLOT( slotConstraintRemoved( const Constraint& ) ) );
    d->resetConstraintItems();
}

void GraphicsScene::setSelectionModel( QItemSelectionModel* smodel )
{
    d->selectionModel = smodel;
    // TODO: update selection from model and connect signals
}

QItemSelectionModel* GraphicsScene::selectionModel() const
{
    return d->selectionModel;
}

void GraphicsScene::setRowController( AbstractRowController* rc )
{
    d->rowController = rc;
}

AbstractRowController* GraphicsScene::rowController() const
{
    return d->rowController;
}

void GraphicsScene::setGrid( AbstractGrid* grid )
{
    QAbstractItemModel* model = d->grid->model();
    if ( grid == 0 ) grid = &d->default_grid;
    if ( d->grid ) disconnect( d->grid );
    d->grid = grid;
    connect( d->grid, SIGNAL( gridChanged() ), this, SLOT( slotGridChanged() ) );
    d->grid->setModel( model );
    slotGridChanged();
}

AbstractGrid* GraphicsScene::grid() const
{
    return d->grid;
}

void GraphicsScene::setReadOnly( bool ro )
{
    d->readOnly = ro;
}

bool GraphicsScene::isReadOnly() const
{
    return d->readOnly;
}

/* Returns the index with column=0 fromt the
 * same row as idx and with the same parent.
 * This is used to traverse the tree-structure
 * of the model
 */
QModelIndex GraphicsScene::mainIndex( const QModelIndex& idx )
{
#if 0
    if ( idx.isValid() ) {
        return idx.model()->index( idx.row(), 0,idx.parent() );
    } else {
        return QModelIndex();
    }
#else
    return idx;
#endif
}

/*! Returns the index pointing to the last column
 * in the same row as idx. This can be thought of
 * as in "inverse" of mainIndex()
 */
QModelIndex GraphicsScene::dataIndex( const QModelIndex& idx )
{
#if 0
    if ( idx.isValid() ) {
        const QAbstractItemModel* model = idx.model();
        return model->index( idx.row(), model->columnCount( idx.parent() )-1,idx.parent() );
    } else {
        return QModelIndex();
    }
#else
    return idx;
#endif
}

/*! Creates a new item of type type.
 * TODO: If the user should be allowed to override
 * this in any way, it needs to be in View!
 */
GraphicsItem* GraphicsScene::createItem( ItemType type ) const
{
#if 0
    switch( type ) {
    case TypeEvent:   return 0;
    case TypeTask:    return new TaskItem;
    case TypeSummary: return new SummaryItem;
    default:          return 0;
    }
#endif
    //qDebug() << "GraphicsScene::createItem("<<type<<")";
    Q_UNUSED( type );
    return new GraphicsItem;
}

void GraphicsScene::updateRow( const QModelIndex& rowidx )
{
    //qDebug() << "GraphicsScene::updateRow("<<rowidx<<")";
    if ( !rowidx.isValid() ) return;
    const QAbstractItemModel* model = rowidx.model(); // why const?
    assert( model );
    assert( rowController() );
    assert( model == summaryHandlingModel() );

    const QModelIndex sidx = summaryHandlingModel()->mapToSource( rowidx );
    const Span rg=rowController()->rowGeometry( sidx );
    for ( int col = 0; col < summaryHandlingModel()->columnCount( rowidx.parent() ); ++col ) {
        const QModelIndex idx = summaryHandlingModel()->index( rowidx.row(), col, rowidx.parent() );
        const int itemtype = summaryHandlingModel()->data( idx, ItemTypeRole ).toInt();
        if ( itemtype == TypeNone ) {
            removeItem( idx );
            continue;
        }
        if ( itemtype == TypeMulti ) {
            int cr=0;
            QModelIndex child;
            while ( ( child = idx.child( cr, 0 ) ).isValid() ) {
                GraphicsItem* item = findItem( child );
                if (!item) {
                    item = createItem( static_cast<ItemType>( itemtype ) );
                    item->setIndex( child );
                    insertItem( child, item);
                }
                item->updateItem( rg, child );
                setSceneRect( sceneRect().united( item->boundingRect() ) );
                ++cr;
            }
        }
	{
            if ( summaryHandlingModel()->data( rowidx.parent(), ItemTypeRole ).toInt() == TypeMulti ) continue;

            GraphicsItem* item = findItem( idx );
            if (!item) {
                item = createItem( static_cast<ItemType>( itemtype ) );
                item->setIndex( idx );
                insertItem(idx, item);
            }
            item->updateItem( rg, idx );
            setSceneRect( sceneRect().united( item->boundingRect() ) );
        }
    }
}

void GraphicsScene::insertItem( const QPersistentModelIndex& idx, GraphicsItem* item )
{
    if ( !d->constraintModel.isNull() ) {
        // Create items for constraints
        const QModelIndex sidx = summaryHandlingModel()->mapToSource( idx );
        const QList<Constraint> clst = d->constraintModel->constraintsForIndex( sidx );
        Q_FOREACH( Constraint c,  clst ) {
            QModelIndex other_idx;
            if ( c.startIndex() == sidx ) {
                other_idx = c.endIndex();
                GraphicsItem* other_item = d->items.value(summaryHandlingModel()->mapFromSource( other_idx ),0);
                if ( !other_item ) continue;
                ConstraintGraphicsItem* citem = new ConstraintGraphicsItem( c );
                item->addStartConstraint( citem );
                other_item->addEndConstraint( citem );
                addItem( citem );
            } else if ( c.endIndex() == sidx ) {
                other_idx = c.startIndex();
                GraphicsItem* other_item = d->items.value(summaryHandlingModel()->mapFromSource( other_idx ),0);
                if ( !other_item ) continue;
                ConstraintGraphicsItem* citem = new ConstraintGraphicsItem( c );
                other_item->addStartConstraint( citem );
                item->addEndConstraint( citem );
                addItem( citem );
            } else {
                assert( 0 ); // Impossible
            }
        }
    }

    d->items.insert( idx, item );
    addItem( item );
}

void GraphicsScene::removeItem( const QModelIndex& idx )
{
    QHash<QPersistentModelIndex,GraphicsItem*>::iterator it = d->items.find( idx );
    if ( it != d->items.end() ) {
        // Remove any constraintitems starting here
        const QList<ConstraintGraphicsItem*> clst = ( *it )->startConstraints();
        Q_FOREACH( ConstraintGraphicsItem* citem, clst ) {
            GraphicsItem* end_item = d->items.value( summaryHandlingModel()->mapFromSource( citem->constraint().endIndex() ),0 );
            if ( end_item ) end_item->removeEndConstraint( citem );
            delete citem;
        }
        // Get rid of the item
        delete *it;
        d->items.erase( it );
    }
}

GraphicsItem* GraphicsScene::findItem( const QModelIndex& idx ) const
{
    if ( !idx.isValid() ) return 0;
    assert( idx.model() == summaryHandlingModel() );
    QHash<QPersistentModelIndex,GraphicsItem*>::const_iterator it = d->items.find( idx );
    return ( it != d->items.end() )?*it:0;
}

GraphicsItem* GraphicsScene::findItem( const QPersistentModelIndex& idx ) const
{
    if ( !idx.isValid() ) return 0;
    assert( idx.model() == summaryHandlingModel() );
    QHash<QPersistentModelIndex,GraphicsItem*>::const_iterator it = d->items.find( idx );
    return ( it != d->items.end() )?*it:0;
}

void GraphicsScene::clearItems()
{
    qDeleteAll( items() );
    d->items.clear();
}

void GraphicsScene::updateItems()
{
    for ( QHash<QPersistentModelIndex,GraphicsItem*>::iterator it = d->items.begin();
          it != d->items.end(); ++it ) {
        GraphicsItem* const item = it.value();
        const QPersistentModelIndex& idx = it.key();
        item->updateItem( Span( item->pos().y(), item->rect().height() ), idx );
    }
}

void GraphicsScene::deleteSubtree( const QModelIndex& _idx )
{
    QModelIndex idx = dataIndex( _idx );
    removeItem( idx );
    for ( int i = 0; i < summaryHandlingModel()->rowCount( _idx ); ++i ) {
        deleteSubtree( summaryHandlingModel()->index( i, summaryHandlingModel()->columnCount(_idx)-1, _idx ) );
    }
}


ConstraintGraphicsItem* GraphicsScene::findConstraintItem( const Constraint& c ) const
{
    return d->findConstraintItem( c );
}

void GraphicsScene::clearConstraintItems()
{
    // TODO
    // d->constraintItems.clearConstraintItems();
}

void GraphicsScene::slotConstraintAdded( const Constraint& c )
{
    d->createConstraintItem( c );
}

void GraphicsScene::slotConstraintRemoved( const Constraint& c )
{
    d->deleteConstraintItem( c );
}

void GraphicsScene::slotGridChanged()
{
    updateItems();
    update();
    emit gridChanged();
}

void GraphicsScene::helpEvent( QGraphicsSceneHelpEvent *helpEvent )
{
#ifndef QT_NO_TOOLTIP
    QGraphicsItem *item = itemAt( helpEvent->scenePos() );
    if ( GraphicsItem* gitem = qgraphicsitem_cast<GraphicsItem*>( item ) ) {
        QToolTip::showText(helpEvent->screenPos(), gitem->ganttToolTip());
    } else if ( ConstraintGraphicsItem* citem = qgraphicsitem_cast<ConstraintGraphicsItem*>( item ) ) {
        QToolTip::showText(helpEvent->screenPos(), citem->ganttToolTip());
    } else {
        QGraphicsScene::helpEvent( helpEvent );
    }
#endif /* QT_NO_TOOLTIP */
}

void GraphicsScene::drawBackground( QPainter* painter, const QRectF& rect )
{
    d->grid->paintGrid( painter, sceneRect(), rect, d->rowController );
}

void GraphicsScene::itemEntered( const QModelIndex& idx )
{
    emit entered( idx );
}

void GraphicsScene::itemPressed( const QModelIndex& idx )
{
    emit pressed( idx );
}

void GraphicsScene::itemClicked( const QModelIndex& idx )
{
    emit clicked( idx );
}

void GraphicsScene::itemDoubleClicked( const QModelIndex& idx )
{
    emit doubleClicked( idx );
}

void GraphicsScene::setDragSource( GraphicsItem* item )
{
    d->dragSource = item;
}

GraphicsItem* GraphicsScene::dragSource() const
{
    return d->dragSource;
}

void GraphicsScene::print( QPainter* painter, const QRectF& target, bool drawRowLabels )
{
  QRectF targetRect(target);

  assert(rowController());

  QVector<QGraphicsTextItem*> labelItems;
  if(drawRowLabels) {
  labelItems.reserve(d->items.size());
  qreal textWidth = 0.;
  qreal rowHeight = 0.;
  {Q_FOREACH( GraphicsItem* item, d->items ) {
    QModelIndex sidx = summaryHandlingModel()->mapToSource( item->index() );
    if( sidx.parent().isValid() && sidx.parent().data( ItemTypeRole ).toInt() == TypeMulti ) {
      continue;
    }
    const Span rg=rowController()->rowGeometry( sidx );
    const QString txt = item->index().data( Qt::DisplayRole ).toString();
    QGraphicsTextItem* ti = new QGraphicsTextItem(txt,0,this);
    ti->setPos( 0, rg.start() );
    if( ti->document()->size().width() > textWidth ) textWidth = ti->document()->size().width();
    if( rg.length() > rowHeight ) rowHeight = rg.length();
    labelItems << ti;
  }}
  {Q_FOREACH( QGraphicsTextItem* item, labelItems ) {
      item->setPos( -textWidth-rowHeight, item->pos().y() );
      item->show();
  }}
  }
  QRectF oldSceneRect( sceneRect() );
  setSceneRect( itemsBoundingRect() );
  if(targetRect.isNull()) targetRect = sceneRect();
  render( painter, target );
  qDeleteAll(labelItems);

  setSceneRect( oldSceneRect );
}


#include "moc_kdganttgraphicsscene.cpp"

