/*
    This file is part of KOrganizer.

    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KORG_RESOURCEVIEW_H
#define KORG_RESOURCEVIEW_H

#include "calendarview.h"

#include <libkcal/resourcecalendar.h>
#include <tqlistview.h>

namespace KCal {
class CalendarResources;
}
using namespace KCal;
class KListView;
class ResourceView;
class QPushButton;

class ResourceViewFactory : public CalendarViewExtension::Factory
{
  public:
    ResourceViewFactory( KCal::CalendarResources *calendar,
                         CalendarView *view );

    CalendarViewExtension *create( TQWidget * );

    ResourceView *resourceView() const;

  private:
    KCal::CalendarResources *mCalendar;
    CalendarView *mView;
    ResourceView *mResourceView;
};


class ResourceItem : public QCheckListItem
{
  public:
    ResourceItem( KCal::ResourceCalendar *resource, ResourceView *view,
                  KListView *parent );
    ResourceItem( KCal::ResourceCalendar *resource, const TQString& sub,
                  const TQString& label, ResourceView *view,
                  ResourceItem* parent );

    KCal::ResourceCalendar *resource() { return mResource; }
    const TQString& resourceIdentifier() { return mResourceIdentifier; }
    bool isSubresource() const { return mIsSubresource; }
    void createSubresourceItems();
    void setStandardResource( bool std );

    void update();

    virtual void paintCell(TQPainter *p, const TQColorGroup &cg,
      int column, int width, int alignment);

    void setResourceColor(TQColor& color);
    TQColor &resourceColor() {return mResourceColor;}
  protected:
    void stateChange( bool active );

    void setGuiState();
    TQColor mResourceColor;

  private:
    KCal::ResourceCalendar *mResource;
    ResourceView *mView;
    bool mBlockStateChange;
    bool mIsSubresource;
    TQString mResourceIdentifier;
    bool mSubItemsCreated;
    bool mIsStandardResource;
};

/**
  This class provides a view of calendar resources.
*/
class ResourceView : public CalendarViewExtension
{
    Q_OBJECT
  public:
    ResourceView( KCal::CalendarResources *calendar, TQWidget *parent = 0,
                  const char *name = 0 );
    ~ResourceView();

    KCal::CalendarResources *calendar() const { return mCalendar; }

    void updateView();

    void emitResourcesChanged();

    void requestClose( ResourceCalendar * );

    void showButtons( bool visible );

  public slots:
    void addResourceItem( ResourceCalendar * );
    void updateResourceItem( ResourceCalendar * );

  signals:
    void resourcesChanged();

  protected:
    ResourceItem *findItem( ResourceCalendar * );
    ResourceItem *findItemByIdentifier( const TQString& id );
    ResourceItem *currentItem();

  protected slots:
    void addResource();
    void removeResource();
    void editResource();
    void currentChanged( TQListViewItem* );
    void slotSubresourceAdded( ResourceCalendar *, const TQString &,
                               const TQString &resource,const TQString& label );

    void slotSubresourceRemoved( ResourceCalendar *, const TQString &,
                                 const TQString & );
    void closeResource( ResourceCalendar * );

    void contextMenuRequested ( TQListViewItem *i, const TQPoint &pos, int );

    void assignColor();
    void disableColor();
    void showInfo();

    void reloadResource();
    void saveResource();

    void setStandard();
    void updateResourceList();

  private:
    KListView *mListView;
    KCal::CalendarResources *mCalendar;
    TQPushButton *mAddButton;
    TQPushButton *mDeleteButton;
    TQPushButton *mEditButton;
    TQPtrList<ResourceCalendar> mResourcesToClose;
};

#endif
