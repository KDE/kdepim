/*
    knsyntaxhighlighter.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2003 the KNode authors.
    See file AUTHORS for details

    Portions of this file Copyright (c) 2003 Trolltech AS.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <qtextedit.h>
#include <qsyntaxhighlighter.h>
#include <qcolor.h>
#include <qstringlist.h>
#include <qregexp.h>

#include "knconfigmanager.h"

#include "knsyntaxhighlighter.h"
#include "knglobals.h"

KNSyntaxHighlighter::KNSyntaxHighlighter( QTextEdit *textEdit )
	: QSyntaxHighlighter( textEdit )
{
}

int KNSyntaxHighlighter::highlightParagraph( const QString &text, int )
{
    QString simplified = text;

    simplified = simplified.replace( QRegExp( "\\s" ), "" ).replace( "|", ">" );
    while ( simplified.startsWith( ">>>>" ) )
	simplified = simplified.mid(3);
    if  ( simplified.startsWith( ">>>" ) || simplified.startsWith( "> > >" ) )
	setFormat( 0, text.length(), knGlobals.cfgManager->appearance()->quoteColor3() );
    else if     ( simplified.startsWith( ">>" ) || simplified.startsWith( "> >" ) )
        setFormat( 0, text.length(), knGlobals.cfgManager->appearance()->quoteColor2() );
    else if     ( simplified.startsWith( ">" ) )
        setFormat( 0, text.length(), knGlobals.cfgManager->appearance()->quoteColor1() );
    else
        setFormat( 0, text.length(), knGlobals.cfgManager->appearance()->textColor() );

    return 0;
}
