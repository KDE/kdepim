/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef ERRORLOGDIALOG_H
#define ERRORLOGDIALOG_H

#include <KDialog>
#include <KListWidget>
#include <QListWidgetItem>
#include <KTextEdit>
#include <QStack>

#include <libkmobiletools/errorhandler.h>
#include <libkmobiletools/errortypes/baseerror.h>

/**
    @author Matthias Lechner <matthias@lmme.de>
*/
class ErrorLogItem;
class ErrorLogDialog : public KDialog
{
    Q_OBJECT
public:
    ErrorLogDialog( QWidget* parent = 0 );
    ~ErrorLogDialog();

public Q_SLOTS:
    void show();
    void showErrorDetails( QListWidgetItem* widgetItem );

private:
    void updateErrorStack();
    void updateErrorView();

    QStack<const KMobileTools::BaseError*> m_errorStack;

    QListWidget* m_errorLogListWidget;
    KTextEdit* m_errorDetailsTextEdit;
};

class ErrorLogItem : public QListWidgetItem
{
public:
    ErrorLogItem( const KMobileTools::BaseError* errorObject, QListWidget* parent );

    const KMobileTools::BaseError* errorObject() const;

private:
    const KMobileTools::BaseError* m_errorObject;
};

#endif
