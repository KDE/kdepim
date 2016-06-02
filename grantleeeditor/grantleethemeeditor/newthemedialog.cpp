/*
   Copyright (C) 2013-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "newthemedialog.h"
#include "globalsettings_base.h"

#include <KLineEdit>
#include <KLocalizedString>
#include <KUrlRequester>

#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace GrantleeThemeEditor;
class GrantleeThemeEditor::NewThemeDialogPrivate
{
public:
    NewThemeDialogPrivate()
        : mThemeName(Q_NULLPTR),
          mUrlRequester(Q_NULLPTR),
          mOkButton(Q_NULLPTR)
    {

    }
    KLineEdit *mThemeName;
    KUrlRequester *mUrlRequester;
    QPushButton *mOkButton;
};
NewThemeDialog::NewThemeDialog(QWidget *parent)
    : QDialog(parent),
      d(new GrantleeThemeEditor::NewThemeDialogPrivate)
{
    setWindowTitle(i18n("New Theme"));

    QWidget *w = new QWidget;

    QVBoxLayout *lay = new QVBoxLayout;
    lay->setMargin(0);

    QLabel *lab = new QLabel(i18n("Theme name:"));
    lay->addWidget(lab);

    d->mThemeName = new KLineEdit;
    d->mThemeName->setClearButtonEnabled(true);
    d->mThemeName->setTrapReturnKey(true);
    connect(d->mThemeName, &KLineEdit::textChanged, this, &NewThemeDialog::slotUpdateOkButton);
    lay->addWidget(d->mThemeName);

    lab = new QLabel(i18n("Theme directory:"));
    lay->addWidget(lab);

    d->mUrlRequester = new KUrlRequester;
    d->mUrlRequester->setMode(KFile::Directory | KFile::LocalOnly);
    connect(d->mUrlRequester->lineEdit(), &KLineEdit::textChanged, this, &NewThemeDialog::slotUpdateOkButton);
    lay->addWidget(d->mUrlRequester);

    w->setLayout(lay);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(w);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    d->mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    d->mOkButton->setDefault(true);
    d->mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &NewThemeDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &NewThemeDialog::reject);
    mainLayout->addWidget(buttonBox);
    d->mOkButton->setDefault(true);
    d->mOkButton->setFocus();

    d->mOkButton->setEnabled(false);
    resize(300, 150);
    d->mThemeName->setFocus();
    readConfig();
}

NewThemeDialog::~NewThemeDialog()
{
    delete d;
}

void NewThemeDialog::readConfig()
{
    d->mUrlRequester->setUrl(QUrl::fromLocalFile(GrantleeThemeEditor::GrantleeThemeEditorSettings::path()));
}

QString NewThemeDialog::themeName() const
{
    return d->mThemeName->text();
}

QString NewThemeDialog::directory() const
{
    return d->mUrlRequester->lineEdit()->text();
}

void NewThemeDialog::slotUpdateOkButton()
{
    d->mOkButton->setEnabled(!d->mUrlRequester->lineEdit()->text().trimmed().isEmpty()
                             && !d->mThemeName->text().trimmed().isEmpty());
}

