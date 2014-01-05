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

#ifndef SIEVEWIDGETPAGEABSTRACT_H
#define SIEVEWIDGETPAGEABSTRACT_H

#include <QWidget>

namespace KSieveUi {
class SieveWidgetPageAbstract : public QWidget
{
    Q_OBJECT
public:
    enum PageType {
        BlockIf = 0,
        BlockElsIf = 1,
        BlockElse = 2,
        Include = 3,
        ForEveryPart = 4,
        GlobalVariable = 5
    };

    explicit SieveWidgetPageAbstract(QWidget *parent = 0);
    ~SieveWidgetPageAbstract();

    virtual void generatedScript(QString &script, QStringList &requires) = 0;

    virtual void setPageType(PageType type);
    PageType pageType() const;

private:
    PageType mType;
};
}

#endif // SIEVEWIDGETPAGEABSTRACT_H
