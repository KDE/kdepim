/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#ifndef SIEVEFOREVERYPARTWIDGET_H
#define SIEVEFOREVERYPARTWIDGET_H

#include "sievewidgetpageabstract.h"
class QCheckBox;
class KLineEdit;
class QDomElement;

namespace KSieveUi {
class SieveForEveryPartWidget : public SieveWidgetPageAbstract
{
    Q_OBJECT
public:
    explicit SieveForEveryPartWidget(QWidget *parent = 0);
    ~SieveForEveryPartWidget();

    void generatedScript(QString &script, QStringList &requires);
    void loadScript(const QDomElement &element, QString &error);

private Q_SLOTS:
    void slotHelp();

private:
    QCheckBox *mForLoop;
    KLineEdit *mName;
};
}


#endif // SIEVEFOREVERYPARTWIDGET_H
