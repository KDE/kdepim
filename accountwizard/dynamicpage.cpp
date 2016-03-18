/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

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

#include "dynamicpage.h"

#include "accountwizard_debug.h"

#include <QUiLoader>
#include <QFile>
#include <qboxlayout.h>
#include <qscrollarea.h>
#include "global.h"
#include <KLocalizedTranslator>
#include <QCoreApplication>

DynamicPage::DynamicPage(const QString &uiFile, KAssistantDialog *parent) : Page(parent),
    m_dynamicWidget(Q_NULLPTR)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

    QWidget *pageParent = this;

    QUiLoader loader;
    loader.setLanguageChangeEnabled(true);
    QFile file(uiFile);
    if (file.open(QFile::ReadOnly)) {
        qCDebug(ACCOUNTWIZARD_LOG) << uiFile;
        m_dynamicWidget = loader.load(&file, pageParent);
        file.close();
        if (!Global::poFileName().isEmpty()) {
            KLocalizedTranslator *translator = new KLocalizedTranslator(this);
            QCoreApplication::instance()->installTranslator(translator);
            translator->setTranslationDomain(Global::poFileName());

            translator->addContextToMonitor(m_dynamicWidget->objectName());
            QEvent le(QEvent::LanguageChange);
            QCoreApplication::sendEvent(m_dynamicWidget, &le);
        }
    } else {
        qCDebug(ACCOUNTWIZARD_LOG) << "Unable to open: " << uiFile;
    }

    if (m_dynamicWidget) {
        layout->addWidget(m_dynamicWidget);
    }

    setValid(true);
}

QObject *DynamicPage::widget() const
{
    return m_dynamicWidget;
}

