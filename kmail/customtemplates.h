/*   -*- mode: C++; c-file-style: "gnu" -*-
 *   kmail: KDE mail client
 *   This file: Copyright (C) 2006 Dmitry Morozhnikov
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <config.h>

#ifndef CUSTOMTEMPLATES_H
#define CUSTOMTEMPLATES_H

#include "customtemplates_base.h"
#include "templatesinsertcommand.h"

#include <kshortcut.h>

struct CustomTemplateItem;
typedef TQDict<CustomTemplateItem> CustomTemplateItemList;
class KShortcut;

class CustomTemplates : public CustomTemplatesBase
{
  Q_OBJECT

  public:

    enum Type { TUniversal, TReply, TReplyAll, TForward };

  public:

    CustomTemplates( TQWidget *parent = 0, const char *name = 0 );
    ~CustomTemplates();

    void load();
    void save();

    TQString indexToType( int index );

  public slots:

    void slotInsertCommand( TQString cmd, int adjustCursor = 0 );

    void slotTextChanged();

    void slotAddClicked();
    void slotRemoveClicked();
    void slotListSelectionChanged();
    void slotTypeActivated( int index );
    void slotShortcutCaptured( const KShortcut &shortcut );
  void slotNameChanged( const TQString& );
  signals:

    void changed();

  protected:

    void setRecipientsEditsEnabled( bool enabled );

    TQListViewItem *mCurrentItem;
    CustomTemplateItemList mItemList;

    /// These templates will be deleted when we're saving.
    TQStringList mItemsToDelete;

    TQPixmap mReplyPix;
    TQPixmap mReplyAllPix;
    TQPixmap mForwardPix;

    /// Whether or not to emit the changed() signal. This is useful to disable when loading
    /// templates, which changes the UI without user action
    bool mBlockChangeSignal;

};

struct CustomTemplateItem
{
  CustomTemplateItem() {}
  CustomTemplateItem( const TQString &name,
                      const TQString &content,
                      KShortcut &shortcut,
                      CustomTemplates::Type type,
                      TQString to, TQString cc ) :
    mName( name ), mContent( content ), mShortcut(shortcut), mType( type ),
    mTo( to ), mCC( cc ) {}

  TQString mName, mContent;
  KShortcut mShortcut;
  CustomTemplates::Type mType;
  TQString mTo, mCC;
};

#endif // CUSTOMTEMPLATES_H
