/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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
# pragma implementation "EmpathFindDialog.h"
#endif

// KDE includes
#include <klocale.h>

// Local headers
#include "EmpathFindDialog.h"

unsigned int EmpathFindDialog::historyMaxElements_  = 32;
QStringList * EmpathFindDialog::findHistory_        = 0L;
QStringList * EmpathFindDialog::replaceHistory_     = 0L;

EmpathFindDialog::EmpathFindDialog(QWidget * parent, const char * name)
    : QDialog(parent, name, true)
{
    if (0 == findHistory_)
        findHistory_ = new QStringList;

    if (0 == replaceHistory_)
        replaceHistory_ = new QStringList;

    findTextCombo_ = new QComboBox(true, this, "findTextCombo");
    findTextCombo_->setInsertionPolicy(QComboBox::AtBottom);
    findTextCombo_->setAutoCompletion(true);
    findTextCombo_->setSizeLimit(10);
    findTextCombo_->setFocus();
    
    replaceTextCombo_ = new QComboBox(true, this, "replaceTextCombo");
    replaceTextCombo_->setInsertionPolicy(QComboBox::AtBottom);
    replaceTextCombo_->setAutoCompletion(true);
    replaceTextCombo_->setSizeLimit(10);

    QLabel * findLabel = new QLabel(i18n("Find"), this, "findLabel");
    
    QLabel * replaceLabel =
        new QLabel(i18n("Replace with"), this, "replaceLabel");
    
    regExpCheckBox_ =
        new QCheckBox(i18n("Regular E&xpression"), this, "regExpCheckBox");

    firstOnLineCheckBox_ =
        new QCheckBox(i18n("First on line &only"), this, "firstOnLineCheckBox");
    
    ignoreCaseCheckBox_ =
        new QCheckBox(i18n("&Ignore case"), this, "ignoreCaseCheckBox");
    
    wrapCheckBox_ = new QCheckBox(i18n("&Wrap search"), this, "wrapCheckBox");
    
    QButtonGroup * directionGroup =
        new QButtonGroup(i18n("Direction"), this, "directionGroup");

    directionForwardsRadio_ =
        new QRadioButton(i18n("&Forwards"), this, "directionForwardsRadio");

    directionForwardsRadio_->setChecked(true);
    
    directionBackwardsRadio_ =
        new QRadioButton(i18n("Backwards"), this, "directionBackwardsRadio");
    
    directionGroup->insert(directionForwardsRadio_);
    directionGroup->insert(directionBackwardsRadio_);

    findButton_ = new QPushButton(i18n("&Find"), this, "findButton");
    findButton_->setAutoDefault(true);
    findButton_->setDefault(true);
    
    replaceButton_ =
        new QPushButton(i18n("&Replace"), this, "replaceButton");
    
    replaceFindButton_ =
        new QPushButton(i18n("Replace &+ find"), this, "replaceFindButton");
    
    replaceAllButton_ =
        new QPushButton(i18n("Replace &All"), this, "replaceAllButton");
    
    QPushButton * helpButton =
        new QPushButton(i18n("&Help"), this, "helpButton");
    
    QPushButton * closeButton =
        new QPushButton(i18n("&Close"), this, "closeButton");

    findTextCombo_->insertStringList(*findHistory_);
    replaceTextCombo_->insertStringList(*replaceHistory_);
    
    QObject::connect(
        regExpCheckBox_,    SIGNAL(clicked()),
        this,               SLOT(regExpSelected()));    

    QObject::connect(
        findButton_,    SIGNAL(clicked()),
        this,           SLOT(findSelected()));
    
    QObject::connect(
        replaceButton_, SIGNAL(clicked()),
        this,           SLOT(replaceSelected()));
    
    QObject::connect(
        replaceAllButton_,  SIGNAL(clicked()),
        this,               SLOT(replaceAllSelected()));
    
    QObject::connect(
        replaceFindButton_, SIGNAL(clicked()),
        this,               SLOT(replaceAndFindSelected()));
    
    QObject::connect(
        helpButton,     SIGNAL(clicked()),
        this,           SLOT(helpSelected()));
    
    QObject::connect(
        closeButton,    SIGNAL(clicked()),
        this,           SLOT(closeSelected()));
}

EmpathFindDialog::~EmpathFindDialog()
{
    // Empty.
}
    void
EmpathFindDialog::findSelected()
{
    _updateFindHistory(findTextCombo_->currentText());
    _updateFields();
    emit(find());
}

    void
EmpathFindDialog::replaceSelected()
{
    _updateFindHistory(findTextCombo_->currentText());
    _updateReplaceHistory(replaceTextCombo_->currentText());
    _updateFields();
    emit(replace());
}

    void
EmpathFindDialog::replaceAllSelected()
{
    _updateFindHistory(findTextCombo_->currentText());
    _updateReplaceHistory(replaceTextCombo_->currentText());
    _updateFields();
    emit(replaceAll());
}

    void
EmpathFindDialog::replaceAndFindSelected()
{
    _updateFindHistory(findTextCombo_->currentText());
    _updateReplaceHistory(replaceTextCombo_->currentText());
    _updateFields();
    emit(replaceAndFind());
}

    void
EmpathFindDialog::helpSelected()
{
}

    void
EmpathFindDialog::closeSelected()
{
    _updateFields();
    accept();
}

    void
EmpathFindDialog::regExpSelected()
{
    bool enable = !regExpCheckBox_->isChecked();

    replaceButton_              ->setEnabled(enable);
    replaceFindButton_          ->setEnabled(enable);
    replaceAllButton_           ->setEnabled(enable);
    firstOnLineCheckBox_        ->setEnabled(enable);
    ignoreCaseCheckBox_         ->setEnabled(enable);
    wrapCheckBox_               ->setEnabled(enable);
    directionForwardsRadio_     ->setEnabled(enable);
    directionBackwardsRadio_    ->setEnabled(enable);
    replaceLabel_               ->setEnabled(enable);
    replaceTextCombo_           ->setEnabled(enable);

    findButton_ ->setText(enable ? i18n("&Find") : i18n("&Run"));
    findLabel_  ->setText(enable ? i18n("Expression") : i18n("Find"));
}

    void
EmpathFindDialog::_updateFindHistory(QString item)
{
    if (item.isEmpty())
        return;
    
    // Remove any duplicate first, so it gets 'moved' to top
    
    QStringList::Iterator it;

    for (it = findHistory_->begin(); it != findHistory_->end(); ++it)
        if (*it == item)
            findHistory_->remove(it);
    
    // Add the new item to the end of the internal list
    *findHistory_ << item;

    // If we filled the history, drop the last element (it's first :)
    if (findHistory_->count() >= historyMaxElements_)
        findHistory_->remove(findHistory_->begin());
    
    // Clear out the combo box. Easier to update this way.
    findTextCombo_->clear();
    
    // Fill the combo box with the updated list
    QStringList::ConstIterator it2;

    for (it2 = findHistory_->end(); it2 != findHistory_->begin() ; --it2)
        findTextCombo_->insertItem(*it2);
}

    void
EmpathFindDialog::_updateReplaceHistory(QString item)
{
    // Don't add empty strings
    if (item.isEmpty())
        return;

    // Remove any duplicate first, so it gets 'moved' to top
    QStringList::Iterator it;

    for (it = replaceHistory_->begin(); it != replaceHistory_->end(); ++it)
        if (*it == item)
            replaceHistory_->remove(it);
    
    // Add the new item to the end of the internal list
    *replaceHistory_ << item;

    // If we filled the history, drop the last element (it's first :)
    if (replaceHistory_->count() >= historyMaxElements_)
        replaceHistory_->remove(replaceHistory_->begin());

    // Clear out the combo box. Easier to update this way.
    replaceTextCombo_->clear();

    // Fill the combo box with the updated list
    QStringList::ConstIterator it2;

    for (it2 = replaceHistory_->end(); it2 != replaceHistory_->begin() ; --it2)
        replaceTextCombo_->insertItem(*it2);
}

    void
EmpathFindDialog::_updateFields()
{
    useRegExp_      = regExpCheckBox_           ->isChecked();
    caseSensitive_  = ignoreCaseCheckBox_       ->isChecked();
    wrap_           = wrapCheckBox_             ->isChecked();
    forwards_       = directionForwardsRadio_   ->isChecked();
    firstOnLine_    = firstOnLineCheckBox_      ->isChecked();
}

// vim:ts=4:sw=4:tw=78
