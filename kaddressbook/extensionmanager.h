/*
    This file is part of KAddressbook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef EXTENSIONMANAGER_H
#define EXTENSIONMANAGER_H

#include "extensionwidget.h"

#include <tqwidget.h>
#include <tqmap.h>
#include <tqptrlist.h>
#include <tqstringlist.h>

class TQSignalMapper;
class TQWidgetStack;
class KActionCollection;

namespace KAB {
class Core;
}

class ExtensionData
{
  public:
    ExtensionData();
    typedef TQValueList<ExtensionData> List;

    KToggleAction* action;
    KAB::ExtensionWidget *widget;
    TQString identifier;
    TQString title;
    int weight;
    bool isDetailsExtension;
};

class ExtensionManager : public QObject
{
  Q_OBJECT

  public:
    ExtensionManager( TQWidget *extensionBar, TQWidgetStack *detailsStack, KAB::Core *core, TQObject *parent, const char *name = 0 );
    ~ExtensionManager();

    /**
      Restores the extension manager specific settings.
     */
    void restoreSettings();

    /**
      Saves the extension manager specific settings.
     */
    void saveSettings();

    /**
      Rereads the extension manager specific settings with some
      additional initialization stuff.
     */
    void reconfigure();

    /**
      Returns whether the quickedit extension is currently visible.
     */
    bool isQuickEditVisible() const;

    TQWidget *activeDetailsWidget() const;
 
  public slots:
    void setSelectionChanged();
    void createActions();

  signals:

    void detailsWidgetActivated( TQWidget* widget );
    void detailsWidgetDeactivated( TQWidget* widget );
    void modified( const KABC::Addressee::List& );
    void deleted( const TQStringList& );

  private slots:
    void activationToggled( const TQString &extid );

  private:
    void createExtensionWidgets();
    void setExtensionActive( const TQString &extid, bool active ); 

  private:
    TQWidget *mExtensionBar;
    KAB::Core *mCore;
    TQMap<TQString, ExtensionData> mExtensionMap;
    TQStringList mActiveExtensions;
    TQSignalMapper *mMapper;
    TQPtrList<KAction> mActionList;
    KActionCollection *mActionCollection;
    TQSplitter *mSplitter;
    TQWidgetStack *mDetailsStack; 
    TQWidget *mActiveDetailsWidget;
};

#endif
