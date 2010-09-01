/*
  This file is part of the KDE Kontact.

  Copyright (C) 2003 Cornelius Schumacher <schumacher@kde.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#ifndef KONTACT_ICONSIDEPANEBASE_H
#define KONTACT_ICONSIDEPANEBASE_H

#include <tqtooltip.h>

#include <klistbox.h>

#include "sidepanebase.h"
#include "prefs.h"


class TQSignalMapper;

namespace KParts { class Part; }

namespace Kontact
{

class Core;
class IconSidePane;
class Plugin;
class Navigator;

enum IconViewMode { LargeIcons = 48, NormalIcons = 32, SmallIcons = 22, ShowText = 3, ShowIcons = 5 };


/**
  A TQListBoxPixmap Square Box with an optional icon and a text
  underneath.
*/
class EntryItem : public QListBoxItem
{
  public:
    EntryItem( Navigator *, Kontact::Plugin * );
    ~EntryItem();

    Kontact::Plugin *plugin() const { return mPlugin; }

    const TQPixmap *pixmap() const { return &mPixmap; }

    Navigator* navigator() const;

    void setHover( bool );
    void setPaintActive( bool );
    bool paintActive() const { return mPaintActive; }
    /**
      returns the width of this item.
    */
    virtual int width( const TQListBox * ) const;
    /**
      returns the height of this item.
    */
    virtual int height( const TQListBox * ) const;

  protected:
    void reloadPixmap();

    virtual void paint( TQPainter *p );

  private:
    Kontact::Plugin *mPlugin;
    TQPixmap mPixmap;
    bool mHasHover;
    bool mPaintActive;
};

/**
 * Tooltip that changes text depending on the item it is above.
 * Compliments of "Practical Qt" by Dalheimer, Petersen et al.
 */
class EntryItemToolTip : public QToolTip
{
  public:
    EntryItemToolTip( TQListBox* parent )
      : TQToolTip( parent->viewport() ), mListBox( parent )
      {}
  protected:
    void maybeTip( const TQPoint& p ) {
      // We only show tooltips when there are no texts shown
      if ( Prefs::self()->sidePaneShowText() ) return;
      if ( !mListBox ) return;
      TQListBoxItem* item = mListBox->itemAt( p );
      if ( !item ) return;
      const TQRect itemRect = mListBox->itemRect( item );
      if ( !itemRect.isValid() ) return;

      const EntryItem *entryItem = static_cast<EntryItem*>( item );
      TQString tipStr = entryItem->text();
      tip( itemRect, tipStr );
    }
  private:
    TQListBox* mListBox;
};

/**
  Navigation pane showing all parts relevant to the user
*/
class Navigator : public KListBox
{
    Q_OBJECT
  public:
    Navigator( IconSidePane *parent = 0, const char *name = 0 );

    virtual void setSelected( TQListBoxItem *, bool );

    void updatePlugins( TQValueList<Kontact::Plugin*> plugins );

    TQSize sizeHint() const;

    void highlightItem( EntryItem* item );

    IconViewMode viewMode() { return mViewMode; }
    IconViewMode sizeIntToEnum(int size) const;
    const TQPtrList<KAction> & actions() { return mActions; }
    bool showIcons() const { return mShowIcons; }
    bool showText() const { return mShowText; }
  signals:
    void pluginActivated( Kontact::Plugin * );

  protected:
    void dragEnterEvent( TQDragEnterEvent * );
    void dragMoveEvent ( TQDragMoveEvent * );
    void dropEvent( TQDropEvent * );
    void resizeEvent( TQResizeEvent * );
    void enterEvent( TQEvent* );
    void leaveEvent( TQEvent* );

    void setHoverItem( TQListBoxItem*, bool );
    void setPaintActiveItem( TQListBoxItem*, bool );

  protected slots:
    void slotExecuted( TQListBoxItem * );
    void slotMouseOn( TQListBoxItem *item );
    void slotMouseOff();
    void slotShowRMBMenu( TQListBoxItem *, const TQPoint& );
    void shortCutSelected( int );
    void slotStopHighlight();

  private:
    IconSidePane *mSidePane;
    IconViewMode mViewMode;

    TQListBoxItem* mMouseOn;

    EntryItem*    mHighlightItem;

    TQSignalMapper *mMapper;
    TQPtrList<KAction> mActions;
    bool mShowIcons;
    bool mShowText;
};

class IconSidePane : public SidePaneBase
{
    Q_OBJECT
  public:
    IconSidePane( Core *core, TQWidget *parent, const char *name = 0 );
    ~IconSidePane();

    virtual void indicateForegrunding( Kontact::Plugin* );

  public slots:
    virtual void updatePlugins();
    virtual void selectPlugin( Kontact::Plugin* );
    virtual void selectPlugin( const TQString &name );
    const TQPtrList<KAction> & actions() { return mNavigator->actions(); }

  private:
    Navigator *mNavigator;
};

}

#endif
