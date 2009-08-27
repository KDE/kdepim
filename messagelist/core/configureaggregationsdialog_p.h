/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#ifndef __MESSAGELIST_CORE_CONFIGUREAGGREGATIONSDIALOG_P_H__
#define __MESSAGELIST_CORE_CONFIGUREAGGREGATIONSDIALOG_P_H__

#include "core/configureaggregationsdialog.h"

namespace MessageList
{

namespace Core
{

class Aggregation;
class AggregationEditor;
class AggregationListWidget;
class AggregationListWidgetItem;

/**
 * The dialog used for configuring MessageList::Aggregation sets.
 *
 * This is managed by MessageList::Manager. Take a look at it first
 * if you want to display this dialog.
 */
class ConfigureAggregationsDialog::Private
{
public:
  Private( ConfigureAggregationsDialog *owner )
    : q( owner ) { }

  /**
   * Called by MessageList::Manager to display an instance of this dialog.
   * If an instance is already existing (and maybe visible) it will be just activated.
   */
  static void display( QWidget * parent, const QString &preselectAggregationId = QString() );

  /**
   * This is called when MessageList::Manager is being destroyed: kill
   * the dialog if it's still there.
   */
  static void cleanup();

  // Private implementation

  void fillAggregationList();
  QString uniqueNameForAggregation( QString baseName, Aggregation * skipAggregation = 0 );
  AggregationListWidgetItem * findAggregationItemByName( const QString &name, Aggregation * skipAggregation = 0 );
  AggregationListWidgetItem * findAggregationItemByAggregation( Aggregation * set );
  AggregationListWidgetItem * findAggregationItemById( const QString &aggregationId );
  void commitEditor();
  void selectAggregationById( const QString &aggregationId );

  // Private slots

  void aggregationListCurrentItemChanged( QListWidgetItem * cur, QListWidgetItem * prev );
  void newAggregationButtonClicked();
  void cloneAggregationButtonClicked();
  void deleteAggregationButtonClicked();
  void editedAggregationNameChanged();
  void okButtonClicked();


  ConfigureAggregationsDialog * const q;

  static ConfigureAggregationsDialog * mInstance;
  AggregationListWidget *mAggregationList;
  AggregationEditor *mEditor;
  QPushButton *mNewAggregationButton;
  QPushButton *mCloneAggregationButton;
  QPushButton *mDeleteAggregationButton;
};

} // namespace Core

} // namespace MessageList

#endif //!__MESSAGELIST_CORE_CONFIGUREAGGREGATIONSDIALOG_P_H__
