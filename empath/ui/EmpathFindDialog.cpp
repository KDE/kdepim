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

QStrList EmpathFindDialog::_findHistory;
QStrList EmpathFindDialog::_replaceHistory;

EmpathFindDialog::EmpathFindDialog(QWidget * parent, const char * name)
    : QDialog(parent, name, true)
{
    empathDebug("ctor");

    directionGroup = new QButtonGroup(i18n("Direction"), this, "directionGroup");
    directionGroup->setGeometry(180, 100, 180, 140);
    
    findTextCombo = new QComboBox(true, this, "findTextCombo");
    findTextCombo->setGeometry(130, 20, 230, 30);
    findTextCombo->setInsertionPolicy(QComboBox::AtBottom);
    findTextCombo->setAutoCompletion(true);
    findTextCombo->setSizeLimit(10);
    findTextCombo->setFocus();
    
    replaceTextCombo = new QComboBox(true, this, "replaceTextCombo");
    replaceTextCombo->setGeometry(130, 60, 230, 30);
    replaceTextCombo->setInsertionPolicy(QComboBox::AtBottom);
    replaceTextCombo->setAutoCompletion(true);
    replaceTextCombo->setSizeLimit(10);

    findLabel = new QLabel(this, "findLabel");
    findLabel->setGeometry(10, 20, 110, 30);
    findLabel->setText(i18n("Find"));
    
    replaceLabel = new QLabel(this, "replaceLabel");
    replaceLabel->setGeometry(10, 60, 110, 30);
    replaceLabel->setText(i18n("Replace with"));
    
    regExpCheckBox = new QCheckBox(this, "regExpCheckBox");
    regExpCheckBox->setGeometry(10, 100, 160, 30);
    regExpCheckBox->setText(i18n("Regular E&xpression"));
    QObject::connect(regExpCheckBox, SIGNAL(clicked()),
            this, SLOT(regExpSelected()));    

    firstOnLineCheckBox = new QCheckBox(this, "firstOnLineCheckBox");
    firstOnLineCheckBox->setGeometry(10, 140, 160, 30);
    firstOnLineCheckBox->setText(i18n("First on line &only"));
    
    ignoreCaseCheckBox = new QCheckBox(this, "ignoreCaseCheckBox");
    ignoreCaseCheckBox->setGeometry(10, 180, 160, 30);
    ignoreCaseCheckBox->setText(i18n("&Ignore case"));
    
    wrapCheckBox = new QCheckBox(this, "wrapCheckBox");
    wrapCheckBox->setGeometry(10, 220, 160, 30);
    wrapCheckBox->setText(i18n("&Wrap search"));
    
    directionForwardsRadio = new QRadioButton(this, "directionForwardsRadio");
    directionForwardsRadio->setGeometry(200, 140, 100, 20);
    directionForwardsRadio->setText(i18n("&Forwards"));
    directionForwardsRadio->setChecked(true);
    
    directionBackwardsRadio = new QRadioButton(this, "directionBackwardsRadio");
    directionBackwardsRadio->setGeometry(200, 180, 100, 20);
    directionBackwardsRadio->setText(i18n("&Backwards"));

    findButton = new QPushButton(this, "findButton");
    findButton->setGeometry(370, 20, 140, 30);
    findButton->setText(i18n("&Find"));
    findButton->setAutoDefault(true);
    findButton->setDefault(true);
    
    replaceButton = new QPushButton(this, "replaceButton");
    replaceButton->setGeometry(370, 60, 140, 30);
    replaceButton->setText(i18n("&Replace"));
    
    replaceFindButton = new QPushButton(this, "replaceFindButton");
    replaceFindButton->setGeometry(370, 100, 140, 30);
    replaceFindButton->setText(i18n("Replace &+ Find"));
    
    replaceAllButton = new QPushButton(this, "replaceAllButton");
    replaceAllButton->setGeometry(370, 140, 140, 30);
    replaceAllButton->setText(i18n("Replace &All"));
    
    helpButton = new QPushButton(this, "helpButton");
    helpButton->setGeometry(370, 180, 140, 30);
    helpButton->setText(i18n("&Help"));
    
    closeButton = new QPushButton(this, "closeButton");
    closeButton->setGeometry(370, 220, 140, 30);
    closeButton->setText(i18n("&Close"));

    directionGroup->insert(directionForwardsRadio);
    directionGroup->insert(directionBackwardsRadio);
    
    QStrListIterator it(_findHistory);
    for (; it.current() ; ++it)
        findTextCombo->insertItem(it.current());
    
    QStrListIterator it2(_replaceHistory);
    for (; it2.current() ; ++it2)
        replaceTextCombo->insertItem(it2.current());
    
    QObject::connect(findButton, SIGNAL(clicked()),
            this, SLOT(findSelected()));
    
    QObject::connect(replaceButton, SIGNAL(clicked()),
            this, SLOT(replaceSelected()));
    
    QObject::connect(replaceAllButton, SIGNAL(clicked()),
            this, SLOT(replaceAllSelected()));
    
    QObject::connect(replaceFindButton, SIGNAL(clicked()),
            this, SLOT(replaceAndFindSelected()));
    
    QObject::connect(helpButton, SIGNAL(clicked()),
            this, SLOT(helpSelected()));
    
    QObject::connect(closeButton, SIGNAL(clicked()),
            this, SLOT(closeSelected()));
    
    resize(520,260);
    setMinimumSize(520,260);
    setMaximumSize(520,260);
}

EmpathFindDialog::~EmpathFindDialog()
{
    empathDebug("dtor");
}
    void
EmpathFindDialog::findSelected()
{
    empathDebug("findSelected called");
    updateFindHistory(findTextCombo->currentText());
    updateFields();
    emit(find());
}

    void
EmpathFindDialog::replaceSelected()
{
    empathDebug("findSelected called");
    updateFindHistory(findTextCombo->currentText());
    updateReplaceHistory(replaceTextCombo->currentText());
    updateFields();
    emit(replace());
}

    void
EmpathFindDialog::replaceAllSelected()
{
    empathDebug("replaceAllSelected called");
    updateFindHistory(findTextCombo->currentText());
    updateReplaceHistory(replaceTextCombo->currentText());
    updateFields();
    emit(replaceAll());
}

    void
EmpathFindDialog::replaceAndFindSelected()
{
    empathDebug("replaceAndFindSelected called");
    updateFindHistory(findTextCombo->currentText());
    updateReplaceHistory(replaceTextCombo->currentText());
    updateFields();
    emit(replaceAndFind());
}

    void
EmpathFindDialog::helpSelected()
{
    empathDebug("helpSelected called");
    emit(help());
}

    void
EmpathFindDialog::closeSelected()
{
    empathDebug("closeSelected called");
    updateFields();
    done(0);
}

    void
EmpathFindDialog::regExpSelected()
{
    empathDebug("regExpSelected called");
    if (regExpCheckBox->isChecked()) {
        replaceButton->setEnabled(false);
        replaceFindButton->setEnabled(false);
        replaceAllButton->setEnabled(false);
        firstOnLineCheckBox->setEnabled(false);
        ignoreCaseCheckBox->setEnabled(false);
        wrapCheckBox->setEnabled(false);
        directionForwardsRadio->setEnabled(false);
        directionBackwardsRadio->setEnabled(false);
        replaceLabel->setEnabled(false);
        replaceTextCombo->setEnabled(false);
        findButton->setText(i18n("&Run"));
        findLabel->setText(i18n("Expression"));
    } else {
        replaceButton->setEnabled(true);
        replaceFindButton->setEnabled(true);
        replaceAllButton->setEnabled(true);
        firstOnLineCheckBox->setEnabled(true);
        ignoreCaseCheckBox->setEnabled(true);
        wrapCheckBox->setEnabled(true);
        directionForwardsRadio->setEnabled(true);
        directionBackwardsRadio->setEnabled(true);
        replaceLabel->setEnabled(true);
        replaceTextCombo->setEnabled(true);
        findButton->setText(i18n("&Find"));
        findLabel->setText(i18n("Find"));
    }
}

    void
EmpathFindDialog::setFindHistory(const QStrList & history)
{
    empathDebug("setFindHistory called");
    _findHistory.clear();
    
    QStrListIterator it(history);
    for (; it.current() ; ++it)
        _findHistory.append(it.current());
}
    
    void
EmpathFindDialog::setReplaceHistory(const QStrList & history)
{
    empathDebug("setReplaceHistory called");
    _replaceHistory.clear();
    
    QStrListIterator it(history);
    for (; it.current() ; ++it)
        _replaceHistory.append(it.current());
}
    
    const QStrList &
EmpathFindDialog::findHistory()
{
    empathDebug("findHistory called");
    return _findHistory;
}

    const QStrList &
EmpathFindDialog::replaceHistory()
{
    empathDebug("replaceHistory called");
    return _replaceHistory;
}

    QString
EmpathFindDialog::findText()
{
    empathDebug("findText called");
    return QString(findTextCombo->currentText());
}
    
    QString
EmpathFindDialog::replaceText()
{
    empathDebug("replaceText called");
    return QString(replaceTextCombo->currentText());
}

    void
EmpathFindDialog::updateFindHistory(QString newItem)
{
    empathDebug("updateFindHistory called");
    if (newItem.length() == 0) return;
    
    QString item(newItem); // shallow - strlist will make deep
    
    // Remove any duplicate first, so it gets 'moved' to top
    for (Q_UINT32 i = 0 ; i < _findHistory.count() ; i++)
        if (strcmp(_findHistory.at(i), item) == 0)
                _findHistory.remove(i);
    
    // Add the new item to the end of the internal list
    _findHistory.append(item);

    // If we filled the history, drop the last element (it's first :)
    if (_findHistory.count() >= historyMaxElements) {
        _findHistory.removeFirst();
    }
    
    // Clear out the combo box. Easier to update this way.
    findTextCombo->clear();
    
    // Fill the combo box with the updated list
    QStrListIterator it(_findHistory);
    it.toLast();
    for (; it.current() ; --it)
        findTextCombo->insertItem(it.current());
}

    void
EmpathFindDialog::updateReplaceHistory(QString newItem)
{
    empathDebug("updateReplaceHistory called");
    // Don't add empty strings
    if (newItem.length() == 0) return;
    
    QString item(newItem);

    // Remove any duplicate first, so it gets 'moved' to top
    for (Q_UINT32 i = 0 ; i < _replaceHistory.count() ; i++)
        if (strcmp(_replaceHistory.at(i), item) == 0)
                _replaceHistory.remove(i);
    
    // Add the new item to the end of the internal list
    _replaceHistory.append(item);

    // If we filled the history, drop the last element (it's first :)
    if (_replaceHistory.count() >= historyMaxElements) {
        _replaceHistory.removeFirst();
    }

    // Clear out the combo box. Easier to update this way.
    replaceTextCombo->clear();

    // Fill the combo box with the updated list
    QStrListIterator it(_replaceHistory);
    it.toLast();
    for (; it.current() ; --it)
        replaceTextCombo->insertItem(it.current());
}

    void
EmpathFindDialog::updateFields()
{
    _useRegExp        = regExpCheckBox->isChecked();
    _caseSensitive    = ignoreCaseCheckBox->isChecked();
    _wrap            = wrapCheckBox->isChecked();
    _forwards        = directionForwardsRadio->isChecked();
    _firstOnLine    = firstOnLineCheckBox->isChecked();
}
// vim:ts=4:sw=4:tw=78
