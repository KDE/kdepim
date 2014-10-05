/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#include "htmleditor.h"

#include <kglobal.h>
#include <ktexteditor/editor.h>
//#include <ktexteditor/editorchooser.h>
#include <ktexteditor/view.h>
#include <ktexteditor/configpage.h>
#include <ktexteditor/configinterface.h>
#include <QAction>
#include <klocalizedstring.h>

#include <QDebug>
#include <QMenu>

class HtmlEditorPrivate
{
public:
    HtmlEditor instance;
};

Q_GLOBAL_STATIC(HtmlEditorPrivate, instancePrivate)

HtmlEditor *HtmlEditor::self()
{
    return &instancePrivate->instance;
}

HtmlEditor::HtmlEditor() : QObject()
{
    mEditor = KTextEditor::Editor::instance();
}

HtmlEditor::~HtmlEditor()
{
    qDebug();
    if (!instancePrivate.isDestroyed()) {
        qDebug() << "editor deleted";
    }
}

KTextEditor::View *HtmlEditor::createView(QWidget *parent)
{
    KTextEditor::Document *document = mEditor->createDocument(parent);
    bool result = document->setHighlightingMode(QLatin1String("html"));
    if (result) {
        qDebug() << "Syntax highlighting enabled";
    }
    KTextEditor::View *view = document->createView(parent);
    QMenu *menu = view->defaultContextMenu();

    KTextEditor::ConfigInterface *interface = qobject_cast< KTextEditor::ConfigInterface * >(view);

    if (interface) {
        QAction *actWordWrap = new QAction(i18n("Dynamic Word Wrap"), view);
        actWordWrap->setCheckable(true);
        connect(actWordWrap, &QAction::triggered, this, &HtmlEditor::toggleWordWrap);

        QAction *actLineNumber = new QAction(i18n("Show line numbers"), view);
        actLineNumber->setCheckable(true);
        connect(actLineNumber, &QAction::triggered, this, &HtmlEditor::toggleLineNumber);

        QMenu *options = new QMenu(i18n("Options"), qobject_cast< QWidget * >(view));
        options->addAction(actWordWrap);
        options->addAction(actLineNumber);

        menu->addSeparator();
        menu->addMenu(options);

        interface->setConfigValue(QLatin1String("dynamic-word-wrap"), true);
        actWordWrap->setChecked(true);
    }
    view->setContextMenu(menu);
    return view;
}

QWidget *HtmlEditor::configPage(int number, QWidget *parent)
{
    KTextEditor::ConfigPage *page = mEditor->configPage(number, parent);
    if (!page) {
        return NULL;
    } else {
        return page;
    }
}

void HtmlEditor::toggleWordWrap()
{
    KTextEditor::View *view = qobject_cast< KTextEditor::View * >(sender()->parent());
    KTextEditor::ConfigInterface *interface = qobject_cast< KTextEditor::ConfigInterface * >(view);
    const bool result = interface->configValue(QLatin1String("dynamic-word-wrap")).toBool();
    interface->setConfigValue(QLatin1String("dynamic-word-wrap"), !(result));
}

void HtmlEditor::toggleLineNumber()
{
    KTextEditor::View *view = qobject_cast< KTextEditor::View * >(sender()->parent());
    KTextEditor::ConfigInterface *interface = qobject_cast< KTextEditor::ConfigInterface * >(view);
    const bool result = interface->configValue(QLatin1String("line-numbers")).toBool();
    interface->setConfigValue(QLatin1String("line-numbers"), !(result));
}

