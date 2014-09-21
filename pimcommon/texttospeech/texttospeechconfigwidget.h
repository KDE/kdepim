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


#ifndef TEXTTOSPEECHCONFIGWIDGET_H
#define TEXTTOSPEECHCONFIGWIDGET_H
#include <QWidget>
#include "pimcommon_export.h"
class QSlider;
namespace PimCommon {
class PIMCOMMON_EXPORT TextToSpeechConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TextToSpeechConfigWidget(QWidget *parent=0);
    ~TextToSpeechConfigWidget();

Q_SIGNALS:
    void configChanged();

private:
    void readConfig();
    void writeConfig();

    QSlider *mVolume;
    QSlider *mRate;
    QSlider *mPitch;
};
}

#endif // TEXTTOSPEECHCONFIGWIDGET_H
