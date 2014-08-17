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

#ifndef __MESSAGELIST_UTILS_CONFIGUREAGGREGATIONSDIALOG_H__
#define __MESSAGELIST_UTILS_CONFIGUREAGGREGATIONSDIALOG_H__

#include <QDialog>

#include <QListWidget>

#include <messagelist/messagelist_export.h>
#include <KConfigGroup>

class QPushButton;

namespace MessageList
{

namespace Core
{

class Manager;

} // namespace Core

namespace Utils
{

/**
 * The dialog used for configuring MessageList::Aggregation sets.
 *
 * This is managed by MessageList::Manager. Take a look at it first
 * if you want to display this dialog.
 */
class MESSAGELIST_EXPORT ConfigureAggregationsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigureAggregationsDialog( QWidget *parent = 0 );
    ~ConfigureAggregationsDialog();

    void selectAggregation( const QString &aggregationId );

private:
    Q_PRIVATE_SLOT(d, void aggregationListItemClicked(QListWidgetItem* cur))
    Q_PRIVATE_SLOT(d, void newAggregationButtonClicked())
    Q_PRIVATE_SLOT(d, void cloneAggregationButtonClicked())
    Q_PRIVATE_SLOT(d, void deleteAggregationButtonClicked())
    Q_PRIVATE_SLOT(d, void editedAggregationNameChanged())
    Q_PRIVATE_SLOT(d, void okButtonClicked())
    Q_PRIVATE_SLOT(d, void importAggregationButtonClicked())
    Q_PRIVATE_SLOT(d, void exportAggregationButtonClicked())

    class Private;
    Private * const d;
};

} // namespace Utils

} // namespace MessageList

#endif //!__MESSAGELIST_UTILS_CONFIGUREAGGREGATIONSDIALOG_H__
