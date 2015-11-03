/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef KMFILTERLISTBOX_H
#define KMFILTERLISTBOX_H

#include "search/searchpattern.h"
#include <QGroupBox>
#include <QListWidgetItem>
class QListWidget;
class QPushButton;
class QModelIndex;
class KListWidgetSearchLine;
/**
 * This is a complex widget that is used to manipulate a mail program's filter
 * list. It consists of an internal list of filters, which is a deep
 * copy of the list KMFilterMgr manages, a QListBox displaying that list,
 * and a few buttons used to create new filters, delete them, rename them
 * and change the order of filters.
 *
 * It does not provide means to change the actual filter (besides the name),
 * but relies on auxiliary widgets (SearchPatternEdit and KMFilterActionEdit)
 * to do that.
 *
 * Communication with this widget is quite easy: simply create an instance,
 * connect the signals filterSelected, resetWidgets and applyWidgets with
 * a slot that does the right thing and there you go...
 *
 * This widget will operate on it's own copy of the filter list as
 * long as you don't call slotApplyFilterChanges. It will then
 * transfer the altered filter list back to KMFilterMgr.
 *
 * @short A complex widget that allows managing a list of MailCommon::MailFilter's.
 * @author Marc Mutz <mutz@kde.org>, based upon work by Stefan Taferner <taferner@kde.org>.
 * @see MailCommon::MailFilter KMFilterDialog KMFilterActionEdit SearchPatternEdit
 */

namespace MailCommon
{
class MailFilter;

class QListWidgetFilterItem : public QListWidgetItem
{
public:
    explicit QListWidgetFilterItem(const QString &text, QListWidget *parent = Q_NULLPTR);
    ~QListWidgetFilterItem();

    void setFilter(MailCommon::MailFilter *filter);
    MailCommon::MailFilter *filter();

private:
    MailCommon::MailFilter *mFilter;
};

class KMFilterListBox : public QGroupBox
{
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    explicit KMFilterListBox(const QString &title, QWidget *widget = Q_NULLPTR);

    /**
     * Destructor.
     */
    ~KMFilterListBox();

    /**
     * Called from KMFilterDialog. Creates a new filter and presets
     * the first rule with "field equals value". It's there mainly to
     * support "rapid filter creation" from a context menu. You should
     * instead call KMFilterMgr::createFilter.
     * @see KMFilterMgr::createFilter KMFilterDialog::createFilter
     */
    void createFilter(const QByteArray &field, const QString &value);

    /**
     * Loads the filter list and selects the first filter. Should be
     * called when all signals are connected properly. If createDummyFilter
     * is true, an empty filter is created to improve the usability of the
     * dialog in case no filter has been defined so far.
     */
    void loadFilterList(bool createDummyFilter);

    void insertFilter(MailCommon::MailFilter *aFilter);

    void appendFilter(MailCommon::MailFilter *aFilter);

    /**
     * Returns a list of _copies_ of the current list of filters.
     * The list owns the contents and thus the caller needs to clean them up.
     * @param closeAfterSaving If true user is given option to continue editing
     * after being warned about invalid filters. Otherwise, user is just warned.
     */
    QList<MailCommon::MailFilter *> filtersForSaving(bool closeAfterSaving, bool &wasCanceled) const;

    QStringList selectedFilterId(SearchRule::RequiredPart &requiredPart, const QString &resource) const;

Q_SIGNALS:
    /**
     * Emitted when a new filter has been selected by the user or if the
     * current filter has changed after a 'new' or 'delete' operation.
     */
    void filterSelected(MailCommon::MailFilter *filter);

    /**
     * Emitted when this widget wants the edit widgets to let go of
     * their filter reference. Everyone holding a reference to a filter
     * should update it from the contents of the widgets used to edit
     * it and set their internal reference to 0.
     */
    void resetWidgets();

    /**
     * Emitted when this widget wants the edit widgets to apply the changes
     * to the current filter.
     */
    void applyWidgets();

    /**
     * Emitted when the user decides to continue editing after being warned
     * about invalid filters.
     */
    void abortClosing() const;

    /**
     * Emitted when a new filter is created.
     */
    void filterCreated();

    /**
     * Emitted when a filter is deleted.
     */
    void filterRemoved(const QList<MailCommon::MailFilter *> &filter);

    /**
     * Emitted when a filter is updated (e.g. renamed).
     */
    void filterUpdated(MailCommon::MailFilter *filter);

    /**
     * Emitted whenever the order in which the filters are displayed is changed.
     */
    void filterOrderAltered();

public Q_SLOTS:
    /**
     * Called when the name of a filter might have changed (e.g. through
     * changing the first rule in SearchPatternEdit). Updates the corresponding
     * entry in the listbox and (if necessary) auto-names the filter.
     */
    void slotUpdateFilterName();

    void slotAccepted();
    void slotApplied();

protected Q_SLOTS:
    /**
     * Called when the user clicks on a filter in the filter list.
     * Calculates the corresponding filter and emits the filterSelected signal.
     */
    void slotSelected(int aIdx);

    /**
     * Called when the user clicks the 'New' button.
     * Creates a new empty filter just before the current one.
     */
    void slotNew();

    /**
     * Called when the user clicks the 'Copy' button.  Creates a copy
     * of the current filter and inserts it just before the current one.
     */
    void slotCopy();

    /**
     * Called when the user clicks the 'Delete' button.  Deletes the
     * current filter.
     */
    void slotDelete();

    /**
     * Called when the user clicks the 'Up' button. Moves the current
     * filter up one line.
     */
    void slotUp();

    /**
     * Called when the user clicks the 'Down' button. Moves the current
     * filter down one line.
     */
    void slotDown();

    /**
     * Called when the user clicks the 'Rename' button. Pops up a
     * dialog prompting to enter the new name.
     */
    void slotRename();

    void slotRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int);

    /**
     * Called when the user clicks the 'Top' button. Moves the current
     * filter to top.
     */
    void slotTop();

    /**
     * Called when the user clicks the 'Bottom' button. Moves the current
     * filter to bottom.
     */
    void slotBottom();

    void slotFilterEnabledChanged(QListWidgetItem *item);

    void slotSelectionChanged();

protected:
    /** The listbox displaying the filter list. */
    QListWidget *mListWidget;

    /** The various action buttons. */
    QPushButton *mBtnNew;
    QPushButton *mBtnCopy;
    QPushButton *mBtnDelete;
    QPushButton *mBtnUp;
    QPushButton *mBtnDown;
    QPushButton *mBtnRename;
    QPushButton *mBtnTop;
    QPushButton *mBtnBottom;
    KListWidgetSearchLine *mSearchListWidget;
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
private:
    void applyFilterChanged(bool closeAfterSaving);
    void enableControls();
    bool itemIsValid(QListWidgetItem *item) const;
    QList<QListWidgetItem *> selectedFilter();
    void swapNeighbouringFilters(int untouchedOne, int movedOne);
};
}
#endif // KMFILTERLISTBOX_H
