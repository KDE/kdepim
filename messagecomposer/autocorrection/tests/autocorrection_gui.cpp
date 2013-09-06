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

#include "autocorrection_gui.h"
#include "messagecomposer/autocorrection/composerautocorrection.h"
#include "messagecomposer/autocorrection/composerautocorrectionwidget.h"

#include <kdebug.h>
#include <kapplication.h>
#include <KCmdLineArgs>
#include <KLocale>

#include <QDebug>
#include <QPointer>
#include <QVBoxLayout>
#include <QPushButton>
#include <QKeyEvent>

TextEditAutoCorrectionWidget::TextEditAutoCorrectionWidget(MessageComposer::ComposerAutoCorrection *autoCorrection, QWidget *parent)
    : QTextEdit(parent),
      mAutoCorrection(autoCorrection)
{
}

TextEditAutoCorrectionWidget::~TextEditAutoCorrectionWidget()
{
}

void TextEditAutoCorrectionWidget::keyPressEvent ( QKeyEvent *e )
{
    if((e->key() == Qt::Key_Space) || (e->key() == Qt::Key_Enter) || (e->key() == Qt::Key_Return)) {
        if(mAutoCorrection) {
            //TODO customize rich text or not
            mAutoCorrection->autocorrect(false, *document(),textCursor().position());
        }
    }
    QTextEdit::keyPressEvent( e );
}


AutocorrectionTestWidget::AutocorrectionTestWidget(QWidget *parent)
    : QWidget(parent)
{
    mAutoCorrection = new MessageComposer::ComposerAutoCorrection;
    QVBoxLayout *lay = new QVBoxLayout;
    mEdit = new TextEditAutoCorrectionWidget(mAutoCorrection);
    lay->addWidget(mEdit);

    QPushButton *configButton = new QPushButton(QLatin1String("Configure..."));
    connect(configButton, SIGNAL(clicked()), SLOT(slotConfigure()));
    lay->addWidget(configButton);

    setLayout(lay);
}

AutocorrectionTestWidget::~AutocorrectionTestWidget()
{
    delete mAutoCorrection;
}

void AutocorrectionTestWidget::slotConfigure()
{
    //TODO
}

int main (int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "autocorrectiontest_gui", 0, ki18n("AutoCorrectionTest_Gui"),
                       "1.0", ki18n("Test for autocorrection widget"));
    KApplication app;

    AutocorrectionTestWidget *w = new AutocorrectionTestWidget();
    w->resize(800,600);

    w->show();
    app.exec();
    delete w;
    return 0;
}

