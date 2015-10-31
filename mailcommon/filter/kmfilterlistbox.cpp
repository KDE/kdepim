/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "kmfilterlistbox.h"
#include "mailfilter.h"
#include "filtermanager.h"
#include "filteractions/filteractiondict.h"
#include "invalidfilters/invalidfilterinfo.h"
#include "invalidfilters/invalidfilterdialog.h"
#include <KInputDialog>
#include <KLocalizedString>
#include <khbox.h>
#include <klistwidgetsearchline.h>
#include <kpushbutton.h>
#include <qboxlayout.h>
#include <qlistwidget.h>
#include <qshortcut.h>
#include <KMessageBox>
#include <KDebug>

// What's this help texts
const char *_wt_filterlist =
        I18N_NOOP( "<qt><p>This is the list of defined filters. "
                   "They are processed top-to-bottom.</p>"
                   "<p>Click on any filter to edit it "
                   "using the controls in the right-hand half "
                   "of the dialog.</p></qt>" );

const char *_wt_filterlist_new =
        I18N_NOOP( "<qt><p>Click this button to create a new filter.</p>"
                   "<p>The filter will be inserted just before the currently-"
                   "selected one, but you can always change that "
                   "later on.</p>"
                   "<p>If you have clicked this button accidentally, you can undo this "
                   "by clicking on the <em>Delete</em> button.</p></qt>" );

const char *_wt_filterlist_copy =
        I18N_NOOP( "<qt><p>Click this button to copy a filter.</p>"
                   "<p>If you have clicked this button accidentally, you can undo this "
                   "by clicking on the <em>Delete</em> button.</p></qt>" );

const char *_wt_filterlist_delete =
        I18N_NOOP( "<qt><p>Click this button to <em>delete</em> the currently-"
                   "selected filter from the list above.</p>"
                   "<p>There is no way to get the filter back once "
                   "it is deleted, but you can always leave the "
                   "dialog by clicking <em>Cancel</em> to discard the "
                   "changes made.</p></qt>" );

const char *_wt_filterlist_up =
        I18N_NOOP( "<qt><p>Click this button to move the currently-"
                   "selected filter <em>up</em> one in the list above.</p>"
                   "<p>This is useful since the order of the filters in the list "
                   "determines the order in which they are tried on messages: "
                   "The topmost filter gets tried first.</p>"
                   "<p>If you have clicked this button accidentally, you can undo this "
                   "by clicking on the <em>Down</em> button.</p></qt>" );

const char *_wt_filterlist_down =
        I18N_NOOP( "<qt><p>Click this button to move the currently-"
                   "selected filter <em>down</em> one in the list above.</p>"
                   "<p>This is useful since the order of the filters in the list "
                   "determines the order in which they are tried on messages: "
                   "The topmost filter gets tried first.</p>"
                   "<p>If you have clicked this button accidentally, you can undo this "
                   "by clicking on the <em>Up</em> button.</p></qt>" );

const char *_wt_filterlist_top =
        I18N_NOOP( "<qt><p>Click this button to move the currently-"
                   "selected filter to top of list.</p>"
                   "<p>This is useful since the order of the filters in the list "
                   "determines the order in which they are tried on messages: "
                   "The topmost filter gets tried first.</p></qt>" );

const char *_wt_filterlist_bottom =
        I18N_NOOP( "<qt><p>Click this button to move the currently-"
                   "selected filter to bottom of list.</p>"
                   "<p>This is useful since the order of the filters in the list "
                   "determines the order in which they are tried on messages: "
                   "The topmost filter gets tried first.</p></qt>" );

const char *_wt_filterlist_rename =
        I18N_NOOP( "<qt><p>Click this button to rename the currently-selected filter.</p>"
                   "<p>Filters are named automatically, as long as they start with "
                   "\"&lt;\".</p>"
                   "<p>If you have renamed a filter accidentally and want automatic "
                   "naming back, click this button and select <em>Clear</em> followed "
                   "by <em>OK</em> in the appearing dialog.</p></qt>" );

const char *_wt_filterdlg_showLater =
        I18N_NOOP( "<qt><p>Check this button to force the confirmation dialog to be "
                   "displayed.</p><p>This is useful if you have defined a ruleset that tags "
                   "messages to be downloaded later. Without the possibility to force "
                   "the dialog popup, these messages could never be downloaded if no "
                   "other large messages were waiting on the server, or if you wanted to "
                   "change the ruleset to tag the messages differently.</p></qt>" );


//=============================================================================
//
// class KMFilterListBox (the filter list manipulator)
//
//=============================================================================
using namespace MailCommon;
KMFilterListBox::KMFilterListBox( const QString & title, QWidget *parent )
    : QGroupBox( title, parent )
{
    QVBoxLayout *layout = new QVBoxLayout();

    //----------- the list box
    mListWidget = new QListWidget(this);
    mListWidget->setMinimumWidth(150);
    mListWidget->setWhatsThis( i18n(_wt_filterlist) );
    mListWidget->setDragDropMode( QAbstractItemView::InternalMove );
    mListWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );
    connect( mListWidget->model(),
             SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
             SLOT(slotRowsMoved(QModelIndex,int,int,QModelIndex,int)) );

    KListWidgetSearchLine *mSearchListWidget = new KListWidgetSearchLine( this, mListWidget );
    mSearchListWidget->setTrapReturnKey( true );
    mSearchListWidget->setClickMessage(
                i18nc( "@info/plain Displayed grayed-out inside the textbox, verb to search",
                       "Search" ) );

    layout->addWidget( mSearchListWidget );
    layout->addWidget( mListWidget );

    //----------- the first row of buttons
    KHBox *hb = new KHBox( this );
    hb->setSpacing( 4 );

    mBtnTop = new KPushButton( QString(), hb );
    mBtnTop->setIcon( KIcon( QLatin1String("go-top") ) );
    mBtnTop->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    mBtnTop->setMinimumSize( mBtnTop->sizeHint() * 1.2 );

    mBtnUp = new KPushButton( QString(), hb );
    mBtnUp->setAutoRepeat( true );
    mBtnUp->setIcon( KIcon( QLatin1String("go-up") ) );
    mBtnUp->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    mBtnUp->setMinimumSize( mBtnUp->sizeHint() * 1.2 );
    mBtnDown = new KPushButton( QString(), hb );
    mBtnDown->setAutoRepeat( true );
    mBtnDown->setIcon( KIcon( QLatin1String("go-down") ) );
    mBtnDown->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    mBtnDown->setMinimumSize( mBtnDown->sizeHint() * 1.2 );

    mBtnBottom = new KPushButton( QString(), hb );
    mBtnBottom->setIcon( KIcon( QLatin1String("go-bottom") ) );
    mBtnBottom->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    mBtnBottom->setMinimumSize( mBtnBottom->sizeHint() * 1.2 );

    mBtnUp->setToolTip( i18nc( "Move selected filter up.", "Up" ) );
    mBtnDown->setToolTip( i18nc( "Move selected filter down.", "Down" ) );
    mBtnTop->setToolTip( i18nc( "Move selected filter to the top.", "Top" ) );
    mBtnBottom->setToolTip( i18nc( "Move selected filter to the bottom.", "Bottom" ) );
    mBtnUp->setWhatsThis( i18n( _wt_filterlist_up ) );
    mBtnDown->setWhatsThis( i18n( _wt_filterlist_down ) );
    mBtnBottom->setWhatsThis( i18n( _wt_filterlist_bottom ) );
    mBtnTop->setWhatsThis( i18n( _wt_filterlist_top ) );

    layout->addWidget( hb );

    //----------- the second row of buttons
    hb = new KHBox( this );
    hb->setSpacing( 4 );
    mBtnNew = new QPushButton( QString(), hb );
    mBtnNew->setIcon( KIcon( QLatin1String("document-new") ) );
    mBtnNew->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    mBtnNew->setMinimumSize( mBtnNew->sizeHint() * 1.2 );
    mBtnCopy = new QPushButton( QString(), hb );
    mBtnCopy->setIcon( KIcon( QLatin1String("edit-copy") ) );
    mBtnCopy->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    mBtnCopy->setMinimumSize( mBtnCopy->sizeHint() * 1.2 );
    mBtnDelete = new QPushButton( QString(), hb );
    mBtnDelete->setIcon( KIcon( QLatin1String("edit-delete") ) );
    mBtnDelete->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    mBtnDelete->setMinimumSize( mBtnDelete->sizeHint() * 1.2 );
    mBtnRename = new QPushButton( i18n( "Rename..." ), hb );
    mBtnNew->setToolTip( i18nc( "@action:button in filter list manipulator", "New" ) );
    mBtnCopy->setToolTip( i18n( "Copy" ) );
    mBtnDelete->setToolTip( i18n( "Delete" ) );
    mBtnNew->setWhatsThis( i18n( _wt_filterlist_new ) );
    mBtnCopy->setWhatsThis( i18n( _wt_filterlist_copy ) );
    mBtnDelete->setWhatsThis( i18n( _wt_filterlist_delete ) );
    mBtnRename->setWhatsThis( i18n( _wt_filterlist_rename ) );

    layout->addWidget( hb );
    setLayout( layout );

    QShortcut *shortcut = new QShortcut( this );
    shortcut->setKey( Qt::Key_Delete );
    connect( shortcut, SIGNAL(activated()), SLOT(slotDelete()) );

    //----------- now connect everything
    connect( mListWidget, SIGNAL(currentRowChanged(int)),
             this, SLOT(slotSelected(int)) );
    connect( mListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
             this, SLOT(slotRename()) );
    connect( mListWidget, SIGNAL(itemChanged(QListWidgetItem*)),
             this, SLOT(slotFilterEnabledChanged(QListWidgetItem*)));

    connect( mListWidget, SIGNAL(itemSelectionChanged()),
             this, SLOT(slotSelectionChanged()));

    connect( mBtnUp, SIGNAL(clicked()),
             this, SLOT(slotUp()) );
    connect( mBtnDown, SIGNAL(clicked()),
             this, SLOT(slotDown()) );
    connect( mBtnTop, SIGNAL(clicked()),
             this, SLOT(slotTop()) );
    connect( mBtnBottom, SIGNAL(clicked()),
             this, SLOT(slotBottom()) );

    connect( mBtnNew, SIGNAL(clicked()),
             this, SLOT(slotNew()) );
    connect( mBtnCopy, SIGNAL(clicked()),
             this, SLOT(slotCopy()) );
    connect( mBtnDelete, SIGNAL(clicked()),
             this, SLOT(slotDelete()) );
    connect( mBtnRename, SIGNAL(clicked()),
             this, SLOT(slotRename()) );

    // the dialog should call loadFilterList()
    // when all signals are connected.
    enableControls();
}

KMFilterListBox::~KMFilterListBox()
{
}

bool KMFilterListBox::itemIsValid( QListWidgetItem *item ) const
{
    if ( !item ) {
        kDebug() << "Called while no filter is selected, ignoring.";
        return false;
    }
    if ( item->isHidden() ) {
        return false;
    }
    return true;
}

void KMFilterListBox::slotFilterEnabledChanged( QListWidgetItem *item )
{
    if ( !item ) {
        kDebug() << "Called while no filter is selected, ignoring.";
        return;
    }
    QListWidgetFilterItem *itemFilter = static_cast<QListWidgetFilterItem*>( item );
    MailCommon::MailFilter *filter = itemFilter->filter();
    filter->setEnabled( ( item->checkState() == Qt::Checked ) );
    emit filterUpdated( filter );
}

void KMFilterListBox::slotRowsMoved( const QModelIndex &,
                                     int sourcestart, int sourceEnd,
                                     const QModelIndex &, int destinationRow )
{
    Q_UNUSED( sourceEnd );
    Q_UNUSED( sourcestart );
    Q_UNUSED( destinationRow );
    enableControls();

    emit filterOrderAltered();
}

void KMFilterListBox::createFilter( const QByteArray &field, const QString &value )
{
    SearchRule::Ptr newRule = SearchRule::createInstance( field, SearchRule::FuncContains, value );

    MailFilter *newFilter = new MailFilter();
    newFilter->pattern()->append( newRule );
    newFilter->pattern()->setName( QString::fromLatin1( "<%1>: %2" ).
                                   arg( QString::fromLatin1( field ) ).
                                   arg( value ) );

    FilterActionDesc *desc = MailCommon::FilterManager::filterActionDict()->value( QLatin1String("transfer") );
    if ( desc ) {
        newFilter->actions()->append( desc->create() );
    }

    insertFilter( newFilter );
    enableControls();
}

void KMFilterListBox::slotUpdateFilterName()
{
    QListWidgetItem *item = mListWidget->currentItem();
    if ( !item ) {
        kDebug() << "Called while no filter is selected, ignoring.";
        return;
    }
    QListWidgetFilterItem *itemFilter = static_cast<QListWidgetFilterItem*>( item );
    MailCommon::MailFilter *filter = itemFilter->filter();

    SearchPattern *p = filter->pattern();
    if ( !p ) {
        return;
    }

    QString shouldBeName = p->name();
    QString displayedName = itemFilter->text();

    if ( shouldBeName.trimmed().isEmpty() ) {
        filter->setAutoNaming( true );
    }

    if ( filter->isAutoNaming() ) {
        // auto-naming of patterns
        if ( !p->isEmpty() && p->first() && !p->first()->field().trimmed().isEmpty() ) {
            shouldBeName = QString::fromLatin1( "<%1>: %2" ).
                    arg( QString::fromLatin1( p->first()->field() ) ).
                    arg( p->first()->contents() );
        } else {
            shouldBeName = QLatin1Char('<') + i18n( "unnamed" ) + QLatin1Char('>');
        }
        p->setName( shouldBeName );
    }

    if ( displayedName == shouldBeName ) {
        return;
    }

    filter->setToolbarName( shouldBeName );

    mListWidget->blockSignals( true );
    itemFilter->setText( shouldBeName );
    mListWidget->blockSignals( false );
}

void KMFilterListBox::slotApplyFilterChanges( KDialog::ButtonCode button )
{
    bool closeAfterSaving;
    if ( button == KDialog::Ok ) {
        closeAfterSaving = true;
    } else if ( button == KDialog::Apply ) {
        closeAfterSaving = false;
    } else {
        return; // ignore close and cancel
    }

    if ( mListWidget->currentItem() ) {
        emit applyWidgets();
        slotSelected( mListWidget->currentRow() );
    }

    // by now all edit widgets should have written back
    // their widget's data into our filter list.

    bool wasCanceled = false;
    const QList<MailFilter*> newFilters = filtersForSaving( closeAfterSaving, wasCanceled );
    if (!wasCanceled)
        MailCommon::FilterManager::instance()->setFilters( newFilters );
}

QList<MailFilter *> KMFilterListBox::filtersForSaving( bool closeAfterSaving, bool &wasCanceled ) const
{
    const_cast<KMFilterListBox*>( this )->applyWidgets(); // signals aren't const
    QList<MailFilter *> filters;
    QStringList emptyFilters;
    QVector<MailCommon::InvalidFilterInfo> listInvalidFilters;
    const int numberOfFilter( mListWidget->count() );
    for ( int i = 0; i <numberOfFilter; ++i ) {
        QListWidgetFilterItem *itemFilter =
                static_cast<QListWidgetFilterItem*>( mListWidget->item( i ) );
        MailFilter *f = new MailFilter( *itemFilter->filter() ); // deep copy

        const QString information = f->purify();
        if ( !f->isEmpty() ) {
            // the filter is valid:
            filters.append( f );
        } else {
            // the filter is invalid:
            emptyFilters << f->name();
            listInvalidFilters.append(MailCommon::InvalidFilterInfo(f->name(), information));
            delete f;
        }
    }

    // report on invalid filters:
    if ( !emptyFilters.empty() ) {
        if ( closeAfterSaving ) {
            QPointer<MailCommon::InvalidFilterDialog> dlg = new MailCommon::InvalidFilterDialog(0);
            dlg->setInvalidFilters(listInvalidFilters);
            if (!dlg->exec()) {
                emit abortClosing();
                wasCanceled = true;
            }
            delete dlg;
        } else {
            QPointer<MailCommon::InvalidFilterDialog> dlg = new MailCommon::InvalidFilterDialog(0);
            dlg->setInvalidFilters(listInvalidFilters);
            if (!dlg->exec()) {
                wasCanceled = true;
            }
            delete dlg;
        }
    }
    return filters;
}

void KMFilterListBox::slotSelectionChanged()
{
    if ( mListWidget->selectedItems().count() > 1 ) {
        resetWidgets();
    }
    enableControls();
}

void KMFilterListBox::slotSelected( int aIdx )
{
    if ( aIdx >= 0 && aIdx < mListWidget->count() ) {
        QListWidgetFilterItem *itemFilter =
                static_cast<QListWidgetFilterItem*>( mListWidget->item( aIdx ) );
        MailFilter *f = itemFilter->filter();

        if ( f ) {
            emit filterSelected( f );
        } else {
            emit resetWidgets();
        }
    } else {
        emit resetWidgets();
    }
    enableControls();
}

void KMFilterListBox::slotNew()
{
    QListWidgetItem *item = mListWidget->currentItem();
    if ( item && item->isHidden() ) {
        return;
    }
    // just insert a new filter.
    insertFilter( new MailFilter() );
    enableControls();
}

void KMFilterListBox::slotCopy()
{
    QListWidgetItem *item = mListWidget->currentItem();
    if ( !itemIsValid( item ) ) {
        return;
    }

    // make sure that all changes are written to the filter before we copy it
    emit applyWidgets();
    QListWidgetFilterItem *itemFilter = static_cast<QListWidgetFilterItem*>( item );

    MailFilter *filter = itemFilter->filter();

    // enableControls should make sure this method is
    // never called when no filter is selected.
    Q_ASSERT( filter );

    // inserts a copy of the current filter.
    MailFilter *copyFilter = new MailFilter( *filter );
    copyFilter->generateRandomIdentifier();
    copyFilter->setShortcut( KShortcut() );

    insertFilter( copyFilter );
    enableControls();
}

void KMFilterListBox::slotDelete()
{
    QListWidgetItem *itemFirst = mListWidget->currentItem();
    if ( !itemIsValid( itemFirst ) ) {
        return;
    }
    const bool uniqFilterSelected = ( mListWidget->selectedItems().count() == 1 );

    QListWidgetFilterItem *itemFilter = static_cast<QListWidgetFilterItem*>( itemFirst );
    MailCommon::MailFilter *filter = itemFilter->filter();
    const QString filterName = filter->pattern()->name();
    if ( uniqFilterSelected ) {
        if ( KMessageBox::questionYesNo(
                 this,
                 i18n( "Do you want to remove the filter \"%1\"?",filterName ),
                 i18n( "Remove Filter" ) ) == KMessageBox::No ) {
            return;
        }
    } else {
        if ( KMessageBox::questionYesNo(
                 this,
                 i18n( "Do you want to remove selected filters?" ),
                 i18n( "Remove Filters" ) ) == KMessageBox::No ) {
            return;
        }
    }

    const int oIdxSelItem = mListWidget->currentRow();
    QList<MailCommon::MailFilter*>lst;

    emit resetWidgets();

    Q_FOREACH ( QListWidgetItem *item, mListWidget->selectedItems() ) {
        QListWidgetFilterItem *itemFilter = static_cast<QListWidgetFilterItem*>( item );

        MailCommon::MailFilter *filter = itemFilter->filter();
        lst << filter;

        // remove the filter from both the listbox
        QListWidgetItem *item2 = mListWidget->takeItem( mListWidget->row( item ) );
        delete item2;
    }
    const int count = mListWidget->count();
    // and set the new current item.
    if ( count > oIdxSelItem ) {
        // oIdxItem is still a valid index
        mListWidget->setCurrentRow( oIdxSelItem );
    } else if ( count ) {
        // oIdxSelIdx is no longer valid, but the
        // list box isn't empty
        mListWidget->setCurrentRow( count - 1 );
    }

    // work around a problem when deleting the first item in a QListWidget:
    // after takeItem, slotSelectionChanged is emitted with 1, but the row 0
    // remains selected and another selectCurrentRow(0) does not trigger the
    // selectionChanged signal
    // (qt-copy as of 2006-12-22 / gungl)
    if ( oIdxSelItem == 0 ) {
        slotSelected( 0 );
    }
    enableControls();

    emit filterRemoved( lst );
}

void KMFilterListBox::slotTop()
{
    QList<QListWidgetItem*> listWidgetItem = selectedFilter();
    if ( listWidgetItem.isEmpty() ) {
        return;
    }

    const int numberOfItem( listWidgetItem.count() );
    if ( ( numberOfItem == 1 ) && ( mListWidget->currentRow() == 0 ) ) {
        kDebug() << "Called while the _topmost_ filter is selected, ignoring.";
        return;
    }

    QListWidgetItem *item = 0;
    bool wasMoved = false;
    for ( int i = 0; i<numberOfItem; ++i ) {
        const int posItem = mListWidget->row( listWidgetItem.at( i ) );
        if ( posItem == i ) {
            continue;
        }
        item = mListWidget->takeItem( mListWidget->row( listWidgetItem.at( i ) ) );
        mListWidget->insertItem( i, item );
        wasMoved = true;
    }

    if ( wasMoved ) {
        enableControls();
        emit filterOrderAltered();
    }
}

QList<QListWidgetItem*> KMFilterListBox::selectedFilter()
{
    QList<QListWidgetItem*> listWidgetItem;
    const int numberOfFilters = mListWidget->count();
    for ( int i = 0; i<numberOfFilters; ++i ) {
        if ( mListWidget->item(i)->isSelected() && !mListWidget->item(i)->isHidden() ) {
            listWidgetItem << mListWidget->item(i);
        }
    }
    return listWidgetItem;
}

QStringList KMFilterListBox::selectedFilterId( SearchRule::RequiredPart& requiredPart, const QString& resource ) const
{
    QStringList listFilterId;
    requiredPart = SearchRule::Envelope;
    const int numberOfFilters = mListWidget->count();
    for ( int i = 0; i <numberOfFilters; ++i ) {
        if ( mListWidget->item(i)->isSelected() && !mListWidget->item(i)->isHidden() ) {
            const QString id =
                    static_cast<QListWidgetFilterItem*>( mListWidget->item( i ) )->filter()->identifier();
            listFilterId << id;
            requiredPart = qMax(requiredPart,
                                static_cast<QListWidgetFilterItem*>( mListWidget->item( i ) )->filter()->requiredPart(resource));
        }
    }
    return listFilterId;
}

void KMFilterListBox::slotBottom()
{
    QList<QListWidgetItem*> listWidgetItem = selectedFilter();
    if ( listWidgetItem.isEmpty() ) {
        return;
    }

    const int numberOfElement( mListWidget->count() );
    const int numberOfItem( listWidgetItem.count() );
    if ( ( numberOfItem == 1 ) && ( mListWidget->currentRow() == numberOfElement - 1 ) ) {
        kDebug() << "Called while the _last_ filter is selected, ignoring.";
        return;
    }

    QListWidgetItem *item = 0;
    int j = 0;
    bool wasMoved = false;
    for ( int i = numberOfItem-1; i>= 0; --i, j++ ) {
        const int posItem = mListWidget->row( listWidgetItem.at( i ) );
        if ( posItem == ( numberOfElement-1 -j ) ) {
            continue;
        }
        item = mListWidget->takeItem( mListWidget->row( listWidgetItem.at( i ) ) );
        mListWidget->insertItem( numberOfElement-j, item );
        wasMoved = true;
    }

    if ( wasMoved ) {
        enableControls();
        emit filterOrderAltered();
    }
}

void KMFilterListBox::slotUp()
{
    QList<QListWidgetItem*> listWidgetItem = selectedFilter();
    if ( listWidgetItem.isEmpty() ) {
        return;
    }

    const int numberOfItem( listWidgetItem.count() );
    if ( ( numberOfItem == 1 ) && ( mListWidget->currentRow() == 0 ) ) {
        kDebug() << "Called while the _topmost_ filter is selected, ignoring.";
        return;
    }
    bool wasMoved = false;

    for ( int i = 0; i<numberOfItem; ++i ) {
        const int posItem = mListWidget->row( listWidgetItem.at( i ) );
        if ( posItem == i ) {
            continue;
        }
        swapNeighbouringFilters( posItem, posItem - 1 );
        wasMoved = true;
    }
    if ( wasMoved ) {
        enableControls();
        emit filterOrderAltered();
    }
}

void KMFilterListBox::slotDown()
{
    QList<QListWidgetItem*> listWidgetItem = selectedFilter();
    if ( listWidgetItem.isEmpty() ) {
        return;
    }

    const int numberOfElement( mListWidget->count() );
    const int numberOfItem( listWidgetItem.count() );
    if ( ( numberOfItem == 1 ) && ( mListWidget->currentRow() == numberOfElement - 1 ) ) {
        kDebug() << "Called while the _last_ filter is selected, ignoring.";
        return;
    }

    int j = 0;
    bool wasMoved = false;
    for ( int i = numberOfItem-1; i>= 0; --i, j++ ) {
        const int posItem = mListWidget->row( listWidgetItem.at( i ) );
        if ( posItem == ( numberOfElement-1 -j ) ) {
            continue;
        }
        swapNeighbouringFilters( posItem, posItem + 1 );
        wasMoved = true;
    }

    if ( wasMoved ) {
        enableControls();
        emit filterOrderAltered();
    }
}

void KMFilterListBox::slotRename()
{
    QListWidgetItem *item = mListWidget->currentItem();
    if ( !itemIsValid(item) ) {
        return;
    }
    QListWidgetFilterItem *itemFilter = static_cast<QListWidgetFilterItem*>( item );

    bool okPressed = false;
    MailFilter *filter = itemFilter->filter();

    // enableControls should make sure this method is
    // never called when no filter is selected.
    Q_ASSERT( filter );

    // allow empty names - those will turn auto-naming on again
    QValidator *validator = new QRegExpValidator( QRegExp( QLatin1String(".*") ), 0 );
    QString newName =
            KInputDialog::getText (
                i18n( "Rename Filter" ),
                i18n( "Rename filter \"%1\" to:\n(leave the field empty for automatic naming)",
                      filter->pattern()->name() ), /*label*/
                filter->pattern()->name(), /* initial value */
                &okPressed,
                window(),
                validator );
    delete validator;

    if ( !okPressed ) {
        return;
    }

    if ( newName.isEmpty() ) {
        // bait for slotUpdateFilterName to
        // use automatic naming again.
        filter->pattern()->setName( QLatin1String("<>") );
        filter->setAutoNaming( true );
    } else {
        filter->pattern()->setName( newName );
        filter->setAutoNaming( false );
    }

    slotUpdateFilterName();

    emit filterUpdated( filter );
}

void KMFilterListBox::enableControls()
{
    const int currentIndex = mListWidget->currentRow();
    const bool theFirst = ( currentIndex == 0 );
    const int numberOfElement( mListWidget->count() );
    const bool theLast = ( currentIndex >= numberOfElement - 1 );
    const bool aFilterIsSelected = ( currentIndex >= 0 );

    const int numberOfSelectedItem( mListWidget->selectedItems().count() );
    const bool uniqFilterSelected = ( numberOfSelectedItem == 1 );
    const bool allItemSelected = ( numberOfSelectedItem == numberOfElement );

    mBtnUp->setEnabled( aFilterIsSelected && ( ( uniqFilterSelected && !theFirst ) ||
                                               ( !uniqFilterSelected ) ) && !allItemSelected );
    mBtnDown->setEnabled( aFilterIsSelected &&
                          ( ( uniqFilterSelected && !theLast ) ||
                            ( !uniqFilterSelected ) ) && !allItemSelected );

    mBtnCopy->setEnabled( aFilterIsSelected && uniqFilterSelected );
    mBtnDelete->setEnabled( aFilterIsSelected );
    mBtnRename->setEnabled( aFilterIsSelected && uniqFilterSelected );

    mBtnTop->setEnabled( aFilterIsSelected &&
                         ( ( uniqFilterSelected && !theFirst ) ||
                           ( !uniqFilterSelected ) ) && !allItemSelected );

    mBtnBottom->setEnabled( aFilterIsSelected &&
                            ( ( uniqFilterSelected && !theLast ) ||
                              ( !uniqFilterSelected ) ) && !allItemSelected );
    if ( aFilterIsSelected ) {
        mListWidget->scrollToItem( mListWidget->currentItem() );
    }
}

void KMFilterListBox::loadFilterList( bool createDummyFilter )
{
    Q_ASSERT( mListWidget );
    setEnabled( false );
    emit resetWidgets();
    // we don't want the insertion to
    // cause flicker in the edit widgets.
    blockSignals( true );

    // clear both lists
    mListWidget->clear();

    const QList<MailFilter*> filters = MailCommon::FilterManager::instance()->filters();
    foreach ( MailFilter *filter, filters ) {
        QListWidgetFilterItem *item =
                new QListWidgetFilterItem( filter->pattern()->name(), mListWidget );
        item->setFilter( new MailFilter( *filter ) );
        mListWidget->addItem( item );
    }

    blockSignals(false);
    setEnabled(true);

    // create an empty filter when there's none, to avoid a completely
    // disabled dialog (usability tests indicated that the new-filter
    // button is too hard to find that way):
    const int numberOfItem( mListWidget->count() );
    if ( numberOfItem == 0 ) {
        if (createDummyFilter ) {
            slotNew();
        }
    } else {
        mListWidget->setCurrentRow( 0 );
    }

    enableControls();
}

void KMFilterListBox::insertFilter( MailFilter *aFilter )
{
    // must be really a filter...
    Q_ASSERT( aFilter );
    const int currentIndex = mListWidget->currentRow();
    // if mIdxSelItem < 0, QListBox::insertItem will append.
    QListWidgetFilterItem *item = new QListWidgetFilterItem( aFilter->pattern()->name() );
    item->setFilter( aFilter );
    mListWidget->insertItem( currentIndex, item );
    mListWidget->clearSelection();
    if ( currentIndex < 0 ) {
        mListWidget->setCurrentRow( mListWidget->count() - 1 );
    } else {
        // insert just before selected
        mListWidget->setCurrentRow( currentIndex );
    }

    emit filterCreated();
    emit filterOrderAltered();
}

void KMFilterListBox::appendFilter( MailFilter *aFilter )
{
    QListWidgetFilterItem *item =
            new QListWidgetFilterItem( aFilter->pattern()->name(), mListWidget );

    item->setFilter( aFilter );
    mListWidget->addItem( item );

    emit filterCreated();
}

void KMFilterListBox::swapNeighbouringFilters( int untouchedOne, int movedOne )
{
    // must be neighbours...
    Q_ASSERT( untouchedOne - movedOne == 1 || movedOne - untouchedOne == 1 );

    // untouchedOne is at idx. to move it down(up),
    // remove item at idx+(-)1 w/o deleting it.
    QListWidgetItem *item = mListWidget->takeItem( movedOne );
    // now selected item is at idx(idx-1), so
    // insert the other item at idx, ie. above(below).
    mListWidget->insertItem( untouchedOne, item );
}

QListWidgetFilterItem::QListWidgetFilterItem( const QString &text, QListWidget *parent )
    : QListWidgetItem( text, parent ), mFilter( 0 )
{
}

QListWidgetFilterItem::~QListWidgetFilterItem()
{
    delete mFilter;
}

void QListWidgetFilterItem::setFilter( MailCommon::MailFilter *filter )
{
    mFilter = filter;
    setCheckState( filter->isEnabled() ? Qt::Checked :  Qt::Unchecked );
}

MailCommon::MailFilter *QListWidgetFilterItem::filter()
{
    return mFilter;
}
