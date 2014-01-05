/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#ifndef SELECTDATEWIDGET_H
#define SELECTDATEWIDGET_H

#include <QWidget>

class KComboBox;
class KLineEdit;
class QStackedWidget;
class QSpinBox;
class KDateComboBox;
class KTimeComboBox;
namespace KSieveUi {
class SelectDateWidget : public QWidget
{
    Q_OBJECT
public:
    enum DateType {
        Year = 0,
        Month,
        Day,
        Date,
        Julian,
        Hour,
        Minute,
        Second,
        Time,
        Iso8601,
        Std11,
        Zone,
        Weekday
    };
    explicit SelectDateWidget(QWidget *parent = 0);
    ~SelectDateWidget();

    QString code() const;
    void setCode(const QString &type, const QString &value);

private Q_SLOTS:
    void slotDateTypeActivated(int);

private:
    SelectDateWidget::DateType dateTypeFromString(const QString &str);
    void initialize();
    QString dateType(DateType type) const;
    QString dateValue(DateType type) const;
    KComboBox *mDateType;
    KLineEdit *mDateLineEdit;
    QSpinBox *mDateValue;
    KDateComboBox *mDateEdit;
    KTimeComboBox *mTimeEdit;
    QStackedWidget *mStackWidget;
};
}

#endif // SELECTDATEWIDGET_H
