/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#ifndef CONTENTTYPEWIDGET_H
#define CONTENTTYPEWIDGET_H

#include <QWidget>
#include "mailcommon_export.h"
class KComboBox;
namespace MailCommon
{
class MAILCOMMON_EXPORT ContentTypeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ContentTypeWidget(QWidget *parent = Q_NULLPTR);
    ~ContentTypeWidget();

    int currentIndex() const;
    void setCurrentIndex(int index);

    void setCurrentItem(const QString &name);
    QString currentText() const;

Q_SIGNALS:
    void activated(int index);

private:
    KComboBox *mContentsComboBox;
};
}
#endif // CONTENTTYPEWIDGET_H
