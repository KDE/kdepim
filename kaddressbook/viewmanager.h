/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <qdict.h>
#include <qwidget.h>
#include <kaddressbookview.h>

class QDropEvent;
class QWidgetStack;

class KAction;
class KSelectAction;

class FilterSelectionWidget;

namespace KAB { class Core; }
namespace KABC { class AddressBook; }

/**
  The view manager manages the views and everything related to them. The
  manager will load the views at startup and display a view when told to
  make one active.

  The view manager will also create and manage all dialogs directly related to
  views (ie: AddView, ConfigureView, DeleteView, etc).
 */
class ViewManager : public QWidget
{
  Q_OBJECT

  public:
    ViewManager( KAB::Core *core, QWidget *parent, const char *name = 0 );
    ~ViewManager();

    void restoreSettings();
    void saveSettings();

    void unloadViews();

    QStringList selectedUids() const;
    QStringList selectedEmails() const;
    KABC::Addressee::List selectedAddressees() const;

    void setFilterSelectionWidget( FilterSelectionWidget *wdg );

    KABC::Field *currentSortField() const;
    KABC::Field::List viewFields() const;

  public slots:
    void setSelected( const QString &uid = QString::null, bool selected = true );
    void setFirstSelected( bool selected = true );

    void refreshView( const QString &uid = QString::null );
    void editView();
    void deleteView();
    void addView();

  protected slots:
    /**
      Called whenever the user drops something in the active view.
      This method will try to decode what was dropped, and if it was
      a valid addressee, add it to the addressbook.
     */
    void dropped( QDropEvent* );

    /**
      Called whenever the user attempts to start a drag in the view.
      This method will convert all the selected addressees into text (vcard)
      and create a drag object.
     */
    void startDrag();

  signals:
    /**
      Emitted whenever the user selects an entry in the view.
     */
    void selected( const QString &uid );

    /**
      Emitted whenever the user activates an entry in the view.
     */
    void executed( const QString &uid );

    /**
      Emitted whenever the address book is modified in some way.
     */
    void modified();

    /**
      Emitted whenever a url is dragged on a view.
     */
    void urlDropped( const KURL& );

    /**
      Emitted whenever the sort field of a view has changed.
     */
    void sortFieldChanged();

    /**
      Emitted whenever the view fields changed.
     */
    void viewFieldsChanged();

  private slots:
    void setActiveView( const QString &name );
    void setActiveFilter( int index );
    void configureFilters();

  private:
    void createViewFactories();
    QStringList filterNames() const;    
    int filterPosition( const QString &name ) const;
    QStringList viewNames() const;
    int viewPosition( const QString &name ) const;
    void initActions();
    void initGUI();

    KAB::Core *mCore;

    Filter mCurrentFilter;
    Filter::List mFilterList;

    QDict<KAddressBookView> mViewDict;
    QDict<ViewFactory> mViewFactoryDict;
    QStringList mViewNameList;

    QWidgetStack *mViewWidgetStack;
    KAddressBookView *mActiveView;

    KAction *mActionDeleteView;
    KSelectAction *mActionSelectView;

    FilterSelectionWidget *mFilterSelectionWidget;
};

#endif
