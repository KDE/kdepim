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

#ifndef TRANSLATORVIEW_H
#define TRANSLATORVIEW_H

#include <customtools/customtoolsviewinterface.h>
class KActionCollection;
namespace PimCommon
{
class TranslatorWidget;
class TranslatorView : public PimCommon::CustomToolsViewInterface
{
    Q_OBJECT
public:
    explicit TranslatorView(KActionCollection *ac, QWidget *parent);
    ~TranslatorView();
    KToggleAction *action() const Q_DECL_OVERRIDE;

    void setText(const QString &text) Q_DECL_OVERRIDE;
private Q_SLOTS:
    void slotActivateTranslator(bool state);

private:
    void createAction(KActionCollection *ac);
    KToggleAction *mAction;
    PimCommon::TranslatorWidget *mTranslatorWidget;

};
}
#endif // TRANSLATORVIEW_H
