/***************************************************************************
 *   This file is part of the Bilbo Blogger.                               *
 *   Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>     *
 *   Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <QMenu>

#include "htmleditor.h"

#include <kglobal.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/view.h>
#include <ktexteditor/configpage.h>
#include <ktexteditor/configinterface.h>
#include <kaction.h>
#include <klocalizedstring.h>


class HtmlEditorPrivate
{
public:
//     HtmlEditorPrivate() : instance( 0L ) {}
//     ~HtmlEditorPrivate() { delete instance; }
    HtmlEditor instance;
};

K_GLOBAL_STATIC( HtmlEditorPrivate, instancePrivate )

HtmlEditor* HtmlEditor::self()
{
//     if ( !instancePrivate->instance ) {
//         instance = new HtmlEditor;
//     }
    return &instancePrivate->instance;
}

HtmlEditor::HtmlEditor() : QObject()
{
    mEditor = KTextEditor::EditorChooser::editor();
}

HtmlEditor::~HtmlEditor()
{
    kDebug();
    if ( !instancePrivate.isDestroyed() ) {
        delete mEditor;
        kDebug() << "editor deleted";
    }
}

KTextEditor::View* HtmlEditor::createView( QWidget* parent )
{
    KTextEditor::Document *document = mEditor->createDocument( parent );
    bool result = document->setHighlightingMode( "html" );
    if ( result ) {
        kDebug() << "Syntax highlighting enabled";
    }
    KTextEditor::View *view = document->createView( parent );
    QMenu *menu = view->defaultContextMenu();

    KTextEditor::ConfigInterface *interface = qobject_cast< KTextEditor::ConfigInterface* >( view );

    if ( interface ) {
        KAction *actWordWrap = new KAction( i18n( "Dynamic Word Wrap" ), view );
        actWordWrap->setCheckable( true );
        connect( actWordWrap, SIGNAL( triggered( bool ) ), this, SLOT( toggleWordWrap() ) );

        KAction *actLineNumber = new KAction( i18n("Show line numbers"), view );
        actLineNumber->setCheckable( true );
        connect( actLineNumber, SIGNAL( triggered( bool ) ), this, SLOT( toggleLineNumber() ) );

        QMenu *options = new QMenu( i18n( "Options" ), qobject_cast< QWidget* >( view ) );
        options->addAction( actWordWrap );
        options->addAction( actLineNumber );

        menu->addSeparator();
        menu->addMenu( options );
        
        interface->setConfigValue( "dynamic-word-wrap", true );
        actWordWrap->setChecked( true );
    }
    view->setContextMenu( menu );
    return view;
}

// void HtmlEditor::setContent( KTextEditor::View* view, const QString &text)
// {
//     view->document()->setText( text );
// }

QWidget* HtmlEditor::configPage( int number, QWidget* parent )
{
    KTextEditor::ConfigPage *page = mEditor->configPage( number, parent );
    if ( !page ) {
        return NULL;
    } else {
        return page;
    }
}

void HtmlEditor::toggleWordWrap()
{
    KTextEditor::View *view = qobject_cast< KTextEditor::View* >( sender()->parent() );
    KTextEditor::ConfigInterface *interface = qobject_cast< KTextEditor::ConfigInterface* >( view );
    bool result = interface->configValue( "dynamic-word-wrap" ).toBool();
    interface->setConfigValue( "dynamic-word-wrap", !( result ) );
}

void HtmlEditor::toggleLineNumber()
{
    KTextEditor::View *view = qobject_cast< KTextEditor::View* >( sender()->parent() );
    KTextEditor::ConfigInterface *interface = qobject_cast< KTextEditor::ConfigInterface* >( view );
    bool result = interface->configValue( "line-numbers" ).toBool();
    interface->setConfigValue( "line-numbers", !( result ) );
}

#include "composer/htmleditor.moc"
