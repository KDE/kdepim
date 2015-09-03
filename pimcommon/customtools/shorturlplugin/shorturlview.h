/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef SHORTURLVIEW_H
#define SHORTURLVIEW_H

#include <customtools/customtoolsviewinterface.h>

#include <shorturlwidget.h>


namespace PimCommon
{
class ShorturlView : public PimCommon::CustomToolsViewInterface
{
    Q_OBJECT
public:
    explicit ShorturlView(QWidget *parent = Q_NULLPTR);
    ~ShorturlView();
    KToggleAction *action() const Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotActivateShorturl(bool state);

private:
    void createAction();
    KToggleAction *mAction;
    PimCommon::ShortUrlWidget *mShorturl;
};
}
#endif // SHORTURLVIEW_H
