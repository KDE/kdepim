/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "newthemedialog.h"
#include "globalsettings_base.h"

#include <QLineEdit>
#include <KLocalizedString>
#include <KUrlRequester>
#include <KLineEdit>

#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>

using namespace GrantleeThemeEditor;
NewThemeDialog::NewThemeDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("New Theme"));

    QWidget *w = new QWidget;

    QVBoxLayout *lay = new QVBoxLayout;

    QLabel *lab = new QLabel(i18n("Theme name:"));
    lay->addWidget(lab);

    mThemeName = new QLineEdit;
    connect(mThemeName, &QLineEdit::textChanged, this, &NewThemeDialog::slotUpdateOkButton);
    lay->addWidget(mThemeName);

    lab = new QLabel(i18n("Theme directory:"));
    lay->addWidget(lab);

    mUrlRequester = new KUrlRequester;
    mUrlRequester->setMode(KFile::Directory | KFile::LocalOnly);
    connect(mUrlRequester->lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(slotUpdateOkButton()));
    lay->addWidget(mUrlRequester);

    w->setLayout(lay);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(w);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &NewThemeDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &NewThemeDialog::reject);
    mainLayout->addWidget(buttonBox);
    mOkButton->setDefault(true);
    mOkButton->setFocus();

    mOkButton->setEnabled(false);
    resize(300, 150);
    mThemeName->setFocus();
    readConfig();
}

NewThemeDialog::~NewThemeDialog()
{
}

void NewThemeDialog::readConfig()
{
    mUrlRequester->setUrl(QUrl(GrantleeThemeEditor::GrantleeThemeEditorSettings::path()));
}

QString NewThemeDialog::themeName() const
{
    return mThemeName->text();
}

QString NewThemeDialog::directory() const
{
    return mUrlRequester->lineEdit()->text();
}

void NewThemeDialog::slotUpdateOkButton()
{
    mOkButton->setEnabled(!mUrlRequester->lineEdit()->text().isEmpty() && !mThemeName->text().isEmpty());
}

