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

#ifndef __MESSAGELIST_UTILS_AGGREGATIONEDITOR_H__
#define __MESSAGELIST_UTILS_AGGREGATIONEDITOR_H__

#include <messagelist/utils/optionseteditor.h>

class KComboBox;

namespace MessageList
{

namespace Core
{

class Aggregation;

} // namespace Core

namespace Utils
{

/**
 * A widget that allows editing a single MessageList::Aggregation.
 *
 * Used by ConfigureAggregationsDialog.
 */
class AggregationEditor : public OptionSetEditor
{
    Q_OBJECT
public:
    explicit AggregationEditor(QWidget *parent);
    ~AggregationEditor();

private:
    Core::Aggregation *mCurrentAggregation;  // shallow, may be null!

    // Grouping, Threading and Sorting tab
    KComboBox *mGroupingCombo;
    KComboBox *mGroupExpandPolicyCombo;
    KComboBox *mThreadingCombo;
    KComboBox *mThreadLeaderCombo;
    KComboBox *mThreadExpandPolicyCombo;
    // Advanced tab
    KComboBox *mFillViewStrategyCombo;

public:
    /**
    * Sets the Aggregation to be edited.
    * Saves and forgets any previously Aggregation that was being edited.
    * The set parameter may be 0: in this case the editor is simply disabled.
    */
    void editAggregation(Core::Aggregation *set);

    /**
    * Returns the Aggregation currently edited by this AggregationEditor.
    * May be 0.
    */
    Core::Aggregation *editedAggregation() const
    {
        return mCurrentAggregation;
    }

    /**
    * Explicitly commits the changes in the editor to the edited Aggregation, if any.
    */
    void commit();

signals:
    /**
    * This is triggered when the aggregation name changes in the editor text field.
    * It's connected to the Aggregation configuration dialog which updates
    * the list of aggregations with the new name.
    */
    void aggregationNameChanged();

private:

    // Helpers for filling the various editing elements

    void fillGroupingCombo();
    void fillGroupExpandPolicyCombo();
    void fillThreadingCombo();
    void fillThreadLeaderCombo();
    void fillThreadExpandPolicyCombo();
    void fillFillViewStrategyCombo();
    void setReadOnly(bool readOnly);

private slots:

    // Internal handlers for editing element interaction

    void groupingComboActivated(int idx);
    void threadingComboActivated(int idx);
    virtual void slotNameEditTextEdited(const QString &newName);

};

} // namespace Utils

} // namespace MessageList

#endif //!__MESSAGELIST_UTILS_AGGREGATIONEDITOR_H__
