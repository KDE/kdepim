/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma implementation "EmpathFilterManagerDialog.h"
#endif

// KDE includes
#include <klocale.h>
#include <kapp.h>
#include <kglobal.h>
#include <kconfig.h>

// Local includes
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathFilterManagerDialog.h"
#include "EmpathFilterEditDialog.h"
#include "EmpathFilterList.h"
#include "EmpathUtilities.h"
#include "EmpathFilter.h"
#include "EmpathUIUtils.h"

EmpathFilterManagerDialog::EmpathFilterManagerDialog(QWidget * parent)
    :    QDialog(parent, "FilterManager", true)
{
    setCaption(i18n("Filter Settings"));

    l_about_ =
        new QLabel(
        i18n("The following filters will be applied, in order, to new mail."),
            this, "l_filtersFolder");

    lv_filters_ = new QListView(this, "lv_matches");

    lv_filters_->addColumn(i18n("Name"));
    lv_filters_->addColumn(i18n("Priority"));
    lv_filters_->setFrameStyle(QFrame::Box | QFrame::Sunken);
    lv_filters_->setSorting(1);
    lv_filters_->setAllColumnsShowFocus(true);

    filtersButtonBox_ = new KButtonBox(this, KButtonBox::VERTICAL);

    pb_addFilter_       = filtersButtonBox_->addButton(i18n("Add filter"));
    pb_editFilter_      = filtersButtonBox_->addButton(i18n("Edit filter"));
    pb_removeFilter_    = filtersButtonBox_->addButton(i18n("Delete filter"));
    filtersButtonBox_->addStretch();
    pb_moveUp_          = filtersButtonBox_->addButton(i18n("Move up"));
    pb_moveDown_        = filtersButtonBox_->addButton(i18n("Move down"));
    
    filtersButtonBox_->addStretch();
   
    filtersButtonBox_->layout();

///////////////////////////////////////////////////////////////////////////////
// Button box

    buttonBox_    = new KButtonBox(this);
    pb_help_    = buttonBox_->addButton(i18n("&Help"));    
    buttonBox_->addStretch();
    pb_OK_        = buttonBox_->addButton(i18n("&OK"));
    pb_OK_->setDefault(true);
    pb_apply_    = buttonBox_->addButton(i18n("&Apply"));
    pb_cancel_    = buttonBox_->addButton(i18n("&Cancel"));
    
    buttonBox_->layout();
    
    QObject::connect(pb_OK_,        SIGNAL(clicked()),  SLOT(s_OK()));
    QObject::connect(pb_apply_,     SIGNAL(clicked()),  SLOT(s_apply()));
    QObject::connect(pb_cancel_,    SIGNAL(clicked()),  SLOT(s_cancel()));
    QObject::connect(pb_help_,      SIGNAL(clicked()),  SLOT(s_help()));
/////////////////////////////////////////////////////////////////////////////


    QVBoxLayout * layout = new QVBoxLayout(this, dialogSpace);

    QHBoxLayout * layout0 = new QHBoxLayout(layout);

    layout0->addWidget(lv_filters_);
    layout0->addWidget(filtersButtonBox_);
    
    layout->addStretch(10);
    layout->addWidget(buttonBox_);

    QObject::connect(
        pb_addFilter_, SIGNAL(clicked()),
        this, SLOT(s_addFilter()));
    
    QObject::connect(
        pb_editFilter_, SIGNAL(clicked()),
        this, SLOT(s_editFilter()));
    
    QObject::connect(
        pb_removeFilter_, SIGNAL(clicked()),
        this, SLOT(s_removeFilter()));
    
    QObject::connect(
        pb_moveUp_, SIGNAL(clicked()),
        this, SLOT(s_moveUp()));
    
    QObject::connect(
        pb_moveDown_, SIGNAL(clicked()),
        this, SLOT(s_moveDown()));
}

    void
EmpathFilterManagerDialog::loadData()
{
    update();
}

    void
EmpathFilterManagerDialog::s_addFilter()
{
    EmpathFilter * newFilter = new EmpathFilter(i18n("Unnamed"));
    
    EmpathFilterEditDialog filterEditDialog(newFilter, this);
    
    if (filterEditDialog.exec() != QDialog::Accepted) {
        delete newFilter;
        newFilter = 0;
        return;
    }
    
    // 0 is the first- filter becomes the last when it's added.
    newFilter->setPriority(empath->filterList().count());
    empath->filterList().append(newFilter);
    update();
}

EmpathFilterManagerDialog::~EmpathFilterManagerDialog()
{
    // Empty.
}

    void
EmpathFilterManagerDialog::s_editFilter()
{
    EmpathFilter * editedFilter =
        ((EmpathFilterListItem *)lv_filters_->currentItem())->filter();
    
    if (editedFilter == 0)
        return;
    
    EmpathFilterEditDialog filterEditDialog(editedFilter, this);

    if (filterEditDialog.exec() != QDialog::Accepted)
        return;
    
    update();
}

    void
EmpathFilterManagerDialog::s_removeFilter()
{
    EmpathFilter * editedFilter =
        ((EmpathFilterListItem *)lv_filters_->currentItem())->filter();
    
    if (editedFilter == 0)
        return;
    
    empath->filterList().remove(editedFilter);
    
    update();
}

    void
EmpathFilterManagerDialog::s_moveUp()
{
    EmpathFilterListItem * currentItem =
        ((EmpathFilterListItem *)lv_filters_->currentItem());
    
    EmpathFilter * editedFilter = currentItem->filter();
    
    if (editedFilter == 0)
        return;
    
    empath->filterList().raisePriority(editedFilter);
    
    update();
}

    void
EmpathFilterManagerDialog::s_moveDown()
{
    EmpathFilterListItem * currentItem =
        ((EmpathFilterListItem *)lv_filters_->currentItem());
    
    EmpathFilter * editedFilter = currentItem->filter();
    
    if (editedFilter == 0)
        return;
    
    empath->filterList().lowerPriority(editedFilter);
    
    update();
}

    void
EmpathFilterManagerDialog::update()
{
    EmpathFilterListItem * currentItem = 0;
    
    currentItem = (EmpathFilterListItem *)lv_filters_->currentItem();
    
    QString selected;

    if (currentItem != 0)
        selected = currentItem->filter()->name();
    
    lv_filters_->setUpdatesEnabled(false);

    lv_filters_->clear();
    
    QListViewItem * reselect = 0;

    EmpathFilterListIterator it(empath->filterList());

    for (; it.current(); ++it) {
        EmpathFilterListItem * i =
            new EmpathFilterListItem(lv_filters_, it.current());
        if (it.current()->name() == selected)
            reselect = (QListViewItem *)i;    
    }
    
    lv_filters_->setUpdatesEnabled(true);
    lv_filters_->triggerUpdate();
    
    if (reselect != 0) {
        lv_filters_->setCurrentItem(reselect);
        lv_filters_->setSelected(reselect, true);
    }
}

    void
EmpathFilterManagerDialog::saveData()
{
    empath->filterList().saveConfig();
}

    void
EmpathFilterManagerDialog::s_OK()
{
    hide();
    s_apply();
    KGlobal::config()->sync();
    accept();
}

    void
EmpathFilterManagerDialog::s_help()
{
    // STUB
}

    void
EmpathFilterManagerDialog::s_apply()
{
    if (applied_) {
        pb_apply_->setText(i18n("&Apply"));
        KGlobal::config()->rollback(true);
        KGlobal::config()->reparseConfiguration();
        applied_ = false;
    } else {
        pb_apply_->setText(i18n("&Revert"));
        pb_cancel_->setText(i18n("&Close"));
        applied_ = true;
    }
    saveData();
}

    void
EmpathFilterManagerDialog::s_cancel()
{
    if (!applied_)
        KGlobal::config()->rollback(true);
    reject();
}

// vim:ts=4:sw=4:tw=78
