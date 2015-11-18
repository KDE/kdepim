/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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
#include "autocorrection/autocorrection.h"
#include "autocorrection/autocorrectionwidget.h"
#include "autocorrection/widgets/lineeditwithautocorrection.h"
#include "settings/pimcommonsettings.h"

#include "pimcommon_debug.h"

#include <KLocalizedString>
#include <KSharedConfig>

#include <QPointer>
#include <QVBoxLayout>
#include <QPushButton>
#include <QKeyEvent>
#include <QToolBar>
#include <QAction>
#include <QApplication>
#include <KAboutData>
#include <QCommandLineParser>
#include <KConfigGroup>
#include <QDialogButtonBox>

ConfigureTestDialog::ConfigureTestDialog(PimCommon::AutoCorrection *autoCorrection, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Configure Autocorrection"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ConfigureTestDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ConfigureTestDialog::reject);

    buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

    mWidget = new PimCommon::AutoCorrectionWidget;
    mainLayout->addWidget(mWidget);
    mainLayout->addWidget(buttonBox);

    mWidget->setAutoCorrection(autoCorrection);
    mWidget->loadConfig();
    connect(okButton, &QPushButton::clicked, this, &ConfigureTestDialog::slotSaveSettings);
}

ConfigureTestDialog::~ConfigureTestDialog()
{
}

void ConfigureTestDialog::slotSaveSettings()
{
    mWidget->writeConfig();
}

TextEditAutoCorrectionWidget::TextEditAutoCorrectionWidget(PimCommon::AutoCorrection *autoCorrection, QWidget *parent)
    : QTextEdit(parent),
      mAutoCorrection(autoCorrection)
{
    setAcceptRichText(false);
}

TextEditAutoCorrectionWidget::~TextEditAutoCorrectionWidget()
{
}

void TextEditAutoCorrectionWidget::keyPressEvent(QKeyEvent *e)
{
    if ((e->key() == Qt::Key_Space) || (e->key() == Qt::Key_Enter) || (e->key() == Qt::Key_Return)) {
        if (mAutoCorrection && mAutoCorrection->isEnabledAutoCorrection()) {
            const QTextCharFormat initialTextFormat = textCursor().charFormat();
            int position = textCursor().position();
            mAutoCorrection->autocorrect(acceptRichText(), *document(), position);
            QTextCursor cur = textCursor();
            cur.setPosition(position);

            const QChar insertChar = (e->key() == Qt::Key_Space) ? QLatin1Char(' ') : QLatin1Char('\n');
            cur.insertText(insertChar, initialTextFormat);
            setTextCursor(cur);
            return;
        }
    }
    QTextEdit::keyPressEvent(e);
}

AutocorrectionTestWidget::AutocorrectionTestWidget(QWidget *parent)
    : QWidget(parent)
{
    mConfig = KSharedConfig::openConfig(QStringLiteral("autocorrectionguirc"));
    PimCommon::PimCommonSettings::self()->setSharedConfig(mConfig);
    PimCommon::PimCommonSettings::self()->load();

    mAutoCorrection = new PimCommon::AutoCorrection;
    QVBoxLayout *lay = new QVBoxLayout;
    QToolBar *bar = new QToolBar;
    lay->addWidget(bar);
    bar->addAction(QStringLiteral("Configure..."), this, SLOT(slotConfigure()));
    QAction *richText = new QAction(QStringLiteral("HTML mode"), this);
    richText->setCheckable(true);
    connect(richText, &QAction::toggled, this, &AutocorrectionTestWidget::slotChangeMode);
    bar->addAction(richText);

    mSubject = new PimCommon::LineEditWithAutoCorrection(this, QStringLiteral("autocorrectionguirc"));
    mSubject->setAutocorrection(mAutoCorrection);
    lay->addWidget(mSubject);

    mEdit = new TextEditAutoCorrectionWidget(mAutoCorrection);
    lay->addWidget(mEdit);

    setLayout(lay);
}

AutocorrectionTestWidget::~AutocorrectionTestWidget()
{
    mAutoCorrection->writeConfig();
    delete mAutoCorrection;
}

void AutocorrectionTestWidget::slotChangeMode(bool mode)
{
    mEdit->setAcceptRichText(mode);
}

void AutocorrectionTestWidget::slotConfigure()
{
    QPointer<ConfigureTestDialog> dlg = new ConfigureTestDialog(mAutoCorrection, this);
    if (dlg->exec()) {
        PimCommon::PimCommonSettings::self()->save();
    }
    delete dlg;
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QStandardPaths::setTestModeEnabled(true);
    KAboutData aboutData(QStringLiteral("autocorrectiontest_gui"), i18n("AutoCorrectionTest_Gui"), QStringLiteral("1.0"));
    aboutData.setShortDescription(i18n("Test for autocorrection widget"));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    AutocorrectionTestWidget *w = new AutocorrectionTestWidget();
    w->resize(800, 600);

    w->show();
    const int ret = app.exec();
    delete w;
    return ret;
}

