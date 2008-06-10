#include "listwidget.h"

#include "ui_listwidget.h"

#include <KIcon>

#include <QItemSelectionModel>
#include <QStringListModel>

using namespace Kleo::NewCertificateUi;

class ListWidget::Private {
    friend class ::Kleo::NewCertificateUi::ListWidget;
    ListWidget * const q;
public:
    explicit Private( ListWidget * qq )
        : q( qq ),
          stringListModel(),
          ui( q )
    {
        ui.listView->setModel( &stringListModel );
        connect( ui.listView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                 q, SLOT(slotSelectionChanged()) );
        connect( &stringListModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                 q, SIGNAL(itemsChanged()) );
        connect( &stringListModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                 q, SIGNAL(itemsChanged()) );
        connect( &stringListModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                 q, SIGNAL(itemsChanged()) );
    }

private:
    void slotAdd() {
        const int idx = stringListModel.rowCount();
        if ( stringListModel.insertRows( idx, 1 ) )
            editRow( idx );
    }

    void slotRemove() {
        const int idx = selectedRow();
        stringListModel.removeRows( idx, 1 );
        selectRow( idx );
    }

    void slotUp() {
        const int idx = selectedRow();
        swapRows( idx - 1, idx );
        selectRow( idx - 1 );
    }

    void slotDown() {
        const int idx = selectedRow();
        swapRows( idx, idx + 1 );
        selectRow( idx + 1 );
    }

    void slotSelectionChanged() {
        enableDisableActions();
    }

private:
    void editRow( int idx ) {
        const QModelIndex mi = stringListModel.index( idx );
        if ( !mi.isValid() )
            return;
        ui.listView->setCurrentIndex( mi );
        ui.listView->edit( mi );
    }

    QModelIndexList selectedIndexes() const {
        return ui.listView->selectionModel()->selectedRows();
    }
    int selectedRow() const {
        const QModelIndexList mil = selectedIndexes();
        return mil.empty() ? -1 : mil.front().row() ;
    }
    void selectRow( int idx ) {
        const QModelIndex mi = stringListModel.index( idx );
        if ( mi.isValid() )
            ui.listView->selectionModel()->select( mi, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    }
    void swapRows( int r1, int r2 ) {
        if ( r1 < 0 || r2 < 0 || r1 >= stringListModel.rowCount() || r2 >= stringListModel.rowCount() )
            return;
        const QModelIndex m1 = stringListModel.index( r1 );
        const QModelIndex m2 = stringListModel.index( r2 );
        const QVariant data1 = m1.data();
        const QVariant data2 = m2.data();
        stringListModel.setData( m1, data2 );
        stringListModel.setData( m2, data1 );
    }
    void enableDisableActions() {
        const QModelIndexList mil = selectedIndexes();
        ui.removeTB->setEnabled( !mil.empty() );
        ui.upTB->setEnabled( mil.size() == 1 && mil.front().row() > 0 );
        ui.downTB->setEnabled( mil.size() == 1 && mil.back().row() < stringListModel.rowCount() - 1 );
    }

private:
    QStringListModel stringListModel;

    struct UI : Ui_ListWidget {
        explicit UI( ListWidget * q )
            : Ui_ListWidget()
        {
            setupUi( q );

            addTB->setIcon( KIcon( "list-add" ) );
            removeTB->setIcon( KIcon( "list-remove" ) );
            upTB->setIcon( KIcon( "go-up" ) );
            downTB->setIcon( KIcon( "go-down" ) );
        }
    } ui;

};

ListWidget::ListWidget( QWidget * p )
    : QWidget( p ), d( new Private( this ) )
{

}

ListWidget::~ListWidget() {}

QStringList ListWidget::items() const {
    return d->stringListModel.stringList();
}

void ListWidget::setItems( const QStringList & items ) {
    d->stringListModel.setStringList( items );
}

#include "moc_listwidget.cpp"
