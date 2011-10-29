/*
  Copyright (c) 2000, 2001, 2002 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Sérgio Martins <sergio.martins@kdab.com>

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

#ifndef INCIDENCEEDITOR_CATEGORYDIALOG_H
#define INCIDENCEEDITOR_CATEGORYDIALOG_H

#include "incidenceeditors-ng_export.h"

#include <KDialog>

class CategoryWidgetBase;

namespace CalendarSupport {
  class CategoryConfig;
}

namespace IncidenceEditorNG {

class AutoCheckTreeWidget;

class INCIDENCEEDITORS_NG_EXPORT CategoryWidget : public QWidget
{
  Q_OBJECT
  public:
    explicit CategoryWidget( CalendarSupport::CategoryConfig *config, QWidget *parent=0 );
    ~CategoryWidget();

    void setCategories( const QStringList &categoryList = QStringList() );
    void setCategoryList( const QStringList &categories );

    void setSelected( const QStringList &selList );
    QStringList selectedCategories() const;
    QStringList selectedCategories( QString &categoriesStr );

    void setAutoselectChildren( bool autoselectChildren );

    void hideButton();
    void hideHeader();

    AutoCheckTreeWidget *listView() const;

  public Q_SLOTS:
    void clear();

  private Q_SLOTS:
    void handleTextChanged( const QString &newText );
    void handleSelectionChanged();
    void handleColorChanged( const QColor & );
    void addCategory();
    void removeCategory();

  private:
    QStringList mCategoryList;
    CategoryWidgetBase *mWidgets;
    CalendarSupport::CategoryConfig *mCategoryConfig;
};

class INCIDENCEEDITORS_NG_EXPORT CategoryDialog : public KDialog
{
  Q_OBJECT
  public:
    explicit CategoryDialog( CalendarSupport::CategoryConfig *cfg, QWidget *parent = 0 );
    ~CategoryDialog();

    QStringList selectedCategories() const;
    void setCategoryList( const QStringList &categories );

    void setAutoselectChildren( bool autoselectChildren );
    void setSelected( const QStringList &selList );

  public Q_SLOTS:
    void slotOk();
    void slotApply();
    void updateCategoryConfig();

  Q_SIGNALS:
    void categoriesSelected( const QString & );
    void categoriesSelected( const QStringList & );

  private:
    CategoryWidget *mWidgets;
    CalendarSupport::CategoryConfig *mCategoryConfig;
    class CategorySelectDialogPrivate;
    CategorySelectDialogPrivate *d;
};

}

#endif
