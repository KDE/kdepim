/*
  This file is part of the KDE Kontact Plugin Interface Library.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KONTACTINTERFACES_SUMMARY_H
#define KONTACTINTERFACES_SUMMARY_H

#include "kontactinterfaces_export.h"

#include <QtGui/QWidget>

class KStatusBar;
class QMouseEvent;
class QDragEnterEvent;
class QDropEvent;

namespace Kontact
{

/**
  Summary widget for display in the Summary View plugin.
 */
class KONTACTINTERFACES_EXPORT Summary : public QWidget
{
  Q_OBJECT
  public:
    explicit Summary( QWidget *parent );

    virtual ~Summary();

    /**
      Return logical height of summary widget. This is used to calculate how
      much vertical space relative to other summary widgets this widget will
      use in the summary view.
    */
    virtual int summaryHeight() const;

    /**
      Creates a heading for a typical summary view with an icon and a heading.
     */
    QWidget *createHeader( QWidget *parent, const QString &iconname, const QString &heading );

    /**
      Return list of strings identifying configuration modules for this summary
      part. The string has to be suitable for being passed to
      KCMultiDialog::addModule().
    */
    virtual QStringList configModules() const;

  public Q_SLOTS:
    virtual void configChanged() {}

    /**
      This is called if the displayed information should be updated.
      @param force true if the update was requested by the user
    */
    virtual void updateSummary( bool force = false );

  Q_SIGNALS:
    void message( const QString &message );
    void summaryWidgetDropped( QWidget *target, QWidget *widget, int alignment );

  protected:
    virtual void mousePressEvent( QMouseEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );
    virtual void dragEnterEvent( QDragEnterEvent * );
    virtual void dropEvent( QDropEvent * );

  private:
    class Private;
    Private *const d;
};

}

#endif
