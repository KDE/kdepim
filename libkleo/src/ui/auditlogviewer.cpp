/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/
#include "auditlogviewer.h"

#include "PimCommon/RichTextEditor"
#include <QSaveFile>
#include <QFileDialog>
#include <KConfigGroup>
#include <KMessageBox>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QTextStream>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <KIconLoader>

using namespace Kleo::Private;

AuditLogViewer::AuditLogViewer(const QString &log, QWidget *parent)
    : QDialog(parent),
      m_log(/* sic */),
      m_textEdit(new PimCommon::RichTextEditorWidget(this))
{
    setWindowTitle(i18n("View GnuPG Audit Log"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    buttonBox->button(QDialogButtonBox::Close)->setIcon(KIconLoader::global()->loadIcon(
                QStringLiteral("dialog-close"),
                KIconLoader::Small));

    QPushButton *copyClipBtn = new QPushButton;
    copyClipBtn->setText(i18n("&Copy to Clipboard"));
    copyClipBtn->setIcon(KIconLoader::global()->loadIcon(QStringLiteral("edit-copy"),
                         KIconLoader::Small));
    buttonBox->addButton(copyClipBtn, QDialogButtonBox::ActionRole);
    connect(copyClipBtn, &QPushButton::clicked, this, &AuditLogViewer::slotCopyClip);

    QPushButton *saveAsBtn = new QPushButton;
    saveAsBtn->setText(i18n("&Save to Disk..."));
    saveAsBtn->setIcon(KIconLoader::global()->loadIcon(QStringLiteral("document-save-as"),
                       KIconLoader::Small));
    buttonBox->addButton(saveAsBtn, QDialogButtonBox::ActionRole);
    connect(saveAsBtn, &QPushButton::clicked, this, &AuditLogViewer::slotSaveAs);

    m_textEdit->setObjectName(QStringLiteral("m_textEdit"));
    m_textEdit->setReadOnly(true);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_textEdit);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setAuditLog(log);

    readConfig();
}

AuditLogViewer::~AuditLogViewer()
{
    writeConfig();
}

void AuditLogViewer::setAuditLog(const QString &log)
{
    if (log == m_log) {
        return;
    }
    m_log = log;
    m_textEdit->setHtml(QLatin1String("<qt>") + log + QLatin1String("</qt>"));
}

void AuditLogViewer::slotSaveAs()
{
    const QString fileName = QFileDialog::getSaveFileName(this, i18n("Choose File to Save GnuPG Audit Log to"));
    if (fileName.isEmpty()) {
        return;
    }

    QSaveFile file(fileName);

    if (file.open(QIODevice::WriteOnly)) {
        QTextStream s(&file);
        s << "<html><head>";
        if (!windowTitle().isEmpty()) {
            s << "\n<title>"
              << windowTitle().toHtmlEscaped()
              << "</title>\n";
        }
        s << "</head><body>\n"
          << m_log
          << "\n</body></html>" << endl;
        s.flush();
        file.commit();
    }

    if (const int err = file.error())
        KMessageBox::error(this, i18n("Could not save to file \"%1\": %2",
                                      file.fileName(), QString::fromLocal8Bit(strerror(err))),
                           i18n("File Save Error"));
}

void AuditLogViewer::slotCopyClip()
{
    m_textEdit->editor()->selectAll();
    m_textEdit->editor()->copy();
    m_textEdit->editor()->textCursor().clearSelection();
}

void AuditLogViewer::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "AuditLogViewer");
    const QSize size = group.readEntry("Size", QSize());
    if (size.isValid()) {
        resize(size);
    } else {
        resize(600, 400);
    }
}

void AuditLogViewer::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "AuditLogViewer");
    group.writeEntry("Size", size());
    group.sync();
}

