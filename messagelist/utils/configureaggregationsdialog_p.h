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

#ifndef __MESSAGELIST_UTILS_CONFIGUREAGGREGATIONSDIALOG_P_H__
#define __MESSAGELIST_UTILS_CONFIGUREAGGREGATIONSDIALOG_P_H__

#include "utils/configureaggregationsdialog.h"

namespace MessageList
{

namespace Core
{

class Aggregation;

} // namespace Core

namespace Utils
{

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
    Private(ConfigureAggregationsDialog *owner)
        : q(owner) { }

    // Private implementation

    void fillAggregationList();
    QString uniqueNameForAggregation(const QString &baseName, Core::Aggregation *skipAggregation = 0);
    AggregationListWidgetItem *findAggregationItemByName(const QString &name, Core::Aggregation *skipAggregation = 0);
    AggregationListWidgetItem *findAggregationItemByAggregation(Core::Aggregation *set);
    AggregationListWidgetItem *findAggregationItemById(const QString &aggregationId);
    void commitEditor();

    // Private slots

    void aggregationListItemClicked(QListWidgetItem *cur);
    void newAggregationButtonClicked();
    void cloneAggregationButtonClicked();
    void deleteAggregationButtonClicked();
    void editedAggregationNameChanged();
    void okButtonClicked();
    void exportAggregationButtonClicked();
    void importAggregationButtonClicked();
    void updateButton(QListWidgetItem *cur);

    ConfigureAggregationsDialog *const q;

    AggregationListWidget *mAggregationList;
    AggregationEditor *mEditor;
    QPushButton *mNewAggregationButton;
    QPushButton *mCloneAggregationButton;
    QPushButton *mDeleteAggregationButton;
    QPushButton *mExportAggregationButton;
    QPushButton *mImportAggregationButton;
};

} // namespace Utils

} // namespace MessageList

#endif //!__MESSAGELIST_UTILS_CONFIGUREAGGREGATIONSDIALOG_P_H__
