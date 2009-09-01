/* Copyright 2009 James Bendig <james@imptalk.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __MESSAGELIST_UTILS_AGGREGATIONCOMBOBOX_H__
#define __MESSAGELIST_UTILS_AGGREGATIONCOMBOBOX_H__

#include <messagelist/messagelist_export.h>
#include <KComboBox>

namespace MessageList
{

namespace Core
{

class Aggregation;
class StorageModel;

}

namespace Utils
{

/**
 * A specialized KComboBox that lists all message list aggregations.
 */
class MESSAGELIST_EXPORT AggregationComboBox : public KComboBox
{
  Q_OBJECT
  friend class AggregationConfigButton;

public:
  explicit AggregationComboBox( QWidget * parent );
  ~AggregationComboBox();

  void writeDefaultConfig() const;

  void writeStorageModelConfig( Core::StorageModel *storageModel, bool isPrivateSetting ) const;
  void readStorageModelConfig( Core::StorageModel *storageModel, bool &isPrivateSetting );

public Q_SLOTS:
  void selectDefault();

private:
  Q_PRIVATE_SLOT(d, void slotLoadAggregations())

  class Private;
  Private * const d;
};

} // namespace Utils

} // namespace MessageList

#endif //!__MESSAGELIST_UTILS_AGGREGATIONCOMBOBOX_H__

