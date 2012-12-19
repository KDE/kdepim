/*
  Copyright (c) 2007 Till Adam <adam@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
*/

#ifndef MAILCOMMON_FILTERIMPORTEREXPORTER_P_H
#define MAILCOMMON_FILTERIMPORTEREXPORTER_P_H

#include <KDialog>

#include <QList>

class KPushButton;

class QListWidget;
class QWidget;

namespace MailCommon {

class MailFilter;

class FilterSelectionDialog : public KDialog
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
    QListWidget *filtersListWidget;
    QList<MailFilter*> originalFilters;
    KPushButton *selectAllButton;
    KPushButton *unselectAllButton;
};

}

#endif
