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

#ifndef SELECTFLAGSWIDGET_H
#define SELECTFLAGSWIDGET_H

#include <KDialog>
#include <QListWidget>
class KLineEdit;

namespace KSieveUi {

class SelectFlagsListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit SelectFlagsListWidget(QWidget *parent = 0);
    ~SelectFlagsListWidget();

    void setFlags(const QStringList& list);
    QStringList flags() const;

private:
    enum Type {
        FlagsRealName = Qt::UserRole+1
    };
    void init();
};


class SelectFlagsListDialog : public KDialog
{
    Q_OBJECT
public:
    explicit SelectFlagsListDialog(QWidget *parent = 0);
    ~SelectFlagsListDialog();

    void setFlags(const QStringList& list);
    QStringList flags() const;

private:
    SelectFlagsListWidget *mListWidget;
};



class SelectFlagsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SelectFlagsWidget(QWidget *parent = 0);
    ~SelectFlagsWidget();

    QString code() const;
    void setFlags(const QStringList &flags);

private Q_SLOTS:
    void slotSelectFlags();

private:
    KLineEdit *mEdit;
};
}

#endif // SELECTFLAGSWIDGET_H
