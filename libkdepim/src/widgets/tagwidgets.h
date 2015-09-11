/*
  Copyright (c) 2014 Christian Mollekopf <mollekopf@kolabsys.com>

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

#ifndef KDEPIM_TAGWIDGETS_H
#define KDEPIM_TAGWIDGETS_H

#include "kdepim_export.h"
#include <Tag>
#include <TagSelectionDialog>
#include <QWidget>

class KJob;
namespace Akonadi
{
class TagWidget;
}

namespace KPIM
{
class TagWidgetPrivate;
class KDEPIM_EXPORT TagWidget: public QWidget
{
    Q_OBJECT
public:
    explicit TagWidget(QWidget *parent = Q_NULLPTR);
    ~TagWidget();
    void setSelection(const QStringList &);
    QStringList selection() const;
Q_SIGNALS:
    void selectionChanged(const QStringList &);
    void selectionChanged(const Akonadi::Tag::List &);

private Q_SLOTS:
    void onTagCreated(KJob *);
    void onSelectionChanged(const Akonadi::Tag::List &);

private:
    TagWidgetPrivate *const d;
};

class TagSelectionDialogPrivate;
class KDEPIM_EXPORT TagSelectionDialog : public Akonadi::TagSelectionDialog
{
    Q_OBJECT
public:
    explicit TagSelectionDialog(QWidget *parent = Q_NULLPTR);
    ~TagSelectionDialog();
    void setSelection(const QStringList &);
    QStringList selection() const;
    Akonadi::Tag::List tagSelection() const;

private Q_SLOTS:
    void onTagCreated(KJob *);

private:
    TagSelectionDialogPrivate *const d;
};

}

#endif
