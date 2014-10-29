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

#include "texttospeechconfiggui.h"
#include "pimcommon/texttospeech/texttospeechconfigwidget.h"
#include "pimcommon/texttospeech/texttospeech.h"
#include <KLocalizedString>
#include <QApplication>
#include <KAboutData>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QCommandLineParser>
#include <QMainWindow>
#include <QDebug>

TextToSpeechConfigGui::TextToSpeechConfigGui(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    setLayout(hbox);
    PimCommon::TextToSpeechConfigWidget *widget = new PimCommon::TextToSpeechConfigWidget(this);
    hbox->addWidget(widget);
}

TextToSpeechConfigGui::~TextToSpeechConfigGui()
{

}

int main(int argc, char **argv)
{
    KAboutData aboutData(QLatin1String("texttospeechconfig_gui"), i18n("texttospeechconfig_Gui"), QLatin1String("1.0"));
    aboutData.setShortDescription(i18n("Test for config text to speech"));
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    TextToSpeechConfigGui *w = new TextToSpeechConfigGui;

    w->show();
    app.exec();
    delete w;
    return 0;
}
