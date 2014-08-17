/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef MAILCOMMON_FILTERSELECTIONDIALOG_H
#define MAILCOMMON_FILTERSELECTIONDIALOG_H


#include <QDialog>

#include <QList>
#include <KConfigGroup>

class QPushButton;

class QListWidget;
class QWidget;

namespace MailCommon {

class MailFilter;

class FilterSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilterSelectionDialog( QWidget * parent = 0 );
    virtual ~FilterSelectionDialog();
    void setFilters( const QList<MailFilter*> &filters );
    QList<MailFilter*> selectedFilters() const;

public Q_SLOTS:
    void slotUnselectAllButton();
    void slotSelectAllButton();
    void reject();

private:
    void writeConfig();
    void readConfig();
    QListWidget *filtersListWidget;
    QList<MailFilter*> originalFilters;
    QPushButton *selectAllButton;
    QPushButton *unselectAllButton;
    QPushButton *mOkButton;
};

}
#endif // FILTERSELECTIONDIALOG_H
