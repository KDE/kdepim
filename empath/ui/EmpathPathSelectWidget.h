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
# pragma interface "EmpathPathSelectWidget.h"
#endif

#ifndef EMPATHPATHSELECTWIDGET_H
#define EMPATHPATHSELECTWIDGET_H

// Qt includes
#include <qstring.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qhbox.h>

/**
 * @short Browsable file/dir selector
 * Quick hack to lay out a QLineEdit and a QPushButton to provide a megawidget
 * that lets the user select a dir or a file path.
 * @author Rikkus
 */
class EmpathPathSelectWidget : public QHBox
{
    Q_OBJECT
    
    public:
    
        EmpathPathSelectWidget(const QString & initialPath, QWidget * parent);

        virtual ~EmpathPathSelectWidget();

        virtual QString path() const;
        virtual void setPath(const QString &);
        
    protected slots:
    
        virtual void s_browse() = 0;

    signals:

        virtual void changed(const QString &);
    
    protected:

        QLineEdit     * le_path_;
        QPushButton   * pb_select_;
};

class EmpathFileSelectWidget : public EmpathPathSelectWidget
{
    Q_OBJECT
    
    public:
    
        EmpathFileSelectWidget(const QString & initialPath, QWidget * parent)
            : EmpathPathSelectWidget(initialPath, parent)
        {
            // Empty.
        }

    protected slots:
    
        virtual void s_browse();
};


class EmpathDirSelectWidget : public EmpathPathSelectWidget
{
    Q_OBJECT
    
    public:
    
        EmpathDirSelectWidget(const QString & initialPath, QWidget * parent)
            : EmpathPathSelectWidget(initialPath, parent)
        {
            // Empty.
        }

    protected slots:
    
        virtual void s_browse();
};

#endif

// vim:ts=4:sw=4:tw=78
