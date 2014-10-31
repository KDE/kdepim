/*  -*- mode: C++; c-file-style: "gnu" -*-
 *
 *  This file is part of KMail, the KDE mail client.
 *
 *  Copyright (c) 2002-2003 Carsten Pfeiffer <pfeiffer@kde.org>
 *  Copyright (c) 2003      Zack Rusin <zack@kde.org>
 *
 *  KMail is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License, version 2, as
 *  published by the Free Software Foundation.
 *
 *  KMail is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */

#include "mailsourceviewer.h"
#include "mailsourceviewtextbrowserwidget.h"
#include "utils/util.h"
#include "findbar/findbarsourceview.h"
#include <kpimtextedit/htmlhighlighter.h>
#include "pimcommon/widgets/slidecontainer.h"
#include "pimcommon/util/pimutil.h"
#include <kiconloader.h>
#include <KLocalizedString>
#include <KStandardAction>
#include <kwindowsystem.h>
#include <QTabWidget>
#include <KMessageBox>
#include <QAction>
#include <QIcon>
#include <KIconTheme>
#include <KLocalizedString>

#include <QtCore/QRegExp>
#include <QApplication>
#include <QIcon>
#include <QShortcut>
#include <QVBoxLayout>
#include <QContextMenuEvent>
#include <QDebug>
#include <QMenu>
#include <QFontDatabase>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

namespace MessageViewer
{

MailSourceViewer::MailSourceViewer(QWidget *parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &MailSourceViewer::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &MailSourceViewer::reject);

    QVBoxLayout *layout = new QVBoxLayout(mainWidget);
    layout->setMargin(0);
    connect(buttonBox->button(QDialogButtonBox::Close), &QPushButton::clicked, this, &MailSourceViewer::close);

    mRawBrowser = new MailSourceViewTextBrowserWidget();

#ifndef NDEBUG
    mTabWidget = new QTabWidget;
    layout->addWidget(mTabWidget);

    mTabWidget->addTab(mRawBrowser, i18nc("Unchanged mail message", "Raw Source"));
    mTabWidget->setTabToolTip(0, i18n("Raw, unmodified mail as it is stored on the filesystem or on the server"));

    mHtmlBrowser = new MailSourceViewTextBrowserWidget();
    mTabWidget->addTab(mHtmlBrowser, i18nc("Mail message as shown, in HTML format", "HTML Source"));
    mTabWidget->setTabToolTip(1, i18n("HTML code for displaying the message to the user"));
    new KPIMTextEdit::HtmlHighlighter(mHtmlBrowser->textBrowser()->document());

    mTabWidget->setCurrentIndex(0);
#else
    layout->addWidget(mRawBrowser);
#endif

    // combining the shortcuts in one qkeysequence() did not work...
    QShortcut *shortcut = new QShortcut(this);
    shortcut->setKey(Qt::Key_Escape);
    connect(shortcut, &QShortcut::activated, this, &MailSourceViewer::close);
    shortcut = new QShortcut(this);
    shortcut->setKey(Qt::Key_W + Qt::CTRL);
    connect(shortcut, &QShortcut::activated, this, &MailSourceViewer::close);

    KWindowSystem::setIcons(winId(),
                            qApp->windowIcon().pixmap(IconSize(KIconLoader::Desktop),
                                    IconSize(KIconLoader::Desktop)),
                            qApp->windowIcon().pixmap(IconSize(KIconLoader::Small),
                                    IconSize(KIconLoader::Small)));
    new MailSourceHighlighter(mRawBrowser->textBrowser()->document());
    mRawBrowser->textBrowser()->setFocus();
    mainLayout->addWidget(buttonBox);
}

MailSourceViewer::~MailSourceViewer()
{
}

void MailSourceViewer::setRawSource(const QString &source)
{
    mRawBrowser->setText(source);
}

void MailSourceViewer::setDisplayedSource(const QString &source)
{
#ifndef NDEBUG
    mHtmlBrowser->setPlainText(HTMLPrettyFormatter::reformat(source));
#else
    Q_UNUSED(source);
#endif
}

void MailSourceViewer::setFixedFont()
{
    mRawBrowser->setFixedFont();
#ifndef NDEBUG
    mHtmlBrowser->setFixedFont();
#endif
}

}
