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
# pragma interface "EmpathFindDialog.h"
#endif

#ifndef EMPATHFINDDIALOG_H
#define EMPATHFINDDIALOG_H

// Qt headers
#include <qwidget.h>
#include <qdialog.h>
#include <qfont.h>
#include <qstring.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qstrlist.h>

// Local headers
#include "EmpathDefines.h"

const Q_UINT32 historyMaxElements = 32;

class EmpathFindDialog : public QDialog {

    Q_OBJECT

public:
    
    EmpathFindDialog(QWidget * parent = 0, const char * name = 0);
    ~EmpathFindDialog();

    bool useRegExp()        { return _useRegExp; }
    bool caseSensitive()    { return _caseSensitive; }
    bool wrap()                { return _wrap; }
    bool forwards()            { return _forwards; }
    bool firstOnLine()        { return _firstOnLine; }

    QString findText();
    QString replaceText();

    void setFindHistory(const QStrList & history);
    void setReplaceHistory(const QStrList & history);
    
    const QStrList & findHistory();
    const QStrList & replaceHistory();
    
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
    void help();
    
private:

    QComboBox        * findTextCombo;
    QComboBox        * replaceTextCombo;

    QLabel            * findLabel;
    QLabel            * replaceLabel;

    QCheckBox        * regExpCheckBox;
    QCheckBox        * firstOnLineCheckBox;
    QCheckBox        * ignoreCaseCheckBox;
    QCheckBox        * wrapCheckBox;

    QButtonGroup    * directionGroup;
    QRadioButton    * directionForwardsRadio;
    QRadioButton    * directionBackwardsRadio;
    
    QPushButton        * findButton;
    QPushButton        * replaceButton;
    QPushButton        * replaceFindButton;
    QPushButton        * replaceAllButton;
    QPushButton        * helpButton;
    QPushButton        * closeButton;
    
    bool            _useRegExp;
    bool            _caseSensitive;
    bool            _wrap;
    bool            _forwards;
    bool            _firstOnLine;
    
    static QStrList        _findHistory;
    static QStrList        _replaceHistory;

    void            updateFindHistory(QString newItem);
    void            updateReplaceHistory(QString newItem);
    void            updateFields();
};

#endif

// vim:ts=4:sw=4:tw=78
