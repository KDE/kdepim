/* Copyright (C) 2013 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "sieveeditorabstractwidget.h"
#include "pimcommon/util/pimutil.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <QFileDialog>

#include <QTextStream>
#include <QPointer>

#include <errno.h>

using namespace KSieveUi;
SieveEditorAbstractWidget::SieveEditorAbstractWidget(QWidget *parent)
    : QWidget(parent)
{
}

SieveEditorAbstractWidget::~SieveEditorAbstractWidget()
{

}

void SieveEditorAbstractWidget::saveAs(const QString &defaultName)
{
    const QString filter = i18n("*.siv|sieve files (*.siv)\n*|all files (*)");
    PimCommon::Util::saveTextAs(currentscript(), filter, this, defaultName, i18n("Save Script"));
}

QString SieveEditorAbstractWidget::currentscript()
{
    return QString();
}

void SieveEditorAbstractWidget::setImportScript(const QString &)
{

}

void SieveEditorAbstractWidget::slotImport()
{
    if (!currentscript().isEmpty()) {
        if (KMessageBox::warningYesNo(this, i18n("You will overwrite script. Do you want to continue?"), i18n("Import Script")) == KMessageBox::No) {
            return;
        }
    }
    const QString filter = i18n("*.siv|sieve files (*.siv)\n*|all files (*)");
    QPointer<QFileDialog> fdlg(new QFileDialog(this, i18n("Import Script Sieve"), QString(), filter));
    fdlg->setFileMode(QFileDialog::ExistingFile);
    if (fdlg->exec() == QDialog::Accepted && fdlg) {
        const QStringList fileNames = fdlg->selectedFiles();
        if (!loadFromFile(fileNames.at(0))) {
            KMessageBox::error(this,
                               i18n("Could not load the file %1:\n"
                                    "\"%2\" is the detailed error description.",
                                    fileNames.at(0),
                                    QString::fromLocal8Bit(strerror(errno))),
                               i18n("Sieve Editor Error"));
        }
    }
    delete fdlg;
}

bool SieveEditorAbstractWidget::loadFromFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);
    QString line = in.readLine();
    QString scriptText;
    while (!line.isNull()) {
        if (scriptText.isEmpty()) {
            scriptText = line;
        } else {
            scriptText += QLatin1Char('\n') + line;
        }
        line = in.readLine();
    }
    setImportScript(scriptText);
    return true;
}

