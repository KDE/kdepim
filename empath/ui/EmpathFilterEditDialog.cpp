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
# pragma implementation "EmpathFilterEditDialog.h"
#endif

// Qt includes
#include <qmessagebox.h>
#include <qlayout.h>

// KDE includes
#include <klocale.h>
#include <kapp.h>
#include <kbuttonbox.h>

// Local includes
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathMatcher.h"
#include "EmpathSeparatorWidget.h"
#include "EmpathMatchPropertiesDialog.h"
#include "EmpathFolderChooserWidget.h"
#include "EmpathFilterActionDialog.h"
#include "EmpathFilterEditDialog.h"
#include "EmpathFilter.h"
#include "EmpathFilterList.h"
#include "EmpathUIUtils.h"

EmpathFilterEditDialog::EmpathFilterEditDialog
    (EmpathFilter * filter, QWidget * parent)
    :   QDialog(parent, "FilterEditDialog", true),
        filter_(filter)
{
    setCaption(i18n("Edit Filters"));
    ASSERT(filter_ != 0);
    
    QLabel * l_name = new QLabel(i18n("Filter name"), this, "l_name");
    
    le_name_ = new QLineEdit(this, "le_name");
    
    QLabel * l_arrives =
        new QLabel(i18n("Folder"), this, "l_arrives");

    fcw_arrives_ = new EmpathFolderChooserWidget(this);
    
    lb_matches_ = new QListBox(this, "lb_matches");

    KButtonBox * exprButtonBox
        = new KButtonBox(this, KButtonBox::VERTICAL);

    pb_addMatch_    = exprButtonBox->addButton(i18n("Add expression"));
    pb_editMatch_   = exprButtonBox->addButton(i18n("Edit expression"));
    pb_removeMatch_ = exprButtonBox->addButton(i18n("Remove expression"));
    
    exprButtonBox->layout();

    l_action_ = new QLabel(this, "l_action");

    pb_editAction_ =
        new QPushButton(i18n("Edit action"), this, "pb_editAction");
    
    QObject::connect(pb_editAction_, SIGNAL(clicked()), SLOT(s_editAction()));
    
    KButtonBox * buttonBox = new KButtonBox(this);

    pb_help_    = buttonBox->addButton(i18n("&Help"));    
    buttonBox->addStretch();
    pb_OK_      = buttonBox->addButton(i18n("&OK"));
    pb_cancel_  = buttonBox->addButton(i18n("&Cancel"));
    
    buttonBox->layout();

    QVBoxLayout * layout        = new QVBoxLayout(this, dialogSpace);
    QHBoxLayout * nameLayout    = new QHBoxLayout(layout);
    layout->addWidget(new EmpathSeparatorWidget(this));
    QHBoxLayout * arrivesLayout = new QHBoxLayout(layout);
    layout->addWidget(new EmpathSeparatorWidget(this));
    QHBoxLayout * matchesLayout = new QHBoxLayout(layout);
    layout->addWidget(new EmpathSeparatorWidget(this));
    QHBoxLayout * actionLayout  = new QHBoxLayout(layout);

    arrivesLayout   ->  addWidget(l_arrives);
    arrivesLayout   ->  addWidget(fcw_arrives_);

    matchesLayout   ->  addWidget(lb_matches_);
    matchesLayout   ->  addWidget(exprButtonBox);    

    actionLayout    ->  addWidget(l_action_);
    actionLayout    ->  addWidget(pb_editAction_);

    nameLayout      ->  addWidget(l_name);
    nameLayout      ->  addWidget(le_name_);

    layout->addStretch(10);

    layout->addWidget(new EmpathSeparatorWidget(this));
    layout->addWidget(buttonBox);

    update();
    
    QObject::connect(pb_addMatch_,    SIGNAL(clicked()), SLOT(s_addExpr()));
    QObject::connect(pb_editMatch_,   SIGNAL(clicked()), SLOT(s_editExpr()));
    QObject::connect(pb_removeMatch_, SIGNAL(clicked()), SLOT(s_removeExpr()));

    QObject::connect(pb_OK_,        SIGNAL(clicked()), SLOT(s_OK()));
    QObject::connect(pb_cancel_,    SIGNAL(clicked()), SLOT(s_cancel()));
    QObject::connect(pb_help_,      SIGNAL(clicked()), SLOT(s_help()));
}

EmpathFilterEditDialog::~EmpathFilterEditDialog()
{
    // Empty.
}
        
    void
EmpathFilterEditDialog::s_OK()
{
    EmpathFilterListIterator it(empath->filterList());

    for (; it.current(); ++it) {
    
        if (it.current()->name() == le_name_->text()) {

            QMessageBox::information(this, "Empath",
                i18n("You already have a filter with that name"),
                i18n("OK"));

            return;
        }
    }
    
    hide();
    filter_->setName(le_name_->text());
    filter_->setURL(fcw_arrives_->url());
    filter_->save();
    accept();
}

    void
EmpathFilterEditDialog::s_cancel()
{
    reject();
}

    void
EmpathFilterEditDialog::s_help()
{
    // STUB
}

    void
EmpathFilterEditDialog::s_addExpr()
{
    EmpathMatcher * m = new EmpathMatcher;
    EmpathMatchPropertiesDialog mpd(this, m);
    
    if (mpd.exec() != QDialog::Accepted) return;

    filter_->matchExprList()->append(m);
    lb_matches_->insertItem(m->description());
}

    void
EmpathFilterEditDialog::s_editExpr()
{
    int i = lb_matches_->currentItem();

    if (i == -1)
        return; // No item selected.
    
    EmpathMatchPropertiesDialog mpd(this, filter_->matchExprList()->at(i));
    mpd.exec();
    
    update();
}

    void
EmpathFilterEditDialog::s_removeExpr()
{
    int i = lb_matches_->currentItem();

    if (i == -1)
        return; // No item selected.

    filter_->matchExprList()->remove(i);
    update();
}

    void
EmpathFilterEditDialog::s_editAction()
{
    ASSERT(filter_ != 0);

    EmpathFilterActionDialog fDlg(filter_, this, "filterActionDialog");
    
    if (fDlg.exec() != QDialog::Accepted)
        return;

    l_action_->setText(filter_->actionDescription());
}

    void
EmpathFilterEditDialog::update()
{
    ASSERT(filter_);
    
    le_name_->setText(filter_->name());
    
    fcw_arrives_->setURL(filter_->url());

    lb_matches_->clear();

    EmpathMatcherListIterator it(*(filter_->matchExprList()));

    for (; it.current(); ++it) {
        lb_matches_->insertItem(it.current()->description());
    }
    
    l_action_->setText(filter_->actionDescription());
}

// vim:ts=4:sw=4:tw=78
