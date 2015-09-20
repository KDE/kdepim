/*
  Filter Dialog
  Author: Marc Mutz <mutz@kde.org>
  based upon work by Stefan Taferner <taferner@kde.org>

  Copyright (c) 2011-2015 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef MAILCOMMON_KMFILTERDIALOG_H
#define MAILCOMMON_KMFILTERDIALOG_H

#include "mailcommon_export.h"
#include "mailcommon/filteraction.h"
#include "filterimporterexporter.h"
#include "mailfilter.h"
#include "mailcommon/searchpattern.h"

#include <QDialog>

#include <QGroupBox>
#include <QList>
#include <QListWidgetItem>
#include <QTreeWidget>

class KActionCollection;
class KIconButton;
class KKeySequenceWidget;

class QCheckBox;
class QLabel;
class QListWidget;
class QPushButton;
class QRadioButton;
class QPushButton;
class QDialogButtonBox;
namespace MailCommon
{
class SearchPatternEdit;
class FilterActionWidgetLister;
class FolderRequester;
class KMFilterAccountList;
class KMFilterListBox;
}

class KJob;

/**
 * The filter dialog. This is a non-modal dialog used to manage the filters.
 * It should only be called through KMFilterMgr::openDialog. The dialog
 * consists of three main parts:
 *
 * @li The KMFilterListBox in the left half allows the user to
 * select a filter to be displayed using the widgets on the right
 * half. It also has buttons to delete filters, add new ones, to
 * rename them and to change their order (maybe you will be able to
 * move the filters around by dragging later, and to optimize the
 * filters by trying to apply them to all locally available
 * KMMessage in turn and thus profiling which filters (and which
 * rules of the search patterns) matches most often and sorting the
 * filter/rules list according to the results, but I first want the
 * basic functionality in place).
 *
 * @li The SearchPatternEdit in the upper-right quarter allows
 * the user to modify the filter criteria.
 *
 * @li The KMFilterActionEdit in the lower-right quarter allows
 * the user to select the actions that will be executed for any
 * message that matches the search pattern.
 *
 * @li (tbi) There will be another widget that will allow the user to
 * select to which folders the filter may be applied and whether it
 * should be applied on outbound or inbound message transfers or both
 * or none (meaning the filter is only applied when the user
 * explicitly hits CTRL-J). I'm not sure whether there should be a
 * per-folder filter list or a single list where you can select the
 * names of folders this rule will be applied to.
 *
 * Upon creating the dialog, a (deep) copy of the current filter list
 * is made by KMFilterListBox. The changed filters are local to
 * KMFilterListBox until the user clicks the 'Apply' button.
 *
 * NOTE: Though this dialog is non-modal, it completely ignores all
 * the stuff that goes on behind the scenes with folders esp. folder
 * creation, move and create. The widgets that depend on the filter
 * list and the filters that use folders as parameters are not
 * updated as you expect. I hope this will change sometime soon.
 *
 * KMFilterDialog supports the creation of new filters through context
 * menus, dubbed "rapid filters". Call KMFilterMgr::createFilter
 * to use this. That call will be delivered to this dialog, which in
 * turn delivers it to the KMFilterListBox.
 *
 * If you change the (DocBook) anchor for the filter dialog help,
 * make sure to change @p const @p QString @p KMFilterDialogHelpAnchor
 * in kmfilterdlg.cpp accordingly.
 *
 * @short The filter dialog.
 * @author Marc Mutz <mutz@kde.org>, based upon work by Stefan Taferner <taferner@kde.org>.
 * @see MailCommon::MailFilter KMFilterActionEdit SearchPatternEdit KMFilterListBox
 */
namespace MailCommon
{
class MAILCOMMON_EXPORT KMFilterDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Creates the filter dialog. The only class which should be able
     * to do this is KMFilterMgr. This ensures that there is only a
     * single filter dialog.
     */
    explicit KMFilterDialog(const QList<KActionCollection *> &actionCollection,
                            QWidget *parent = Q_NULLPTR, bool createDummyFilter = true);

    /**
     * Called from KMFilterMgr. Creates a new filter and presets
     * the first rule with "field equals value". Internally forwarded to
     * KMFilterListBox::createFilter. You should instead call
     * KMFilterMgr::createFilter.
     */
    void createFilter(const QByteArray &field, const QString &value);

public Q_SLOTS:
    /**
     * Internally connected to KMFilterListBox::filterSelected.
     * Just does a simple check and then calls
     * SearchPatternEdit::setSearchPattern and
     * KMFilterActionEdit::setActionList.
     */
    void slotFilterSelected(MailCommon::MailFilter *aFilter);

    /** Override QDialog::accept to allow disabling close */
    void accept() Q_DECL_OVERRIDE;

protected Q_SLOTS:
    void slotApplicabilityChanged();
    void slotApplicableAccountsChanged();
    void slotStopProcessingButtonToggled(bool aChecked);
    void slotConfigureShortcutButtonToggled(bool aChecked);
    void slotShortcutChanged(const QKeySequence &newSeq);
    void slotConfigureToolbarButtonToggled(bool aChecked);
    void slotFilterActionIconChanged(const QString &icon);
    void slotReset();
    void slotUpdateFilter();
    void slotSaveSize();

    /**
     * Called when the dialog is closed (finished).
     */
    void slotFinished();

    /**
     * Update the list of accounts shown in the advanced tab.
     */
    void slotUpdateAccountList();

    /**
     * Called when a user clicks the import filters button. Pops up
     * a dialog asking the user which file to import from and which
     * of the filters in that file to import.
     */
    void slotImportFilter(QAction *);

    /**
     * Called when a user clicks the export filters button. Pops up
     * a dialog asking the user which filters to export and which
     * file to export to.
     */
    void slotExportFilters();

    /**
     * Called when a user decides to continue editing invalid filters
     */
    void slotDisableAccept();

    /**
     * Called whenever a change in the filters configuration is detected,
     * to enable the Apply button.
     */
    void slotDialogUpdated();

    /**
     * Called wherenever the apply button is pressed.
     */
    void slotApply();

    void slotRunFilters();

    void slotFetchItemsForFolderDone(KJob *job);

    void slotFolderChanged(const Akonadi::Collection &);

private Q_SLOTS:
    void slotExportAsSieveScript();
    void slotHelp();

private:
    void importFilters(MailCommon::FilterImporterExporter::FilterType type);

protected:
    bool event(QEvent *e) Q_DECL_OVERRIDE;

    /** The widget that contains the ListBox showing the filters, and the
        controls to remove filters, add new ones and to change their order. */
    KMFilterListBox *mFilterList;

    /** The widget that allows editing of the filter pattern. */
    MailCommon::SearchPatternEdit *mPatternEdit;

    /** The widget that allows editing of the filter actions. */
    MailCommon::FilterActionWidgetLister *mActionLister;

    /** Lets the user select whether to apply this filter on
       inbound/outbound messages, both, or only on explicit CTRL-J. */
    QCheckBox *mApplyOnIn, *mApplyOnOut, *mApplyBeforeOut, *mApplyOnCtrlJ;

    /** For a filter applied to inbound messages selects whether to apply
        this filter to all accounts or to selected accounts only. */
    QRadioButton *mApplyOnForAll, *mApplyOnForTraditional, *mApplyOnForChecked;

    /** ListView that shows the accounts in the advanced tab */
    KMFilterAccountList *mAccountList;

    QCheckBox *mStopProcessingHere;
    QCheckBox *mConfigureShortcut;
    QCheckBox *mConfigureToolbar;
    QLabel *mFilterActionLabel;
    KIconButton *mFilterActionIconButton;
    KKeySequenceWidget *mKeySeqWidget;
    QGroupBox *mAdvOptsGroup;

    MailCommon::MailFilter *mFilter;
    MailCommon::FolderRequester *mFolderRequester;
    QPushButton *mRunNow;
    QDialogButtonBox *buttonBox;
    bool mDoNotClose;
    bool mIgnoreFilterUpdates;
};

}

#endif /*kmfilterdialog_h*/
