/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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


#ifndef TEXTTOSPEECHWIDGETTEST_H
#define TEXTTOSPEECHWIDGETTEST_H

#include <QObject>
namespace PimCommon {
class TextToSpeechWidget;
}
class TextToSpeechWidgetTest : public QObject
{
    Q_OBJECT
public:
    explicit TextToSpeechWidgetTest(QObject *parent=0);

private:
    void addInterface(PimCommon::TextToSpeechWidget *widget);

private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldChangeButtonEnableStateWhenChangeState();
    void shouldChangeStateWhenClickOnPlayPause();
    void shouldChangeStateWhenClickOnStop();
    void shouldEmitStateChanged();
};

#endif // TEXTTOSPEECHWIDGETTEST_H
