/*
   This file is part of KDE Kontact.

   Copyright (C) 2003 Sven Lüppken <sven@kde.org>
   Copyright (C) 2003 Tobias König <tokoe@kde.org>
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef SUMMARYVIEW_PART_H
#define SUMMARYVIEW_PART_H

#include <tqdatetime.h>
#include <tqmap.h>

#include <kparts/part.h>

#include "core.h"
#include "dropwidget.h"

namespace Kontact
{
  class Plugin;
  class Summary;
}

namespace KParts
{
  class PartActivateEvent;
}

class QFrame;
class QLabel;
class QGridLayout;
class KAction;
class KCMultiDialog;

class SummaryViewPart : public KParts::ReadOnlyPart
{
  Q_OBJECT

  public:
    SummaryViewPart( Kontact::Core *core, const char *widgetName,
                     const KAboutData *aboutData,
                     TQObject *parent = 0, const char *name = 0 );
    ~SummaryViewPart();

  public slots:
    void slotTextChanged();
    void slotAdjustPalette();
    void setDate( const TQDate& newDate );
    void updateSummaries();

  signals:
    void textChanged( const TQString& );

  protected:
    virtual bool openFile();
    virtual void partActivateEvent( KParts::PartActivateEvent *event );

  protected slots:
    void slotConfigure();
    void updateWidgets();
    void summaryWidgetMoved( TQWidget *target, TQWidget *widget, int alignment );

  private:
    void initGUI( Kontact::Core *core );
    void loadLayout();
    void saveLayout();
    TQString widgetName( TQWidget* ) const;

    TQStringList configModules() const;

    TQMap<TQString, Kontact::Summary*> mSummaries;
    Kontact::Core *mCore;
    DropWidget *mFrame;
    TQFrame *mMainWidget;
    TQVBoxLayout *mMainLayout;
    TQVBoxLayout *mLeftColumn;
    TQVBoxLayout *mRightColumn;
    TQLabel *mUsernameLabel;
    TQLabel *mDateLabel;
    KAction *mConfigAction;

    TQStringList mLeftColumnSummaries;
    TQStringList mRightColumnSummaries;
};

#endif
