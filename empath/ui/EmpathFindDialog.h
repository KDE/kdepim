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
# pragma interface "EmpathFindDialog.h"
#endif

#ifndef EMPATHFINDDIALOG_H
#define EMPATHFINDDIALOG_H

// Qt headers
#include <qwidget.h>
#include <qdialog.h>
#include <qfont.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>

#include <qstring.h>
#include <qstringlist.h>

// Local headers
#include "EmpathDefines.h"

class EmpathFindDialog : public QDialog {

    Q_OBJECT

public:
    
    EmpathFindDialog(QWidget * parent = 0, const char * name = 0);
    ~EmpathFindDialog();

    bool useRegExp()        const { return useRegExp_; }
    bool caseSensitive()    const { return caseSensitive_; }
    bool wrap()             const { return wrap_; }
    bool forwards()         const { return forwards_; }
    bool firstOnLine()      const { return firstOnLine_; }

    QString findText() const { return findTextCombo_->currentText(); }
    QString replaceText() const { return replaceTextCombo_->currentText(); }

    void setFindHistory(const QStringList & l) { *findHistory_ = l; }
    void setReplaceHistory(const QStringList & l) { *replaceHistory_ = l; }
    
    QStringList findHistory() const { return *findHistory_; }
    QStringList replaceHistory() const { return *replaceHistory_; }
    
protected slots:

    void findSelected();
    void replaceSelected();
    void replaceAllSelected();
    void replaceAndFindSelected();
    void helpSelected();
    void closeSelected();
    void regExpSelected();
    
signals:

    void find();
    void replace();
    void replaceAll();
    void replaceAndFind();
    
private:

    void _updateFindHistory(QString newItem);
    void _updateReplaceHistory(QString newItem);
    void _updateFields();
    
    QComboBox * findTextCombo_;
    QComboBox * replaceTextCombo_;

    QLabel * findLabel_;
    QLabel * replaceLabel_;

    QCheckBox * regExpCheckBox_;
    QCheckBox * firstOnLineCheckBox_;
    QCheckBox * ignoreCaseCheckBox_;
    QCheckBox * wrapCheckBox_;

    QRadioButton * directionForwardsRadio_;
    QRadioButton * directionBackwardsRadio_;
    
    QPushButton * findButton_;
    QPushButton * replaceButton_;
    QPushButton * replaceFindButton_;
    QPushButton * replaceAllButton_;
    
    static unsigned int historyMaxElements_;
    static QStringList * findHistory_;
    static QStringList * replaceHistory_;

    bool useRegExp_;
    bool caseSensitive_;
    bool wrap_;
    bool forwards_;
    bool firstOnLine_;
};

#endif

// vim:ts=4:sw=4:tw=78
