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

#include <qwidget.h>
#include <qmap.h>
#include <qptrlist.h>
#include <qstringlist.h>

class QSignalMapper;
class QWidgetStack;
class KActionCollection;

namespace KAB {
class Core;
}

class ExtensionData
{
  public:
    ExtensionData();
    typedef QValueList<ExtensionData> List;

    KToggleAction* action;
    KAB::ExtensionWidget *widget;
    QString identifier;
    QString title;
    int weight;
    bool isDetailsExtension;
};

class ExtensionManager : public QObject
{
  Q_OBJECT

  public:
    ExtensionManager( QWidget *extensionBar, QWidgetStack *detailsStack, KAB::Core *core, QObject *parent, const char *name = 0 );
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

    QWidget *activeDetailsWidget() const;
 
  public slots:
    void setSelectionChanged();
    void createActions();

  signals:

    void detailsWidgetActivated( QWidget* widget );
    void detailsWidgetDeactivated( QWidget* widget );
    void modified( const KABC::Addressee::List& );
    void deleted( const QStringList& );

  private slots:
    void activationToggled( const QString &extid );

  private:
    void createExtensionWidgets();
    void setExtensionActive( const QString &extid, bool active ); 

  private:
    QWidget *mExtensionBar;
    KAB::Core *mCore;
    QMap<QString, ExtensionData> mExtensionMap;
    QStringList mActiveExtensions;
    QSignalMapper *mMapper;
    QPtrList<KAction> mActionList;
    KActionCollection *mActionCollection;
    QSplitter *mSplitter;
    QWidgetStack *mDetailsStack; 
    QWidget *mActiveDetailsWidget;
};

#endif
