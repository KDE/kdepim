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

#ifndef SENDLATERTIMEDATEWIDGET_H
#define SENDLATERTIMEDATEWIDGET_H

#include <QWidget>
#include <QDateTime>
#include "sendlater_export.h"

class KTimeComboBox;
class KDateComboBox;

namespace SendLater {
class SENDLATER_EXPORT SendLaterTimeDateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SendLaterTimeDateWidget(QWidget *parent = 0);
    ~SendLaterTimeDateWidget();

    void setDateTime(const QDateTime &);
    QDateTime dateTime() const;

Q_SIGNALS:
    void dateTimeChanged(const QDateTime &);

private Q_SLOTS:
    void slotDateTimeChanged();

private:
    KTimeComboBox *mTimeComboBox;
    KDateComboBox *mDateComboBox;
};
}

#endif // SENDLATERTIMEDATEWIDGET_H
