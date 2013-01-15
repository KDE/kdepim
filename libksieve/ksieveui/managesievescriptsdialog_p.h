#ifndef KSIEVE_KSIEVEUI_MANAGESIEVESCRIPTSDIALOG_P_H
#define KSIEVE_KSIEVEUI_MANAGESIEVESCRIPTSDIALOG_P_H

#include <QContextMenuEvent>
#include <QRadioButton>
#include <QTreeWidget>

namespace KSieveUi {

/**
 * A radio button which has an associated tree widget item, and which selectes that
 * tree widget item when receiving focus.
 */
class ItemRadioButton : public QRadioButton
{
  public:

    ItemRadioButton( QTreeWidgetItem *item )
        : QRadioButton(), mItem( item )
    {}

    ~ItemRadioButton() {
      mItem = 0;
    }

    /**
     * When calling QTreeWidget::clear() on a tree widget whose items have an
     * ItemRadioButton as item widget, focusInEvent is being called. Since
     * calling treeWidget() there would lead to a crash, as the tree widget is being
     * cleared, set this to true to prevent the crash.
     */
    static void setTreeWidgetIsBeingCleared( bool clearing ) {
      mTreeWidgetIsBeingCleared = clearing;
    }

  protected:

    /**
     * Reimplemented from QRadioButton to set the focus to the item
     */
    virtual void focusInEvent( QFocusEvent * )
    {
      if ( mItem && !mTreeWidgetIsBeingCleared ) {
        mItem->setSelected( true );
        mItem->treeWidget()->setCurrentItem( mItem );
      }
    }

  private:

    static bool mTreeWidgetIsBeingCleared;
    QTreeWidgetItem *mItem;
};

}

#endif
