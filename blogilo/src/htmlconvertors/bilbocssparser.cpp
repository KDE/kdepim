/***************************************************************************
 *   This file is part of the Bilbo Blogger.                               *
 *   Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>     *
 *   Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>          *
 *                                                                         *
 *   It is a modified version of "qcssparser.cpp", which was part          *
 *   of the QtGui module of the Qt Toolkit. It has been customized for     *
 *   use in Bilbo Blogger, at December 2008.                               *
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

#include "bilbocssparser.h"

//#include "qcssparser_p.h"
//#include "bilbocssparser.h"

#include <QDebug>
#include <QColor>
#include <QFont>
#include <QFileInfo>
#include <QFontMetrics>
#include <QBrush>

//#include "qcssscanner.cpp"

using namespace QCss;

QCssScanner_Generated::QCssScanner_Generated( const QString &inp )
{
    input = inp;
    pos = 0;
    lexemStart = 0;
    lexemLength = 0;
}

QCssScanner_Generated::~QCssScanner_Generated()
{
}

int QCssScanner_Generated::handleCommentStart()
{
    while ( pos < input.size() - 1 ) {
        if ( input.at( pos ) == QLatin1Char( '*' )
                && input.at( pos + 1 ) == QLatin1Char( '/' ) ) {
            pos += 2;
            break;
        }
        ++pos;
    }
    return QCss::S;
}

int QCssScanner_Generated::lex()
{
    lexemStart = pos;
    lexemLength = 0;
    int lastAcceptingPos = -1;
    int token = -1;
    QChar ch;

    // initial state
    ch = next();
    if (( ch.unicode() >= 9 ) && ( ch.unicode() <= 10 ) )
        goto state_1;
    if (( ch.unicode() >= 12 ) && ( ch.unicode() <= 13 ) )
        goto state_1;
    if ( ch.unicode() == 32 )
        goto state_1;
    if ( ch.unicode() == 33 ) {
        token = QCss::EXCLAMATION_SYM;
        goto found;
    }
    if ( ch.unicode() == 34 )
        goto state_3;
    if ( ch.unicode() == 35 )
        goto state_4;
    if ( ch.unicode() == 39 )
        goto state_5;
    if ( ch.unicode() == 40 ) {
        token = QCss::LPAREN;
        goto found;
    }
    if ( ch.unicode() == 41 ) {
        token = QCss::RPAREN;
        goto found;
    }
    if ( ch.unicode() == 42 ) {
        token = QCss::STAR;
        goto found;
    }
    if ( ch.unicode() == 43 )
        goto state_9;
    if ( ch.unicode() == 44 )
        goto state_10;
    if ( ch.unicode() == 45 )
        goto state_11;
    if ( ch.unicode() == 46 )
        goto state_12;
    if ( ch.unicode() == 47 )
        goto state_13;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_14;
    if ( ch.unicode() == 58 ) {
        token = QCss::COLON;
        goto found;
    }
    if ( ch.unicode() == 59 ) {
        token = QCss::SEMICOLON;
        goto found;
    }
    if ( ch.unicode() == 60 )
        goto state_17;
    if ( ch.unicode() == 61 ) {
        token = QCss::EQUAL;
        goto found;
    }
    if ( ch.unicode() == 62 )
        goto state_19;
    if ( ch.unicode() == 64 )
        goto state_20;
    if ( ch.unicode() == 91 ) {
        token = QCss::LBRACKET;
        goto found;
    }
    if ( ch.unicode() == 92 )
        goto state_22;
    if ( ch.unicode() == 93 ) {
        token = QCss::RBRACKET;
        goto found;
    }
    if ( ch.unicode() == 95 )
        goto state_24;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_24;
    if ( ch.unicode() == 123 )
        goto state_25;
    if ( ch.unicode() == 124 )
        goto state_26;
    if ( ch.unicode() == 125 ) {
        token = QCss::RBRACE;
        goto found;
    }
    if ( ch.unicode() == 126 )
        goto state_28;
    goto out;
state_1:
    lastAcceptingPos = pos;
    token = QCss::S;
    ch = next();
    if (( ch.unicode() >= 9 ) && ( ch.unicode() <= 10 ) )
        goto state_29;
    if (( ch.unicode() >= 12 ) && ( ch.unicode() <= 13 ) )
        goto state_29;
    if ( ch.unicode() == 32 )
        goto state_29;
    if ( ch.unicode() == 43 )
        goto state_9;
    if ( ch.unicode() == 44 )
        goto state_10;
    if ( ch.unicode() == 62 )
        goto state_19;
    if ( ch.unicode() == 123 )
        goto state_25;
    goto out;
state_3:
    lastAcceptingPos = pos;
    token = QCss::INVALID;
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_30;
    if ( ch.unicode() == 11 )
        goto state_30;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 33 ) )
        goto state_30;
    if ( ch.unicode() == 34 )
        goto state_31;
    if (( ch.unicode() >= 35 ) && ( ch.unicode() <= 91 ) )
        goto state_30;
    if ( ch.unicode() == 92 )
        goto state_32;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_30;
    if ( ch.unicode() >= 123 )
        goto state_30;
    goto out;
state_4:
    ch = next();
    if ( ch.unicode() == 45 )
        goto state_33;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_33;
    if ( ch.unicode() == 92 )
        goto state_34;
    if ( ch.unicode() == 95 )
        goto state_33;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_33;
    goto out;
state_5:
    lastAcceptingPos = pos;
    token = QCss::INVALID;
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_35;
    if ( ch.unicode() == 11 )
        goto state_35;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 38 ) )
        goto state_35;
    if ( ch.unicode() == 39 )
        goto state_36;
    if (( ch.unicode() >= 40 ) && ( ch.unicode() <= 91 ) )
        goto state_35;
    if ( ch.unicode() == 92 )
        goto state_37;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_35;
    if ( ch.unicode() >= 123 )
        goto state_35;
    goto out;
state_9:
    lastAcceptingPos = pos;
    token = QCss::PLUS;
    goto out;
state_10:
    lastAcceptingPos = pos;
    token = QCss::COMMA;
    goto out;
state_11:
    lastAcceptingPos = pos;
    token = QCss::MINUS;
    ch = next();
    if ( ch.unicode() == 45 )
        goto state_38;
    if ( ch.unicode() == 92 )
        goto state_22;
    if ( ch.unicode() == 95 )
        goto state_24;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_24;
    goto out;
state_12:
    lastAcceptingPos = pos;
    token = QCss::DOT;
    ch = next();
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_39;
    goto out;
state_13:
    lastAcceptingPos = pos;
    token = QCss::SLASH;
    ch = next();
    if ( ch.unicode() == 42 ) {
        token = handleCommentStart();
        goto found;
    }
    goto out;
state_14:
    lastAcceptingPos = pos;
    token = QCss::NUMBER;
    ch = next();
    if ( ch.unicode() == 37 )
        goto state_41;
    if ( ch.unicode() == 45 )
        goto state_42;
    if ( ch.unicode() == 46 )
        goto state_43;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_44;
    if ( ch.unicode() == 92 )
        goto state_45;
    if ( ch.unicode() == 95 )
        goto state_46;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_46;
    goto out;
state_17:
    ch = next();
    if ( ch.unicode() == 33 )
        goto state_47;
    goto out;
state_19:
    lastAcceptingPos = pos;
    token = QCss::GREATER;
    goto out;
state_20:
    ch = next();
    if ( ch.unicode() == 45 )
        goto state_48;
    if ( ch.unicode() == 92 )
        goto state_49;
    if ( ch.unicode() == 95 )
        goto state_50;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_50;
    goto out;
state_22:
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_51;
    if ( ch.unicode() == 11 )
        goto state_51;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 47 ) )
        goto state_51;
    if (( ch.unicode() >= 58 ) && ( ch.unicode() <= 96 ) )
        goto state_51;
    if ( ch.unicode() >= 103 )
        goto state_51;
    goto out;
state_24:
    lastAcceptingPos = pos;
    token = QCss::IDENT;
    ch = next();
    if ( ch.unicode() == 40 )
        goto state_52;
    if ( ch.unicode() == 45 )
        goto state_53;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_53;
    if ( ch.unicode() == 92 )
        goto state_54;
    if ( ch.unicode() == 95 )
        goto state_53;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_53;
    goto out;
state_25:
    lastAcceptingPos = pos;
    token = QCss::LBRACE;
    goto out;
state_26:
    lastAcceptingPos = pos;
    token = QCss::OR;
    ch = next();
    if ( ch.unicode() == 61 ) {
        token = QCss::DASHMATCH;
        goto found;
    }
    goto out;
state_28:
    ch = next();
    if ( ch.unicode() == 61 ) {
        token = QCss::INCLUDES;
        goto found;
    }
    goto out;
state_29:
    lastAcceptingPos = pos;
    token = QCss::S;
    ch = next();
    if (( ch.unicode() >= 9 ) && ( ch.unicode() <= 10 ) )
        goto state_29;
    if (( ch.unicode() >= 12 ) && ( ch.unicode() <= 13 ) )
        goto state_29;
    if ( ch.unicode() == 32 )
        goto state_29;
    if ( ch.unicode() == 43 )
        goto state_9;
    if ( ch.unicode() == 44 )
        goto state_10;
    if ( ch.unicode() == 62 )
        goto state_19;
    if ( ch.unicode() == 123 )
        goto state_25;
    goto out;
state_30:
    lastAcceptingPos = pos;
    token = QCss::INVALID;
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_30;
    if ( ch.unicode() == 11 )
        goto state_30;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 33 ) )
        goto state_30;
    if ( ch.unicode() == 34 )
        goto state_31;
    if (( ch.unicode() >= 35 ) && ( ch.unicode() <= 91 ) )
        goto state_30;
    if ( ch.unicode() == 92 )
        goto state_32;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_30;
    if ( ch.unicode() >= 123 )
        goto state_30;
    goto out;
state_31:
    lastAcceptingPos = pos;
    token = QCss::STRING;
    goto out;
state_32:
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_57;
    if ( ch.unicode() == 10 )
        goto state_58;
    if ( ch.unicode() == 11 )
        goto state_57;
    if ( ch.unicode() == 12 )
        goto state_59;
    if ( ch.unicode() == 13 )
        goto state_60;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 47 ) )
        goto state_57;
    if (( ch.unicode() >= 58 ) && ( ch.unicode() <= 96 ) )
        goto state_57;
    if ( ch.unicode() >= 103 )
        goto state_57;
    goto out;
state_33:
    lastAcceptingPos = pos;
    token = QCss::HASH;
    ch = next();
    if ( ch.unicode() == 45 )
        goto state_61;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_61;
    if ( ch.unicode() == 92 )
        goto state_62;
    if ( ch.unicode() == 95 )
        goto state_61;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_61;
    goto out;
state_34:
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_63;
    if ( ch.unicode() == 11 )
        goto state_63;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 47 ) )
        goto state_63;
    if (( ch.unicode() >= 58 ) && ( ch.unicode() <= 96 ) )
        goto state_63;
    if ( ch.unicode() >= 103 )
        goto state_63;
    goto out;
state_35:
    lastAcceptingPos = pos;
    token = QCss::INVALID;
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_35;
    if ( ch.unicode() == 11 )
        goto state_35;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 38 ) )
        goto state_35;
    if ( ch.unicode() == 39 )
        goto state_36;
    if (( ch.unicode() >= 40 ) && ( ch.unicode() <= 91 ) )
        goto state_35;
    if ( ch.unicode() == 92 )
        goto state_37;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_35;
    if ( ch.unicode() >= 123 )
        goto state_35;
    goto out;
state_36:
    lastAcceptingPos = pos;
    token = QCss::STRING;
    goto out;
state_37:
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_64;
    if ( ch.unicode() == 10 )
        goto state_65;
    if ( ch.unicode() == 11 )
        goto state_64;
    if ( ch.unicode() == 12 )
        goto state_66;
    if ( ch.unicode() == 13 )
        goto state_67;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 47 ) )
        goto state_64;
    if (( ch.unicode() >= 58 ) && ( ch.unicode() <= 96 ) )
        goto state_64;
    if ( ch.unicode() >= 103 )
        goto state_64;
    goto out;
state_38:
    ch = next();
    if ( ch.unicode() == 62 ) {
        token = QCss::CDC;
        goto found;
    }
    goto out;
state_39:
    lastAcceptingPos = pos;
    token = QCss::NUMBER;
    ch = next();
    if ( ch.unicode() == 37 )
        goto state_41;
    if ( ch.unicode() == 45 )
        goto state_42;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_69;
    if ( ch.unicode() == 92 )
        goto state_45;
    if ( ch.unicode() == 95 )
        goto state_46;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_46;
    goto out;
state_41:
    lastAcceptingPos = pos;
    token = QCss::PERCENTAGE;
    goto out;
state_42:
    ch = next();
    if ( ch.unicode() == 92 )
        goto state_45;
    if ( ch.unicode() == 95 )
        goto state_46;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_46;
    goto out;
state_43:
    ch = next();
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_39;
    goto out;
state_44:
    lastAcceptingPos = pos;
    token = QCss::NUMBER;
    ch = next();
    if ( ch.unicode() == 37 )
        goto state_41;
    if ( ch.unicode() == 45 )
        goto state_42;
    if ( ch.unicode() == 46 )
        goto state_43;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_44;
    if ( ch.unicode() == 92 )
        goto state_45;
    if ( ch.unicode() == 95 )
        goto state_46;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_46;
    goto out;
state_45:
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_70;
    if ( ch.unicode() == 11 )
        goto state_70;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 47 ) )
        goto state_70;
    if (( ch.unicode() >= 58 ) && ( ch.unicode() <= 96 ) )
        goto state_70;
    if ( ch.unicode() >= 103 )
        goto state_70;
    goto out;
state_46:
    lastAcceptingPos = pos;
    token = QCss::LENGTH;
    ch = next();
    if ( ch.unicode() == 45 )
        goto state_71;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_71;
    if ( ch.unicode() == 92 )
        goto state_72;
    if ( ch.unicode() == 95 )
        goto state_71;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_71;
    goto out;
state_47:
    ch = next();
    if ( ch.unicode() == 45 )
        goto state_73;
    goto out;
state_48:
    ch = next();
    if ( ch.unicode() == 92 )
        goto state_49;
    if ( ch.unicode() == 95 )
        goto state_50;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_50;
    goto out;
state_49:
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_74;
    if ( ch.unicode() == 11 )
        goto state_74;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 47 ) )
        goto state_74;
    if (( ch.unicode() >= 58 ) && ( ch.unicode() <= 96 ) )
        goto state_74;
    if ( ch.unicode() >= 103 )
        goto state_74;
    goto out;
state_50:
    lastAcceptingPos = pos;
    token = QCss::ATKEYWORD_SYM;
    ch = next();
    if ( ch.unicode() == 45 )
        goto state_75;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_75;
    if ( ch.unicode() == 92 )
        goto state_76;
    if ( ch.unicode() == 95 )
        goto state_75;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_75;
    goto out;
state_51:
    lastAcceptingPos = pos;
    token = QCss::IDENT;
    ch = next();
    if ( ch.unicode() == 40 )
        goto state_52;
    if ( ch.unicode() == 45 )
        goto state_53;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_53;
    if ( ch.unicode() == 92 )
        goto state_54;
    if ( ch.unicode() == 95 )
        goto state_53;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_53;
    goto out;
state_52:
    lastAcceptingPos = pos;
    token = QCss::FUNCTION;
    goto out;
state_53:
    lastAcceptingPos = pos;
    token = QCss::IDENT;
    ch = next();
    if ( ch.unicode() == 40 )
        goto state_52;
    if ( ch.unicode() == 45 )
        goto state_53;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_53;
    if ( ch.unicode() == 92 )
        goto state_54;
    if ( ch.unicode() == 95 )
        goto state_53;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_53;
    goto out;
state_54:
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_77;
    if ( ch.unicode() == 11 )
        goto state_77;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 47 ) )
        goto state_77;
    if (( ch.unicode() >= 58 ) && ( ch.unicode() <= 96 ) )
        goto state_77;
    if ( ch.unicode() >= 103 )
        goto state_77;
    goto out;
state_57:
    lastAcceptingPos = pos;
    token = QCss::INVALID;
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_30;
    if ( ch.unicode() == 11 )
        goto state_30;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 33 ) )
        goto state_30;
    if ( ch.unicode() == 34 )
        goto state_31;
    if (( ch.unicode() >= 35 ) && ( ch.unicode() <= 91 ) )
        goto state_30;
    if ( ch.unicode() == 92 )
        goto state_32;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_30;
    if ( ch.unicode() >= 123 )
        goto state_30;
    goto out;
state_58:
    lastAcceptingPos = pos;
    token = QCss::INVALID;
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_30;
    if ( ch.unicode() == 11 )
        goto state_30;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 33 ) )
        goto state_30;
    if ( ch.unicode() == 34 )
        goto state_31;
    if (( ch.unicode() >= 35 ) && ( ch.unicode() <= 91 ) )
        goto state_30;
    if ( ch.unicode() == 92 )
        goto state_32;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_30;
    if ( ch.unicode() >= 123 )
        goto state_30;
    goto out;
state_59:
    lastAcceptingPos = pos;
    token = QCss::INVALID;
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_30;
    if ( ch.unicode() == 11 )
        goto state_30;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 33 ) )
        goto state_30;
    if ( ch.unicode() == 34 )
        goto state_31;
    if (( ch.unicode() >= 35 ) && ( ch.unicode() <= 91 ) )
        goto state_30;
    if ( ch.unicode() == 92 )
        goto state_32;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_30;
    if ( ch.unicode() >= 123 )
        goto state_30;
    goto out;
state_60:
    lastAcceptingPos = pos;
    token = QCss::INVALID;
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_30;
    if ( ch.unicode() == 10 )
        goto state_78;
    if ( ch.unicode() == 11 )
        goto state_30;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 33 ) )
        goto state_30;
    if ( ch.unicode() == 34 )
        goto state_31;
    if (( ch.unicode() >= 35 ) && ( ch.unicode() <= 91 ) )
        goto state_30;
    if ( ch.unicode() == 92 )
        goto state_32;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_30;
    if ( ch.unicode() >= 123 )
        goto state_30;
    goto out;
state_61:
    lastAcceptingPos = pos;
    token = QCss::HASH;
    ch = next();
    if ( ch.unicode() == 45 )
        goto state_61;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_61;
    if ( ch.unicode() == 92 )
        goto state_62;
    if ( ch.unicode() == 95 )
        goto state_61;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_61;
    goto out;
state_62:
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_79;
    if ( ch.unicode() == 11 )
        goto state_79;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 47 ) )
        goto state_79;
    if (( ch.unicode() >= 58 ) && ( ch.unicode() <= 96 ) )
        goto state_79;
    if ( ch.unicode() >= 103 )
        goto state_79;
    goto out;
state_63:
    lastAcceptingPos = pos;
    token = QCss::HASH;
    ch = next();
    if ( ch.unicode() == 45 )
        goto state_61;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_61;
    if ( ch.unicode() == 92 )
        goto state_62;
    if ( ch.unicode() == 95 )
        goto state_61;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_61;
    goto out;
state_64:
    lastAcceptingPos = pos;
    token = QCss::INVALID;
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_35;
    if ( ch.unicode() == 11 )
        goto state_35;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 38 ) )
        goto state_35;
    if ( ch.unicode() == 39 )
        goto state_36;
    if (( ch.unicode() >= 40 ) && ( ch.unicode() <= 91 ) )
        goto state_35;
    if ( ch.unicode() == 92 )
        goto state_37;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_35;
    if ( ch.unicode() >= 123 )
        goto state_35;
    goto out;
state_65:
    lastAcceptingPos = pos;
    token = QCss::INVALID;
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_35;
    if ( ch.unicode() == 11 )
        goto state_35;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 38 ) )
        goto state_35;
    if ( ch.unicode() == 39 )
        goto state_36;
    if (( ch.unicode() >= 40 ) && ( ch.unicode() <= 91 ) )
        goto state_35;
    if ( ch.unicode() == 92 )
        goto state_37;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_35;
    if ( ch.unicode() >= 123 )
        goto state_35;
    goto out;
state_66:
    lastAcceptingPos = pos;
    token = QCss::INVALID;
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_35;
    if ( ch.unicode() == 11 )
        goto state_35;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 38 ) )
        goto state_35;
    if ( ch.unicode() == 39 )
        goto state_36;
    if (( ch.unicode() >= 40 ) && ( ch.unicode() <= 91 ) )
        goto state_35;
    if ( ch.unicode() == 92 )
        goto state_37;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_35;
    if ( ch.unicode() >= 123 )
        goto state_35;
    goto out;
state_67:
    lastAcceptingPos = pos;
    token = QCss::INVALID;
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_35;
    if ( ch.unicode() == 10 )
        goto state_80;
    if ( ch.unicode() == 11 )
        goto state_35;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 38 ) )
        goto state_35;
    if ( ch.unicode() == 39 )
        goto state_36;
    if (( ch.unicode() >= 40 ) && ( ch.unicode() <= 91 ) )
        goto state_35;
    if ( ch.unicode() == 92 )
        goto state_37;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_35;
    if ( ch.unicode() >= 123 )
        goto state_35;
    goto out;
state_69:
    lastAcceptingPos = pos;
    token = QCss::NUMBER;
    ch = next();
    if ( ch.unicode() == 37 )
        goto state_41;
    if ( ch.unicode() == 45 )
        goto state_42;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_69;
    if ( ch.unicode() == 92 )
        goto state_45;
    if ( ch.unicode() == 95 )
        goto state_46;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_46;
    goto out;
state_70:
    lastAcceptingPos = pos;
    token = QCss::LENGTH;
    ch = next();
    if ( ch.unicode() == 45 )
        goto state_71;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_71;
    if ( ch.unicode() == 92 )
        goto state_72;
    if ( ch.unicode() == 95 )
        goto state_71;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_71;
    goto out;
state_71:
    lastAcceptingPos = pos;
    token = QCss::LENGTH;
    ch = next();
    if ( ch.unicode() == 45 )
        goto state_71;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_71;
    if ( ch.unicode() == 92 )
        goto state_72;
    if ( ch.unicode() == 95 )
        goto state_71;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_71;
    goto out;
state_72:
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_81;
    if ( ch.unicode() == 11 )
        goto state_81;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 47 ) )
        goto state_81;
    if (( ch.unicode() >= 58 ) && ( ch.unicode() <= 96 ) )
        goto state_81;
    if ( ch.unicode() >= 103 )
        goto state_81;
    goto out;
state_73:
    ch = next();
    if ( ch.unicode() == 45 ) {
        token = QCss::CDO;
        goto found;
    }
    goto out;
state_74:
    lastAcceptingPos = pos;
    token = QCss::ATKEYWORD_SYM;
    ch = next();
    if ( ch.unicode() == 45 )
        goto state_75;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_75;
    if ( ch.unicode() == 92 )
        goto state_76;
    if ( ch.unicode() == 95 )
        goto state_75;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_75;
    goto out;
state_75:
    lastAcceptingPos = pos;
    token = QCss::ATKEYWORD_SYM;
    ch = next();
    if ( ch.unicode() == 45 )
        goto state_75;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_75;
    if ( ch.unicode() == 92 )
        goto state_76;
    if ( ch.unicode() == 95 )
        goto state_75;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_75;
    goto out;
state_76:
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_83;
    if ( ch.unicode() == 11 )
        goto state_83;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 47 ) )
        goto state_83;
    if (( ch.unicode() >= 58 ) && ( ch.unicode() <= 96 ) )
        goto state_83;
    if ( ch.unicode() >= 103 )
        goto state_83;
    goto out;
state_77:
    lastAcceptingPos = pos;
    token = QCss::IDENT;
    ch = next();
    if ( ch.unicode() == 40 )
        goto state_52;
    if ( ch.unicode() == 45 )
        goto state_53;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_53;
    if ( ch.unicode() == 92 )
        goto state_54;
    if ( ch.unicode() == 95 )
        goto state_53;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_53;
    goto out;
state_78:
    lastAcceptingPos = pos;
    token = QCss::INVALID;
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_30;
    if ( ch.unicode() == 11 )
        goto state_30;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 33 ) )
        goto state_30;
    if ( ch.unicode() == 34 )
        goto state_31;
    if (( ch.unicode() >= 35 ) && ( ch.unicode() <= 91 ) )
        goto state_30;
    if ( ch.unicode() == 92 )
        goto state_32;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_30;
    if ( ch.unicode() >= 123 )
        goto state_30;
    goto out;
state_79:
    lastAcceptingPos = pos;
    token = QCss::HASH;
    ch = next();
    if ( ch.unicode() == 45 )
        goto state_61;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_61;
    if ( ch.unicode() == 92 )
        goto state_62;
    if ( ch.unicode() == 95 )
        goto state_61;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_61;
    goto out;
state_80:
    lastAcceptingPos = pos;
    token = QCss::INVALID;
    ch = next();
    if (( ch.unicode() >= 1 ) && ( ch.unicode() <= 9 ) )
        goto state_35;
    if ( ch.unicode() == 11 )
        goto state_35;
    if (( ch.unicode() >= 14 ) && ( ch.unicode() <= 38 ) )
        goto state_35;
    if ( ch.unicode() == 39 )
        goto state_36;
    if (( ch.unicode() >= 40 ) && ( ch.unicode() <= 91 ) )
        goto state_35;
    if ( ch.unicode() == 92 )
        goto state_37;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_35;
    if ( ch.unicode() >= 123 )
        goto state_35;
    goto out;
state_81:
    lastAcceptingPos = pos;
    token = QCss::LENGTH;
    ch = next();
    if ( ch.unicode() == 45 )
        goto state_71;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_71;
    if ( ch.unicode() == 92 )
        goto state_72;
    if ( ch.unicode() == 95 )
        goto state_71;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_71;
    goto out;
state_83:
    lastAcceptingPos = pos;
    token = QCss::ATKEYWORD_SYM;
    ch = next();
    if ( ch.unicode() == 45 )
        goto state_75;
    if (( ch.unicode() >= 48 ) && ( ch.unicode() <= 57 ) )
        goto state_75;
    if ( ch.unicode() == 92 )
        goto state_76;
    if ( ch.unicode() == 95 )
        goto state_75;
    if ((( ch.unicode() >= 'a' ) && ( ch.unicode() <= 'z' ) ) || ( ch.unicode() >= 256 ) )
        goto state_75;
    goto out;
found:
    lastAcceptingPos = pos;

out:
    if ( lastAcceptingPos != -1 ) {
        lexemLength = lastAcceptingPos - lexemStart;
        pos = lastAcceptingPos;
    }
    return token;
}


const char *Scanner::tokenName( TokenType t )
{
    switch ( t ) {
        case NONE:
            return "NONE";
        case S:
            return "S";
        case CDO:
            return "CDO";
        case CDC:
            return "CDC";
        case INCLUDES:
            return "INCLUDES";
        case DASHMATCH:
            return "DASHMATCH";
        case LBRACE:
            return "LBRACE";
        case PLUS:
            return "PLUS";
        case GREATER:
            return "GREATER";
        case COMMA:
            return "COMMA";
        case STRING:
            return "STRING";
        case INVALID:
            return "INVALID";
        case IDENT:
            return "IDENT";
        case HASH:
            return "HASH";
        case ATKEYWORD_SYM:
            return "ATKEYWORD_SYM";
        case EXCLAMATION_SYM:
            return "EXCLAMATION_SYM";
        case LENGTH:
            return "LENGTH";
        case PERCENTAGE:
            return "PERCENTAGE";
        case NUMBER:
            return "NUMBER";
        case FUNCTION:
            return "FUNCTION";
        case COLON:
            return "COLON";
        case SEMICOLON:
            return "SEMICOLON";
        case RBRACE:
            return "RBRACE";
        case SLASH:
            return "SLASH";
        case MINUS:
            return "MINUS";
        case DOT:
            return "DOT";
        case STAR:
            return "STAR";
        case LBRACKET:
            return "LBRACKET";
        case RBRACKET:
            return "RBRACKET";
        case EQUAL:
            return "EQUAL";
        case LPAREN:
            return "LPAREN";
        case RPAREN:
            return "RPAREN";
        case OR:
            return "OR";
    }
    return "";
}

struct QCssKnownValue {
    const char *name;
    int id;
};

static const QCssKnownValue properties[NumProperties - 1] = {
    { "-qt-block-indent", QtBlockIndent },
    { "-qt-list-indent", QtListIndent },
    { "-qt-paragraph-type", QtParagraphType },
    { "-qt-table-type", QtTableType },
    { "alternate-background-color", QtAlternateBackground },
    { "background", Background },
    { "background-color", BackgroundColor },
    { "background-image", BackgroundImage },
    { "background-origin", BackgroundOrigin },
    { "background-position", BackgroundPosition },
    { "background-repeat", BackgroundRepeat },
    { "border", Border },
    { "border-bottom", BorderBottom },
    { "border-bottom-color", BorderBottomColor },
    { "border-bottom-left-radius", BorderBottomLeftRadius },
    { "border-bottom-right-radius", BorderBottomRightRadius },
    { "border-bottom-style", BorderBottomStyle },
    { "border-bottom-width", BorderBottomWidth },
    { "border-color", BorderColor },
    { "border-image", BorderImage },
    { "border-left", BorderLeft },
    { "border-left-color", BorderLeftColor },
    { "border-left-style", BorderLeftStyle },
    { "border-left-width", BorderLeftWidth },
    { "border-radius", BorderRadius },
    { "border-right", BorderRight },
    { "border-right-color", BorderRightColor },
    { "border-right-style", BorderRightStyle },
    { "border-right-width", BorderRightWidth },
    { "border-style", BorderStyles },
    { "border-top", BorderTop },
    { "border-top-color", BorderTopColor },
    { "border-top-left-radius", BorderTopLeftRadius },
    { "border-top-right-radius", BorderTopRightRadius },
    { "border-top-style", BorderTopStyle },
    { "border-top-width", BorderTopWidth },
    { "border-width", BorderWidth },
    { "bottom", Bottom },
    { "color", Color },
    { "float", Float },
    { "font", Font },
    { "font-family", FontFamily },
    { "font-size", FontSize },
    { "font-style", FontStyle },
    { "font-weight", FontWeight },
    { "height", Height },
    { "image", QtImage },
    { "left", Left },
    { "margin" , Margin },
    { "margin-bottom", MarginBottom },
    { "margin-left", MarginLeft },
    { "margin-right", MarginRight },
    { "margin-top", MarginTop },
    { "max-height", MaximumHeight },
    { "max-width", MaximumWidth },
    { "min-height", MinimumHeight },
    { "min-width", MinimumWidth },
    { "padding", Padding },
    { "padding-bottom", PaddingBottom },
    { "padding-left", PaddingLeft },
    { "padding-right", PaddingRight },
    { "padding-top", PaddingTop },
    { "page-break-after", PageBreakAfter },
    { "page-break-before", PageBreakBefore },
    { "position", Position },
    { "right", Right },
    { "selection-background-color", QtSelectionBackground },
    { "selection-color", QtSelectionForeground },
    { "spacing", QtSpacing },
    { "subcontrol-origin", QtOrigin },
    { "subcontrol-position", QtPosition },
    { "text-decoration", TextDecoration },
    { "text-indent", TextIndent },
    { "top", Top },
    { "vertical-align", VerticalAlignment },
    { "white-space", Whitespace },
    { "width", Width }
};

static const QCssKnownValue values[NumKnownValues - 1] = {
    { "always", Value_Always },
    { "auto", Value_Auto },
    { "bold", Value_Bold },
    { "bottom", Value_Bottom },
    { "center", Value_Center },
    { "italic", Value_Italic },
    { "large", Value_Large },
    { "left", Value_Left },
    { "line-through", Value_LineThrough },
    { "medium", Value_Medium },
    { "native", Value_Native },
    { "none", Value_None },
    { "normal", Value_Normal },
    { "nowrap", Value_NoWrap },
    { "oblique", Value_Oblique },
    { "overline", Value_Overline },
    { "pre", Value_Pre },
    { "pre-wrap", Value_PreWrap },
    { "right", Value_Right },
    { "small" , Value_Small },
    { "sub", Value_Sub },
    { "super", Value_Super },
    { "top", Value_Top },
    { "underline", Value_Underline },
    { "x-large", Value_XLarge },
    { "xx-large", Value_XXLarge }
};

static const QCssKnownValue pseudos[NumPseudos - 1] = {
    { "checked", PseudoState_Checked },
    { "disabled", PseudoState_Disabled },
    { "enabled", PseudoState_Enabled },
    { "focus", PseudoState_Focus },
    { "hover", PseudoState_Hover },
    { "indeterminate" , PseudoState_Indeterminate },
    { "off", PseudoState_Unchecked },
    { "on", PseudoState_Checked },
    { "pressed", PseudoState_Pressed },
    { "unchecked" , PseudoState_Unchecked }
};

static const QCssKnownValue borderStyles[NumKnownBorderStyles - 1] = {
    { "dashed", BorderStyle_Dashed },
    { "dot-dash", BorderStyle_DotDash },
    { "dot-dot-dash", BorderStyle_DotDotDash },
    { "dotted", BorderStyle_Dotted },
    { "double", BorderStyle_Double },
    { "groove", BorderStyle_Groove },
    { "inset", BorderStyle_Inset },
    { "none", BorderStyle_None }, // note: parsed as Value_None
    { "outset", BorderStyle_Outset },
    { "ridge", BorderStyle_Ridge },
    { "solid", BorderStyle_Solid }
};

static const QCssKnownValue origins[NumKnownOrigins - 1] = {
    { "border", Origin_Border },
    { "content", Origin_Content },
    { "margin", Origin_Margin }, // not in css
    { "padding", Origin_Padding }
};

static const QCssKnownValue repeats[NumKnownRepeats - 1] = {
    { "no-repeat", Repeat_None },
    { "repeat-x", Repeat_X },
    { "repeat-xy", Repeat_XY },
    { "repeat-y", Repeat_Y }
};

static const QCssKnownValue tileModes[NumKnownTileModes - 1] = {
    { "repeat", TileMode_Repeat },
    { "round", TileMode_Round },
    { "stretch", TileMode_Stretch },
};

static const QCssKnownValue positions[NumKnownPositionModes - 1] = {
    { "absolute", PositionMode_Absolute },
    { "fixed", PositionMode_Fixed },
    { "relative", PositionMode_Relative },
    { "static", PositionMode_Static }
};

static bool operator<( const QString &name, const QCssKnownValue &prop )
{
    return QString::compare( name, QLatin1String( prop.name ), Qt::CaseInsensitive ) < 0;
}

static bool operator<( const QCssKnownValue &prop, const QString &name )
{
    return QString::compare( QLatin1String( prop.name ), name, Qt::CaseInsensitive ) < 0;
}

static int findKnownValue( const QString &name, const QCssKnownValue *start, int numValues )
{
    const QCssKnownValue *end = &start[numValues - 1];
    const QCssKnownValue *prop = qBinaryFind( start, end, name );
    if ( prop == end )
        return 0;
    return prop->id;
}

///////////////////////////////////////////////////////////////////////////////
// Value Extractor
ValueExtractor::ValueExtractor( const QVector<Declaration> &decls )
        : declarations( decls ), adjustment( 0 ), fontExtracted( false )
{
}

int ValueExtractor::lengthValue( const Value& v )
{
    QString s = v.variant.toString();
    QRegExp re( "(-?[\\d\\.]*)(px|em|ex)?", Qt::CaseInsensitive );
    if ( re.indexIn( s ) == -1 )
        return 0;
    QString real = re.cap( 1 );
    QString unit = re.cap( 2 );

    bool ok;
    qreal number = real.toDouble( &ok );
    if ( !ok )
        return 0;

    QFontMetrics fm( f );
    if ( unit.compare( "ex", Qt::CaseInsensitive ) == 0 )
        return qRound( fm.xHeight() * number );
    else if ( unit.compare( "em", Qt::CaseInsensitive ) == 0 )
        return qRound( fm.height() * number );

    return qRound( number );
}

int ValueExtractor::lengthValue( const Declaration &decl )
{
    if ( decl.values.count() < 1 )
        return 0;
    return lengthValue( decl.values.first() );
}

void ValueExtractor::lengthValues( const Declaration &decl, int *m )
{
    int i;
    for ( i = 0; i < qMin( decl.values.count(), 4 ); i++ )
        m[i] = lengthValue( decl.values[i] );

    if ( i == 0 ) {
        m[0] = m[1] = m[2] = m[3] = 0;
    } else if ( i == 1 ) {
        m[3] = m[2] = m[1] = m[0];
    } else if ( i == 2 ) {
        m[2] = m[0], m[3] = m[1];
    } else if ( i == 3 ) {
        m[3] = m[1];
    }
}

bool ValueExtractor::extractGeometry( int *w, int *h, int *mw, int *mh )
{
    extractFont();
    bool hit = false;
    for ( int i = 0; i < declarations.count(); i++ ) {
        const Declaration &decl = declarations.at( i );
        switch ( decl.propertyId ) {
            case Width:
                *w = lengthValue( decl );
                break;
            case Height:
                *h = lengthValue( decl );
                break;
            case MinimumWidth:
                *mw = lengthValue( decl );
                break;
            case MinimumHeight:
                *mh = lengthValue( decl );
                break;
            default:
                continue;
        }
        hit = true;
    }

    return hit;
}

bool ValueExtractor::extractPosition( int *left, int *top, int *right, int *bottom,
                                      QCss::Origin *origin, Qt::Alignment *position, QCss::PositionMode *mode )
{
    extractFont();
    bool hit = false;
    for ( int i = 0; i < declarations.count(); i++ ) {
        const Declaration &decl = declarations.at( i );
        switch ( decl.propertyId ) {
            case Left:
                *left = lengthValue( decl );
                break;
            case Top:
                *top = lengthValue( decl );
                break;
            case Right:
                *right = lengthValue( decl );
                break;
            case Bottom:
                *bottom = lengthValue( decl );
                break;
            case QtOrigin:
                *origin = decl.originValue();
                break;
            case QtPosition:
                *position = decl.alignmentValue();
                break;
            case Position:
                *mode = decl.positionValue();
                break;
            default:
                continue;
        }
        hit = true;
    }

    return hit;
}

bool ValueExtractor::extractBox( int *margins, int *paddings, int *spacing )
{
    extractFont();
    bool hit = false;
    for ( int i = 0; i < declarations.count(); i++ ) {
        const Declaration &decl = declarations.at( i );
        switch ( decl.propertyId ) {
            case PaddingLeft:
                paddings[LeftEdge] = lengthValue( decl );
                break;
            case PaddingRight:
                paddings[RightEdge] = lengthValue( decl );
                break;
            case PaddingTop:
                paddings[TopEdge] = lengthValue( decl );
                break;
            case PaddingBottom:
                paddings[BottomEdge] = lengthValue( decl );
                break;
            case Padding:
                lengthValues( decl, paddings );
                break;
            case MarginLeft:
                margins[LeftEdge] = lengthValue( decl );
                break;
            case MarginRight:
                margins[RightEdge] = lengthValue( decl );
                break;
            case MarginTop:
                margins[TopEdge] = lengthValue( decl );
                break;
            case MarginBottom:
                margins[BottomEdge] = lengthValue( decl );
                break;
            case Margin:
                lengthValues( decl, margins );
                break;
            case QtSpacing:
                if ( spacing ) {
                    *spacing = lengthValue( decl );
                }
                break;
            default:
                continue;
        }
        hit = true;
    }

    return hit;
}

QSize ValueExtractor::sizeValue( const Declaration &decl )
{
    int x[2] = { 0, 0 };
    if ( decl.values.count() > 0 ) {
        x[0] = lengthValue( decl.values.at( 0 ) );
    }
    if ( decl.values.count() > 1 ) {
        x[1] = lengthValue( decl.values.at( 1 ) );
    } else {
        x[1] = x[0];
    }
    return QSize( x[0], x[1] );
}

void ValueExtractor::sizeValues( const Declaration &decl, QSize *radii )
{
    radii[0] = sizeValue( decl );
    for ( int i = 1; i < 4; i++ )
        radii[i] = radii[0];
}

bool ValueExtractor::extractBorder( int *borders, QColor *colors, BorderStyle *styles,
                                    QSize *radii )
{
    extractFont();
    bool hit = false;
    for ( int i = 0; i < declarations.count(); i++ ) {
        const Declaration &decl = declarations.at( i );
        switch ( decl.propertyId ) {
            case BorderLeftWidth:
                borders[LeftEdge] = lengthValue( decl );
                break;
            case BorderRightWidth:
                borders[RightEdge] = lengthValue( decl );
                break;
            case BorderTopWidth:
                borders[TopEdge] = lengthValue( decl );
                break;
            case BorderBottomWidth:
                borders[BottomEdge] = lengthValue( decl );
                break;
            case BorderWidth:
                lengthValues( decl, borders );
                break;
            case BorderLeftColor:
                colors[LeftEdge] = decl.colorValue();
                break;
            case BorderRightColor:
                colors[RightEdge] = decl.colorValue();
                break;
            case BorderTopColor:
                colors[TopEdge] = decl.colorValue();
                break;
            case BorderBottomColor:
                colors[BottomEdge] = decl.colorValue();
                break;
            case BorderColor:
                decl.colorValues( colors );
                break;
            case BorderTopStyle:
                styles[TopEdge] = decl.styleValue();
                break;
            case BorderBottomStyle:
                styles[BottomEdge] = decl.styleValue();
                break;
            case BorderLeftStyle:
                styles[LeftEdge] = decl.styleValue();
                break;
            case BorderRightStyle:
                styles[RightEdge] = decl.styleValue();
                break;
            case BorderStyles:
                decl.styleValues( styles );
                break;

            case BorderTopLeftRadius:
                radii[0] = sizeValue( decl );
                break;
            case BorderTopRightRadius:
                radii[1] = sizeValue( decl );
                break;
            case BorderBottomLeftRadius:
                radii[2] = sizeValue( decl );
                break;
            case BorderBottomRightRadius:
                radii[3] = sizeValue( decl );
                break;
            case BorderRadius:
                sizeValues( decl, radii );
                break;

            case BorderLeft:
                borderValue( decl, &borders[LeftEdge], &styles[LeftEdge], &colors[LeftEdge] );
                break;
            case BorderTop:
                borderValue( decl, &borders[TopEdge], &styles[TopEdge], &colors[TopEdge] );
                break;
            case BorderRight:
                borderValue( decl, &borders[RightEdge], &styles[RightEdge], &colors[RightEdge] );
                break;
            case BorderBottom:
                borderValue( decl, &borders[BottomEdge], &styles[BottomEdge], &colors[BottomEdge] );
                break;
            case Border:
                borderValue( decl, &borders[LeftEdge], &styles[LeftEdge], &colors[LeftEdge] );
                borders[TopEdge] = borders[RightEdge] = borders[BottomEdge] = borders[LeftEdge];
                styles[TopEdge] = styles[RightEdge] = styles[BottomEdge] = styles[LeftEdge];
                colors[TopEdge] = colors[RightEdge] = colors[BottomEdge] = colors[LeftEdge];
                break;

            default:
                continue;
        }
        hit = true;
    }

    return hit;
}

static Qt::Alignment parseAlignment( const Value *values, int count )
{
    Qt::Alignment a[2];
    for ( int i = 0; i < qMin( 2, count ); i++ ) {
        if ( values[i].type != Value::KnownIdentifier )
            break;
        switch ( values[i].variant.toInt() ) {
            case Value_Left:
                a[i] = Qt::AlignLeft;
                break;
            case Value_Right:
                a[i] = Qt::AlignRight;
                break;
            case Value_Top:
                a[i] = Qt::AlignTop;
                break;
            case Value_Bottom:
                a[i] = Qt::AlignBottom;
                break;
            case Value_Center:
                a[i] = Qt::AlignCenter;
                break;
            default:
                break;
        }
    }

    if (( a[0] == Qt::AlignCenter ) && ( a[1] != 0 ) && ( a[1] != Qt::AlignCenter ) )
        a[0] = (( a[1] == Qt::AlignLeft ) || ( a[1] == Qt::AlignRight ) ) ? Qt::AlignVCenter :
               Qt::AlignHCenter;
    if ((( a[1] == 0 ) || ( a[1] == Qt::AlignCenter ) ) && ( a[0] != Qt::AlignCenter ) )
        a[1] = (( a[0] == Qt::AlignLeft ) || ( a[0] == Qt::AlignRight ) ) ? Qt::AlignVCenter :
               Qt::AlignHCenter;
    return ( a[0] | a[1] );
}

static QColor parseColorValue( Value v )
{
    if (( v.type == Value::Identifier ) || ( v.type == Value::String ) ) {
        if ( v.variant.toString().compare( "transparent", Qt::CaseInsensitive ) != 0 ) {
            v.variant.convert( QVariant::Color );
        } else {
            v.variant = QColor( Qt::transparent );
        }
        v.type = Value::Color;
    }
    if ( v.type == Value::Color )
        return qvariant_cast<QColor>( v.variant );

    if ( v.type != Value::Function )
        return QColor();

    QStringList lst = v.variant.toStringList();
    if ( lst.count() != 2 )
        return QColor();

    // function name
    if ( lst.at( 0 ).compare( QLatin1String( "rgb" ), Qt::CaseInsensitive ) != 0 )
        return QColor();

    Parser p( lst.at( 1 ) );
    if ( !p.testExpr() )
        return QColor();

    QVector<Value> colorDigits;
    if ( !p.parseExpr( &colorDigits ) )
        return QColor();

    if (( colorDigits.count() != 5 )
            || ( colorDigits.at( 1 ).type != Value::TermOperatorComma )
            || ( colorDigits.at( 3 ).type != Value::TermOperatorComma ) )
        return QColor();

    for ( int i = 0; i < 5; i += 2 ) {
        if ( colorDigits.at( i ).type == Value::Percentage ) {
            colorDigits[i].variant = colorDigits.at( i ).variant.toDouble() * 255. / 100.;
            colorDigits[i].type = Value::Number;
        }
    }

    return QColor( colorDigits.at( 0 ).variant.toInt(), colorDigits.at( 2 ).variant.toInt(),
                   colorDigits.at( 4 ).variant.toInt() );
}

static QBrush parseBrushValue( Value v )
{
    QColor c = parseColorValue( v );
    if ( c.isValid() )
        return QBrush( c );

    if ( v.type != Value::Function )
        return QBrush();

    QStringList lst = v.variant.toStringList();
    if ( lst.count() != 2 )
        return QBrush();

    QStringList gradFuncs;
    gradFuncs << "qlineargradient" << "qradialgradient" << "qconicalgradient" << "qgradient";
    int gradType = -1;

    if (( lst.at( 0 ).compare( QLatin1String( "gradient" ), Qt::CaseInsensitive ) != 0 )
            && (( gradType = gradFuncs.indexOf( lst.at( 0 ).toLower() ) ) == -1 ) )
        return QBrush();

    QStringList gradientTypes;
    gradientTypes << "linear" << "radial" << "conical" << "gradient";
    QStringList params = lst.at( 1 ).split( QLatin1String( "," ), QString::SkipEmptyParts );
    QHash<QString, qreal> vars;
    QVector<QGradientStop> stops;
    QRegExp re( "\\s*(\\S*):\\s*(\\S*)\\s*(\\S*)" );
    int spread = -1;
    QStringList spreads;
    spreads << "pad" << "reflect" << "repeat";
    for ( int i = 0; i < params.count(); i++ ) {
        if ( re.indexIn( params.at( i ) ) == -1 )
            continue;
        QString attr = re.cap( 1 );
        QString value = re.cap( 2 );

        if ( attr.compare( QLatin1String( "type" ), Qt::CaseInsensitive ) == 0 ) {
            gradType = gradientTypes.indexOf( value );
        } else if ( attr.compare( QLatin1String( "spread" ), Qt::CaseInsensitive ) == 0 ) {
            spread = spreads.indexOf( value );
        } else if ( attr.compare( QLatin1String( "stop" ), Qt::CaseInsensitive ) == 0 ) {
            stops.append( QGradientStop( value.toDouble(), QColor( re.cap( 3 ) ) ) );
        } else {
            vars[attr] = value.toDouble();
        }
    }

    if ( gradType == 0 ) {
        QLinearGradient lg( vars.value( QLatin1String( "x1" ) ), vars.value( QLatin1String( "y1" ) ),
                            vars.value( QLatin1String( "x2" ) ), vars.value( QLatin1String( "y2" ) ) );
        lg.setStops( stops );
        if ( spread != -1 )
            lg.setSpread( QGradient::Spread( spread ) );
        return QBrush( lg );
    }

    if ( gradType == 1 ) {
        QRadialGradient rg( vars.value( QLatin1String( "cx" ) ), vars.value( QLatin1String( "cy" ) ),
                            vars.value( QLatin1String( "radius" ) ), vars.value( QLatin1String( "fx" ) ),
                            vars.value( QLatin1String( "fy" ) ) );
        rg.setStops( stops );
        if ( spread != -1 )
            rg.setSpread( QGradient::Spread( spread ) );
        return QBrush( rg );
    }

    if ( gradType == 2 ) {
        QConicalGradient cg( vars.value( QLatin1String( "cx" ) ), vars.value( QLatin1String( "cy" ) ),
                             vars.value( QLatin1String( "angle" ) ) );
        cg.setStops( stops );
        if ( spread != -1 )
            cg.setSpread( QGradient::Spread( spread ) );
        return QBrush( cg );
    }

    return QBrush();
}

static BorderStyle parseStyleValue( Value v )
{
    if ( v.type == Value::KnownIdentifier ) {
        if ( v.variant.toInt() == Value_None )
            return BorderStyle_None;
        return BorderStyle_Unknown;
    }

    return static_cast<BorderStyle>( findKnownValue( v.variant.toString(),
                                     borderStyles, NumKnownBorderStyles ) );
}

void ValueExtractor::borderValue( const Declaration &decl, int *width, QCss::BorderStyle *style, QColor *color )
{
    *width = 0;
    *style = BorderStyle_None;
    *color = QColor();

    if ( decl.values.isEmpty() )
        return;

    int i = 0;
    if (( decl.values.at( i ).type == Value::Length ) || ( decl.values.at( i ).type == Value::Number ) ) {
        *width = lengthValue( decl.values.at( i ) );
        if ( ++i >= decl.values.count() )
            return;
    }

    *style = parseStyleValue( decl.values.at( i ) );
    if ( *style != BorderStyle_Unknown ) {
        if ( ++i >= decl.values.count() )
            return;
    } else {
        *style = BorderStyle_None;
    }

    *color = parseColorValue( decl.values.at( i ) );
}

static void parseShorthandBackgroundProperty( const QVector<Value> &values, QBrush *brush,
        QString *image, Repeat *repeat, Qt::Alignment *alignment )
{
    *brush = QBrush();
    *image = QString();
    *repeat = Repeat_XY;
    *alignment = ( Qt::AlignTop | Qt::AlignLeft );

    for ( int i = 0; i < values.count(); ++i ) {
        const Value v = values.at( i );
        if ( v.type == Value::Uri ) {
            *image = v.variant.toString();
            continue;
        } else if (( v.type == Value::KnownIdentifier ) && ( v.variant.toInt() == Value_None ) ) {
            *image = QString();
            continue;
        }

        Repeat repeatAttempt = static_cast<Repeat>( findKnownValue( v.variant.toString(), repeats,
                               NumKnownRepeats ) );
        if ( repeatAttempt != Repeat_Unknown ) {
            *repeat = repeatAttempt;
            continue;
        }

        if ( v.type == Value::KnownIdentifier ) {
            const int start = i;
            int count = 1;
            if (( i < values.count() - 1 ) && ( values.at( i + 1 ).type == Value::KnownIdentifier ) ) {
                ++i;
                ++count;
            }
            Qt::Alignment a = parseAlignment( values.constData() + start, count );
            if ( int( a ) != 0 ) {
                *alignment = a;
                continue;
            }
            i -= count - 1;
        }

        *brush = parseBrushValue( v );
    }
}

bool ValueExtractor::extractBackground( QBrush *brush, QString *image, Repeat *repeat,
                                        Qt::Alignment *alignment, Origin *origin )
{
    bool hit = false;
    for ( int i = 0; i < declarations.count(); ++i ) {
        const Declaration &decl = declarations.at( i );
        if ( decl.values.isEmpty() )
            continue;
        const Value val = decl.values.first();
        switch ( decl.propertyId ) {
            case BackgroundColor:
                *brush = parseBrushValue( val );
                break;
            case BackgroundImage:
                if ( val.type == Value::Uri )
                    *image = val.variant.toString();
                break;
            case BackgroundRepeat:
                *repeat = static_cast<Repeat>( findKnownValue( val.variant.toString(), repeats,
                                               NumKnownRepeats ) );
                break;
            case BackgroundPosition:
                *alignment = decl.alignmentValue();
                break;
            case BackgroundOrigin:
                *origin = decl.originValue();
                break;
            case Background:
                parseShorthandBackgroundProperty( decl.values, brush, image, repeat, alignment );
                break;
            default:
                continue;
        }
        hit = true;
    }
    return hit;
}

static bool setFontSizeFromValue( Value value, QFont *font, int *fontSizeAdjustment )
{
    if ( value.type == Value::KnownIdentifier ) {
        bool valid = true;
        switch ( value.variant.toInt() ) {
            case Value_Small:
                *fontSizeAdjustment = -1;
                break;
            case Value_Medium:
                *fontSizeAdjustment = 0;
                break;
            case Value_Large:
                *fontSizeAdjustment = 1;
                break;
            case Value_XLarge:
                *fontSizeAdjustment = 2;
                break;
            case Value_XXLarge:
                *fontSizeAdjustment = 3;
                break;
            default:
                valid = false;
                break;
        }
        return valid;
    }
    if ( value.type != Value::Length )
        return false;

    bool valid = false;
    QString s = value.variant.toString();
    if ( s.endsWith( QLatin1String( "pt" ), Qt::CaseInsensitive ) ) {
        s.chop( 2 );
        value.variant = s;
        if ( value.variant.convert( QVariant::Double ) ) {
            font->setPointSizeF( value.variant.toDouble() );
            valid = true;
        }
    } else if ( s.endsWith( QLatin1String( "px" ), Qt::CaseInsensitive ) ) {
        s.chop( 2 );
        value.variant = s;
        if ( value.variant.convert( QVariant::Int ) ) {
            font->setPixelSize( value.variant.toInt() );
            valid = true;
        }
    }
    return valid;
}

static bool setFontStyleFromValue( const Value &value, QFont *font )
{
    if ( value.type != Value::KnownIdentifier )
        return false ;
    switch ( value.variant.toInt() ) {
        case Value_Normal:
            font->setStyle( QFont::StyleNormal );
            return true;
        case Value_Italic:
            font->setStyle( QFont::StyleItalic );
            return true;
        case Value_Oblique:
            font->setStyle( QFont::StyleOblique );
            return true;
        default:
            break;
    }
    return false;
}

static bool setFontWeightFromValue( const Value &value, QFont *font )
{
    if ( value.type == Value::KnownIdentifier ) {
        switch ( value.variant.toInt() ) {
            case Value_Normal:
                font->setWeight( QFont::Normal );
                return true;
            case Value_Bold:
                font->setWeight( QFont::Bold );
                return true;
            default:
                break;
        }
        return false;
    }
    if ( value.type != Value::Number )
        return false;
    font->setWeight( value.variant.toInt() / 8 );
    return true;
}

static bool setFontFamilyFromValues( const QVector<Value> &values, QFont *font )
{
    QString family;
    for ( int i = 0; i < values.count(); ++i ) {
        const Value &v = values.at( i );
        if ( v.type == Value::TermOperatorComma )
            break;
        const QString str = v.variant.toString();
        if ( str.isEmpty() )
            break;
        family += str;
        family += QLatin1Char( ' ' );
    }
    family = family.simplified();
    if ( family.isEmpty() )
        return false;
    font->setFamily( family );
    return true;
}

static void setTextDecorationFromValues( const QVector<Value> &values, QFont *font )
{
    font->setUnderline( false );
    font->setOverline( false );
    font->setStrikeOut( false );
    for ( int i = 0; i < values.count(); ++i ) {
        if ( values.at( i ).type != Value::KnownIdentifier )
            continue;
        switch ( values.at( i ).variant.toInt() ) {
            case Value_Underline:
                font->setUnderline( true );
                break;
            case Value_Overline:
                font->setOverline( true );
                break;
            case Value_LineThrough:
                font->setStrikeOut( true );
                break;
            default:
                break;
        }
    }
}

static void parseShorthandFontProperty( const QVector<Value> &values, QFont *font, int *fontSizeAdjustment )
{
    font->setStyle( QFont::StyleNormal );
    font->setWeight( QFont::Normal );
    *fontSizeAdjustment = 0;

    int i = 0;
    while ( i < values.count() ) {
        if (( setFontStyleFromValue( values.at( i ), font ) )
                || ( setFontWeightFromValue( values.at( i ), font ) ) )
            ++i;
        else
            break;
    }

    if ( i < values.count() ) {
        setFontSizeFromValue( values.at( i ), font, fontSizeAdjustment );
        ++i;
    }

    if ( i < values.count() ) {
        QString fam = values.at( i ).variant.toString();
        if ( !fam.isEmpty() )
            font->setFamily( fam );
    }
}

void ValueExtractor::extractFont( QFont *font, int *fontSizeAdjustment )
{
    if ( fontExtracted ) {
        *font = f;
        *fontSizeAdjustment = adjustment;
        return;
    }

    for ( int i = 0; i < declarations.count(); ++i ) {
        const Declaration &decl = declarations.at( i );
        if ( decl.values.isEmpty() )
            continue;
        const Value val = decl.values.first();
        switch ( decl.propertyId ) {
            case FontSize:
                setFontSizeFromValue( val, font, fontSizeAdjustment );
                break;
            case FontStyle:
                setFontStyleFromValue( val, font );
                break;
            case FontWeight:
                setFontWeightFromValue( val, font );
                break;
            case FontFamily:
                setFontFamilyFromValues( decl.values, font );
                break;
            case TextDecoration:
                setTextDecorationFromValues( decl.values, font );
                break;
            case Font:
                parseShorthandFontProperty( decl.values, font, fontSizeAdjustment );
                break;
            default:
                break;
        }
    }

    f = *font;
    adjustment = *fontSizeAdjustment;
    fontExtracted = true;
}

bool ValueExtractor::extractPalette( QColor *fg, QColor *sfg, QBrush *sbg, QBrush *abg )
{
    bool hit = false;
    for ( int i = 0; i < declarations.count(); ++i ) {
        const Declaration &decl = declarations.at( i );
        switch ( decl.propertyId ) {
            case Color:
                *fg = decl.colorValue();
                break;
            case QtSelectionForeground:
                *sfg = decl.colorValue();
                break;
            case QtSelectionBackground:
                *sbg = decl.brushValue();
                break;
            case QtAlternateBackground:
                *abg = decl.brushValue();
                break;
            default:
                continue;
        }
        hit = true;
    }
    return hit;
}

void ValueExtractor::extractFont()
{
    if ( fontExtracted )
        return;
    int dummy = -255;
    extractFont( &f, &dummy );
    fontExtracted = true;
}

///////////////////////////////////////////////////////////////////////////////
// Declaration
QColor Declaration::colorValue() const
{
    if ( values.count() != 1 )
        return QColor();

    return parseColorValue( values.first() );
}

QBrush Declaration::brushValue() const
{
    if ( values.count() != 1 )
        return QBrush();

    return parseBrushValue( values.first() );
}

bool Declaration::realValue( qreal *real, const char *unit ) const
{
    if ( values.count() != 1 )
        return false;
    const Value &v = values.first();
    if (( unit ) && ( v.type != Value::Length ) )
        return false;
    QString s = v.variant.toString();
    if ( unit ) {
        if ( !s.endsWith( QLatin1String( unit ), Qt::CaseInsensitive ) )
            return false;
        s.chop( qstrlen( unit ) );
    }
    bool ok = false;
    qreal val = s.toDouble( &ok );
    if ( ok )
        *real = val;
    return ok;
}

bool Declaration::intValue( int *i, const char *unit ) const
{
    if ( values.count() != 1 )
        return false;
    const Value &v = values.first();
    if (( unit ) && ( v.type != Value::Length ) )
        return false;
    QString s = v.variant.toString();
    if ( unit ) {
        if ( !s.endsWith( QLatin1String( unit ), Qt::CaseInsensitive ) )
            return false;
        s.chop( qstrlen( unit ) );
    }
    bool ok = false;
    int val = s.toInt( &ok );
    if ( ok )
        *i = val;
    return ok;
}

QRect Declaration::rectValue() const
{
    if ( values.count() != 1 )
        return QRect();
    const Value &v = values.first();
    if ( v.type != Value::Function )
        return QRect();
    QStringList func = v.variant.toStringList();
    if (( func.count() != 2 ) || ( func.first().compare( QLatin1String( "rect" ) ) != 0 ) )
        return QRect();
    QStringList args = func[1].split( " ", QString::SkipEmptyParts );
    if ( args.count() != 4 )
        return QRect();
    return QRect( args[0].toInt(), args[1].toInt(), args[2].toInt(), args[3].toInt() );
}

void Declaration::colorValues( QColor *c ) const
{
    int i;
    for ( i = 0; i < qMin( values.count(), 4 ); i++ )
        c[i] = parseColorValue( values.at( i ) );
    if ( i == 0 ) {
        c[0] = c[1] = c[2] = c[3] = QColor();
    } else if ( i == 1 ) {
        c[3] = c[2] = c[1] = c[0];
    } else if ( i == 2 ) {
        c[2] = c[0], c[3] = c[1];
    } else if ( i == 3 ) {
        c[3] = c[1];
    }
}

BorderStyle Declaration::styleValue() const
{
    if ( values.count() != 1 )
        return BorderStyle_None;
    return parseStyleValue( values.first() );
}

void Declaration::styleValues( BorderStyle *s ) const
{
    int i;
    for ( i = 0; i < qMin( values.count(), 4 ); i++ )
        s[i] = parseStyleValue( values.at( i ) );
    if ( i == 0 ) {
        s[0] = s[1] = s[2] = s[3] = BorderStyle_None;
    } else if ( i == 1 ) {
        s[3] = s[2] = s[1] = s[0];
    } else if ( i == 2 ) {
        s[2] = s[0], s[3] = s[1];
    } else if ( i == 3 ) {
        s[3] = s[1];
    }
}

Repeat Declaration::repeatValue() const
{
    if ( values.count() != 1 )
        return Repeat_Unknown;
    return static_cast<Repeat>( findKnownValue( values.first().variant.toString(),
                                repeats, NumKnownRepeats ) );
}

Origin Declaration::originValue() const
{
    if ( values.count() != 1 )
        return Origin_Unknown;
    return static_cast<Origin>( findKnownValue( values.first().variant.toString(),
                                origins, NumKnownOrigins ) );
}

PositionMode Declaration::positionValue() const
{
    if ( values.count() != 1 )
        return PositionMode_Unknown;
    return static_cast<PositionMode>( findKnownValue( values.first().variant.toString(),
                                      positions, NumKnownPositionModes ) );
}

QString Declaration::uriValue() const
{
    if (( values.isEmpty() ) || ( values.first().type != Value::Uri ) )
        return QString();
    return values.first().variant.toString();
}

Qt::Alignment Declaration::alignmentValue() const
{
    if (( values.isEmpty() ) || ( values.count() > 2 ) )
        return Qt::AlignLeft | Qt::AlignTop;

    return parseAlignment( values.constData(), values.count() );
}

void Declaration::borderImageValue( QString *image, int *cuts,
                                    TileMode *h, TileMode *v ) const
{
    *image = uriValue();
    for ( int i = 0; i < 4; i++ )
        cuts[i] = -1;
    *h = *v = TileMode_Stretch;

    if ( values.count() < 2 )
        return;

    if ( values.at( 1 ).type == Value::Number ) { // cuts!
        int i;
        for ( i = 0; i < qMin( values.count() - 1, 4 ); i++ ) {
            const Value& v = values.at( i + 1 );
            if ( v.type != Value::Number )
                break;
            cuts[i] = v.variant.toString().toInt();
        }
        if ( i == 0 ) {
            cuts[0] = cuts[1] = cuts[2] = cuts[3] = 0;
        } else if ( i == 1 ) {
            cuts[3] = cuts[2] = cuts[1] = cuts[0];
        } else if ( i == 2 ) {
            cuts[2] = cuts[0], cuts[3] = cuts[1];
        } else if ( i == 3 ) {
            cuts[3] = cuts[1];
        }
    }

    if ( values.last().type == Value::Identifier ) {
        *v = static_cast<TileMode>( findKnownValue( values.last().variant.toString(),
                                    tileModes, NumKnownTileModes ) );
    }
    if ( values[values.count() - 2].type == Value::Identifier ) {
        *h = static_cast<TileMode>
             ( findKnownValue( values[values.count()-2].variant.toString(),
                               tileModes, NumKnownTileModes ) );
    } else
        *h = *v;
}

///////////////////////////////////////////////////////////////////////////////
// Selector
int Selector::specificity() const
{
    int val = 0;
    for ( int i = 0; i < basicSelectors.count(); ++i ) {
        const BasicSelector &sel = basicSelectors.at( i );
        if ( !sel.elementName.isEmpty() )
            val += 1;

        val += (( sel.pseudos.count() + sel.attributeSelectors.count() ) * 0x10 );
        val += sel.ids.count() * 0x100;
    }
    return val;
}

QString Selector::pseudoElement() const
{
    const BasicSelector& bs = basicSelectors.last();
    if (( !bs.pseudos.isEmpty() ) && ( bs.pseudos.first().type == PseudoState_Unknown ) )
        return bs.pseudos.first().name;
    return QString();
}

int Selector::pseudoState() const
{
    const BasicSelector& bs = basicSelectors.last();
    if ( bs.pseudos.isEmpty() )
        return PseudoState_Unspecified;
    int state = PseudoState_Unknown;
    for ( int i = !pseudoElement().isEmpty(); i < bs.pseudos.count(); i++ ) {
        if ( bs.pseudos.at( i ).type == PseudoState_Unknown )
            return PseudoState_Unknown;
        state |= bs.pseudos.at( i ).type;
    }
    return state;
}

///////////////////////////////////////////////////////////////////////////////
// StyleSelector
StyleSelector::~StyleSelector()
{
}

QStringList StyleSelector::nodeIds( NodePtr node ) const
{
    return QStringList( attribute( node, QLatin1String( "id" ) ) );
}

bool StyleSelector::selectorMatches( const Selector &selector, NodePtr node )
{
    if ( selector.basicSelectors.isEmpty() )
        return false;

    if ( selector.basicSelectors.first().relationToNext == BasicSelector::NoRelation ) {
        if ( selector.basicSelectors.count() != 1 )
            return false;
        return basicSelectorMatches( selector.basicSelectors.first(), node );
    }
    if ( selector.basicSelectors.count() <= 1 )
        return false;

    int i = selector.basicSelectors.count() - 1;
    node = duplicateNode( node );
    bool match = true;

    BasicSelector sel = selector.basicSelectors.at( i );
    do {
        match = basicSelectorMatches( sel, node );
        if ( !match ) {
            if (( sel.relationToNext == BasicSelector::MatchNextSelectorIfParent )
                    || ( i == selector.basicSelectors.count() - 1 ) ) // first element must always match!
                break;
        }

        if (( match ) || ( sel.relationToNext != BasicSelector::MatchNextSelectorIfAncestor ) )
            --i;

        if ( i < 0 )
            break;

        sel = selector.basicSelectors.at( i );
        if (( sel.relationToNext == BasicSelector::MatchNextSelectorIfAncestor )
                || ( sel.relationToNext == BasicSelector::MatchNextSelectorIfParent ) ) {
            NodePtr nextParent = parentNode( node );
            freeNode( node );
            node = nextParent;
        } else if ( sel.relationToNext == BasicSelector::MatchNextSelectorIfPreceeds ) {
            NodePtr previousSibling = previousSiblingNode( node );
            freeNode( node );
            node = previousSibling;
        }
        if ( isNullNode( node ) ) {
            match = false;
            break;
        }
    } while (( i >= 0 ) && (( match ) || ( sel.relationToNext ==
                                           BasicSelector::MatchNextSelectorIfAncestor ) ) );

    freeNode( node );

    return match;
}

bool StyleSelector::basicSelectorMatches( const BasicSelector &sel, NodePtr node )
{
    if ( !sel.attributeSelectors.isEmpty() ) {
        if ( !hasAttributes( node ) )
            return false;

        for ( int i = 0; i < sel.attributeSelectors.count(); ++i ) {
            const QCss::AttributeSelector &a = sel.attributeSelectors.at( i );
            if ( !hasAttribute( node, a.name ) )
                return false;

            const QString attrValue = attribute( node, a.name );

            if ( a.valueMatchCriterium == QCss::AttributeSelector::MatchContains ) {

                QStringList lst = attrValue.split( QLatin1Char( ' ' ) );
                if ( !lst.contains( a.value ) )
                    return false;
            } else if ((( a.valueMatchCriterium == QCss::AttributeSelector::MatchEqual ) &&
                        ( attrValue != a.value ) ) || (( a.valueMatchCriterium ==
                                                         QCss::AttributeSelector::MatchBeginsWith ) &&
                                                       ( !attrValue.startsWith( a.value ) ) )
                      )
                return false;
        }
    }

    if (( !sel.elementName.isEmpty() )
            && ( !nodeNameEquals( node, sel.elementName ) ) )
        return false;

    if ( !sel.ids.isEmpty()
            && sel.ids != nodeIds( node ) )
        return false;

    return true;
}

static inline bool qcss_selectorLessThan( const QPair<int, QCss::StyleRule> &lhs,
        const QPair<int, QCss::StyleRule> &rhs )
{
    return lhs.first < rhs.first;
}

void StyleSelector::matchRules( NodePtr node, const QVector<StyleRule> &rules,
                                StyleSheetOrigin origin, int depth, QVector<QPair<int, StyleRule> > *weightedRules )
{
    for ( int i = 0; i < rules.count(); ++i ) {
        const StyleRule &rule = rules.at( i );
        for ( int j = 0; j < rule.selectors.count(); ++j ) {
            const Selector& selector = rule.selectors.at( j );
            if ( selectorMatches( selector, node ) ) {
                QPair<int, StyleRule> weightedRule;
                weightedRule.first = selector.specificity()
                                     + (( origin == StyleSheetOrigin_Inline ) * 0x1000 * depth );
                weightedRule.second.selectors.append( selector );
                weightedRule.second.declarations = rule.declarations;
                weightedRules->append( weightedRule );
            }
        }
    }
}

#if 0
static void printDeclarations( const QVector<QPair<int, StyleRule> >& decls )
{
    for ( int i = 0; i < decls.count(); i++ ) {
        const StyleRule& rule = decls.at( i ).second;
        qDebug() << rule.declarations.first().property
        << rule.declarations.first().values.first().variant.toString();
    }
}
#endif

// Returns style rules that are in ascending order of specificity
// Each of the StyleRule returned will contain exactly one Selector
QVector<StyleRule> StyleSelector::styleRulesForNode( NodePtr node )
{
    QVector<StyleRule> rules;
    if ( styleSheets.isEmpty() )
        return rules;

    QVector<QPair<int, StyleRule> > weightedRules; // (spec, rule) that will be sorted below
    for ( int sheetIdx = 0; sheetIdx < styleSheets.count(); ++sheetIdx ) {
        const StyleSheet &styleSheet = styleSheets.at( sheetIdx );

        matchRules( node, styleSheet.styleRules, styleSheet.origin, styleSheet.depth, &weightedRules );
        if ( !medium.isEmpty() ) {
            for ( int i = 0; i < styleSheet.mediaRules.count(); ++i ) {
                if ( styleSheet.mediaRules.at( i ).media.contains( medium, Qt::CaseInsensitive ) ) {
                    matchRules( node, styleSheet.mediaRules.at( i ).styleRules, styleSheet.origin,
                                styleSheet.depth, &weightedRules );
                }
            }
        }
    }

    qStableSort( weightedRules.begin(), weightedRules.end(), qcss_selectorLessThan );

    for ( int j = 0; j < weightedRules.count(); j++ )
        rules += weightedRules.at( j ).second;

    return rules;
}

// for qtexthtmlparser which requires just the declarations with Enabled state
// and without pseudo elements
QVector<Declaration> StyleSelector::declarationsForNode( NodePtr node )
{
    QVector<Declaration> decls;
    QVector<StyleRule> rules = styleRulesForNode( node );
    for ( int i = 0; i < rules.count(); i++ ) {
        const Selector& selector = rules.at( i ).selectors.at( 0 );
        if ( !selector.pseudoElement().isEmpty() ) // skip rules with pseudo elements
            continue;
        int pseudoState = selector.pseudoState();
        if (( pseudoState == PseudoState_Enabled ) || ( pseudoState == PseudoState_Unspecified ) )
            decls += rules.at( i ).declarations;
    }
    return decls;
}

static inline bool isHexDigit( const char c )
{
    return (( c >= '0' && c <= '9' )
            || ( c >= 'a' && c <= 'f' )
            || ( c >= 'A' && c <= 'F' ) );
}

QString Scanner::preprocess( const QString &input )
{
    QString output = input;

    int i = 0;
    while ( i < output.size() ) {
        if ( output.at( i ) == QLatin1Char( '\\' ) ) {

            ++i;
            // test for unicode hex escape
            int hexCount = 0;
            const int hexStart = i;
            while (( i < output.size() ) && ( isHexDigit( output.at( i ).toLatin1() ) )
                    && ( hexCount < 7 ) ) {
                ++hexCount;
                ++i;
            }
            if ( hexCount == 0 )
                continue;

            hexCount = qMin( hexCount, 6 );
            bool ok = false;
            ushort code = output.mid( hexStart, hexCount ).toUShort( &ok, 16 );
            if ( ok ) {
                output.replace( hexStart - 1, hexCount + 1, QChar( code ) );
                i = hexStart;
            } else {
                i = hexStart;
            }
        } else {
            ++i;
        }
    }
    return output;
}

QVector<Symbol> Scanner::scan( const QString &preprocessedInput )
{
    QVector<Symbol> symbols;
    QCssScanner_Generated scanner( preprocessedInput );
    Symbol sym;
    int tok = scanner.lex();
    while ( tok != -1 ) {
        sym.token = static_cast<TokenType>( tok );
        sym.text = scanner.input;
        sym.start = scanner.lexemStart;
        sym.len = scanner.lexemLength;
        symbols.append( sym );
        tok = scanner.lex();
    }
    return symbols;
}

QString Symbol::lexem() const
{
    QString result;
    if ( len > 0 )
        result.reserve( len );
    for ( int i = 0; i < len; ++i ) {
        if (( text.at( start + i ) == QLatin1Char( '\\' ) ) && ( i < len - 1 ) )
            ++i;
        result.append( text.at( start + i ) );
    }
    return result;
}

Parser::Parser( const QString &css )
{
    symbols = Scanner::scan( Scanner::preprocess( css ) );
    index = 0;
    errorIndex = -1;
}

bool Parser::parse( StyleSheet *styleSheet )
{
    if ( testTokenAndEndsWith( ATKEYWORD_SYM, QLatin1String( "charset" ) ) ) {
        if ( !next( STRING ) ) return false;
        if ( !next( SEMICOLON ) ) return false;
    }

    while ( test( S ) || test( CDO ) || test( CDC ) ) {};

    while ( testImport() ) {
        ImportRule rule;
        if ( !parseImport( &rule ) ) return false;
        styleSheet->importRules.append( rule );
        while ( test( S ) || test( CDO ) || test( CDC ) ) {};
    }

    do {
        if ( testMedia() ) {
            MediaRule rule;
            if ( !parseMedia( &rule ) ) return false;
            styleSheet->mediaRules.append( rule );
        } else if ( testPage() ) {
            PageRule rule;
            if ( !parsePage( &rule ) ) return false;
            styleSheet->pageRules.append( rule );
        } else if ( testRuleset() ) {
            StyleRule rule;
            if ( !parseRuleset( &rule ) ) return false;
            styleSheet->styleRules.append( rule );
        } else if ( test( ATKEYWORD_SYM ) ) {
            if ( !until( RBRACE ) ) return false;
        } else if ( hasNext() ) {
            return false;
        }
        while ( test( S ) || test( CDO ) || test( CDC ) ) {};
    } while ( hasNext() );
    return true;
}

Symbol Parser::errorSymbol()
{
    if ( errorIndex == -1 ) return Symbol();
    return symbols.at( errorIndex );
}

static inline void removeOptionalQuotes( QString *str )
{
    if ( !str->startsWith( QLatin1Char( '\'' ) )
            && !str->startsWith( QLatin1Char( '\"' ) ) )
        return;
    str->remove( 0, 1 );
    str->chop( 1 );
}

bool Parser::parseImport( ImportRule *importRule )
{
    skipSpace();

    if ( test( STRING ) ) {
        importRule->href = lexem();
    } else {
        if ( !testAndParseUri( &importRule->href ) ) return false;
    }
    removeOptionalQuotes( &importRule->href );

    skipSpace();

    if ( testMedium() ) {
        if ( !parseMedium( &importRule->media ) ) return false;

        while ( test( COMMA ) ) {
            skipSpace();
            if ( !parseNextMedium( &importRule->media ) ) return false;
        }
    }

    if ( !next( SEMICOLON ) ) return false;

    skipSpace();
    return true;
}

bool Parser::parseMedia( MediaRule *mediaRule )
{
    do {
        skipSpace();
        if ( !parseNextMedium( &mediaRule->media ) )
            return false;
    } while ( test( COMMA ) );

    if ( !next( LBRACE ) )
        return false;
    skipSpace();

    while ( testRuleset() ) {
        StyleRule rule;
        if ( !parseRuleset( &rule ) )
            return false;
        mediaRule->styleRules.append( rule );
    }

    if ( !next( RBRACE ) )
        return false;
    skipSpace();
    return true;
}

bool Parser::parseMedium( QStringList *media )
{
    media->append( lexem() );
    skipSpace();
    return true;
}

bool Parser::parsePage( PageRule *pageRule )
{
    skipSpace();

    if ( testPseudoPage() )
        if ( !parsePseudoPage( &pageRule->selector ) )
            return false;

    skipSpace();
    if ( !next( LBRACE ) )
        return false;

    do {
        skipSpace();
        Declaration decl;
        if ( !parseNextDeclaration( &decl ) )
            return false;
        if ( !decl.isEmpty() )
            pageRule->declarations.append( decl );
    } while ( test( SEMICOLON ) );

    if ( !next( RBRACE ) )
        return false;
    skipSpace();
    return true;
}

bool Parser::parsePseudoPage( QString *selector )
{
    if ( !next( IDENT ) )
        return false;
    *selector = lexem();
    return true;
}

bool Parser::parseNextOperator( Value *value )
{
    if ( !hasNext() )
        return true;
    switch ( next() ) {
        case SLASH:
            value->type = Value::TermOperatorSlash;
            skipSpace();
            break;
        case COMMA:
            value->type = Value::TermOperatorComma;
            skipSpace();
            break;
        default:
            prev();
            break;
    }
    return true;
}

bool Parser::parseCombinator( BasicSelector::Relation *relation )
{
    *relation = BasicSelector::NoRelation;
    if ( lookup() == S ) {
        *relation = BasicSelector::MatchNextSelectorIfAncestor;
        skipSpace();
    } else {
        prev();
    }
    if ( test( PLUS ) ) {
        *relation = BasicSelector::MatchNextSelectorIfPreceeds;
    } else if ( test( GREATER ) ) {
        *relation = BasicSelector::MatchNextSelectorIfParent;
    }
    skipSpace();
    return true;
}

bool Parser::parseProperty( Declaration *decl )
{
    decl->property = lexem();
    decl->propertyId = static_cast<Property>( findKnownValue( decl->property, properties, NumProperties ) );
    skipSpace();
    return true;
}

bool Parser::parseRuleset( StyleRule *styleRule )
{
    Selector sel;
    if ( !parseSelector( &sel ) )
        return false;
    styleRule->selectors.append( sel );

    while ( test( COMMA ) ) {
        skipSpace();
        Selector sel;
        if ( !parseNextSelector( &sel ) )
            return false;
        styleRule->selectors.append( sel );
    }

    skipSpace();
    if ( !next( LBRACE ) )
        return false;
    const int declarationStart = index;

    do {
        skipSpace();
        Declaration decl;
        const int rewind = index;
        if ( !parseNextDeclaration( &decl ) ) {
            index = rewind;
            const bool foundSemicolon = until( SEMICOLON );
            const int semicolonIndex = index;

            index = declarationStart;
            const bool foundRBrace = until( RBRACE );

            if ( foundSemicolon && semicolonIndex < index ) {
                decl = Declaration();
                index = semicolonIndex - 1;
            } else {
                skipSpace();
                return foundRBrace;
            }
        }
        if ( !decl.isEmpty() )
            styleRule->declarations.append( decl );
    } while ( test( SEMICOLON ) );

    if ( !next( RBRACE ) )
        return false;
    skipSpace();
    return true;
}

bool Parser::parseSelector( Selector *sel )
{
    BasicSelector basicSel;
    if ( !parseSimpleSelector( &basicSel ) )
        return false;
    while ( testCombinator() ) {
        if ( !parseCombinator( &basicSel.relationToNext ) )
            return false;

        if ( !testSimpleSelector() )
            break;
        sel->basicSelectors.append( basicSel );

        basicSel = BasicSelector();
        if ( !parseSimpleSelector( &basicSel ) )
            return false;
    }
    sel->basicSelectors.append( basicSel );
    return true;
}

bool Parser::parseSimpleSelector( BasicSelector *basicSel )
{
    int minCount = 0;
    if ( lookupElementName() ) {
        if ( !parseElementName( &basicSel->elementName ) )
            return false;
    } else {
        prev();
        minCount = 1;
    }
    bool onceMore;
    int count = 0;
    do {
        onceMore = false;
        if ( test( HASH ) ) {
            QString id = lexem();
            // chop off leading #
            id.remove( 0, 1 );
            basicSel->ids.append( id );
            onceMore = true;
        } else if ( testClass() ) {
            onceMore = true;
            AttributeSelector a;
            a.name = QLatin1String( "class" );
            a.valueMatchCriterium = AttributeSelector::MatchContains;
            if ( !parseClass( &a.value ) )
                return false;
            basicSel->attributeSelectors.append( a );
        } else if ( testAttrib() ) {
            onceMore = true;
            AttributeSelector a;
            if ( !parseAttrib( &a ) )
                return false;
            basicSel->attributeSelectors.append( a );
        } else if ( testPseudo() ) {
            onceMore = true;
            Pseudo ps;
            if ( !parsePseudo( &ps ) )
                return false;
            basicSel->pseudos.append( ps );
        }
        if ( onceMore ) ++count;
    } while ( onceMore );
    return count >= minCount;
}

bool Parser::parseClass( QString *name )
{
    if ( !next( IDENT ) )
        return false;
    *name = lexem();
    return true;
}

bool Parser::parseElementName( QString *name )
{
    switch ( lookup() ) {
        case STAR:
            name->clear();
            break;
        case IDENT:
            *name = lexem();
            break;
        default:
            return false;
    }
    return true;
}

bool Parser::parseAttrib( AttributeSelector *attr )
{
    skipSpace();
    if ( !next( IDENT ) )
        return false;
    attr->name = lexem();
    skipSpace();

    if ( test( EQUAL ) ) {
        attr->valueMatchCriterium = AttributeSelector::MatchEqual;
    } else if ( test( INCLUDES ) ) {
        attr->valueMatchCriterium = AttributeSelector::MatchContains;
    } else if ( test( DASHMATCH ) ) {
        attr->valueMatchCriterium = AttributeSelector::MatchBeginsWith;
    } else {
        return next( RBRACKET );
    }

    skipSpace();

    if ( !test( IDENT ) && !test( STRING ) )
        return false;
    attr->value = unquotedLexem();

    skipSpace();
    return next( RBRACKET );
}

bool Parser::parsePseudo( Pseudo *pseudo )
{
    test( COLON );
    if ( test( IDENT ) ) {
        pseudo->name = lexem();
        pseudo->type = static_cast<PseudoState>( findKnownValue( pseudo->name, pseudos, NumPseudos ) );
        return true;
    }
    if ( !next( FUNCTION ) )
        return false;
    pseudo->function = lexem();
    // chop off trailing parenthesis
    pseudo->function.chop( 1 );
    skipSpace();
    if ( !test( IDENT ) )
        return false;
    pseudo->name = lexem();
    skipSpace();
    return next( RPAREN );
}

bool Parser::parseNextDeclaration( Declaration *decl )
{
    if ( !testProperty() )
        return true; // not an error!
    if ( !parseProperty( decl ) )
        return false;
    if ( !next( COLON ) )
        return false;
    skipSpace();
    if ( !parseNextExpr( &decl->values ) )
        return false;
    if ( testPrio() )
        if ( !parsePrio( decl ) )
            return false;
    return true;
}

bool Parser::testPrio()
{
    const int rewind = index;
    if ( !test( EXCLAMATION_SYM ) )
        return false;
    skipSpace();
    if ( !test( IDENT ) ) {
        index = rewind;
        return false;
    }
    if ( lexem().compare( QLatin1String( "important" ), Qt::CaseInsensitive ) != 0 ) {
        index = rewind;
        return false;
    }
    return true;
}

bool Parser::parsePrio( Declaration *declaration )
{
    declaration->important = true;
    skipSpace();
    return true;
}

bool Parser::parseExpr( QVector<Value> *values )
{
    Value val;
    if ( !parseTerm( &val ) )
        return false;
    values->append( val );
    bool onceMore;
    do {
        onceMore = false;
        val = Value();
        if ( !parseNextOperator( &val ) )
            return false;
        if ( val.type != QCss::Value::Unknown )
            values->append( val );
        if ( testTerm() ) {
            onceMore = true;
            val = Value();
            if ( !parseTerm( &val ) )
                return false;
            values->append( val );
        }
    } while ( onceMore );
    return true;
}

bool Parser::testTerm()
{
    return ( test( PLUS ) || test( MINUS )
             || test( NUMBER )
             || test( PERCENTAGE )
             || test( LENGTH )
             || test( STRING )
             || test( IDENT )
             || testHexColor()
             || testFunction() );
}

bool Parser::parseTerm( Value *value )
{
    QString str = lexem();
    bool haveUnary = false;
    if ( lookup() == PLUS || lookup() == MINUS ) {
        haveUnary = true;
        if ( !hasNext() )
            return false;
        next();
        str += lexem();
    }

    value->variant = str;
    value->type = QCss::Value::String;
    switch ( lookup() ) {
        case NUMBER:
            value->type = Value::Number;
            value->variant.convert( QVariant::Double );
            break;
        case PERCENTAGE:
            value->type = Value::Percentage;
            str.chop( 1 ); // strip off %
            value->variant = str;
            break;
        case LENGTH:
            value->type = Value::Length;
            break;

        case STRING:
            if ( haveUnary ) return false;
            value->type = Value::String;
            str.chop( 1 );
            str.remove( 0, 1 );
            value->variant = str;
            break;
        case IDENT: {
            if ( haveUnary ) return false;
            value->type = Value::Identifier;
            const int id = findKnownValue( str, values, NumKnownValues );
            if ( id != 0 ) {
                value->type = Value::KnownIdentifier;
                value->variant = id;
            }
            break;
        }
        default: {
            if ( haveUnary ) return false;
            prev();
            if ( testHexColor() ) {
                QColor col;
                if ( !parseHexColor( &col ) ) return false;
                value->type = Value::Color;
                value->variant = col;
            } else if ( testFunction() ) {
                QString name, args;
                if ( !parseFunction( &name, &args ) ) return false;
                if ( name == QLatin1String( "url" ) ) {
                    value->type = Value::Uri;
                    removeOptionalQuotes( &args );
                    value->variant = args;
                } else {
                    value->type = Value::Function;
                    value->variant = QStringList() << name << args;
                }
            } else {
                return recordError();
            }
            return true;
        }
    }
    skipSpace();
    return true;
}

bool Parser::parseFunction( QString *name, QString *args )
{
    *name = lexem();
    name->chop( 1 );
    skipSpace();
    const int start = index;
    if ( !until( RPAREN ) ) return false;
    for ( int i = start; i < index - 1; ++i )
        args->append( symbols.at( i ).lexem() );
    /*
    if (!nextExpr(&arguments)) return false;
    if (!next(RPAREN)) return false;
    */
    skipSpace();
    return true;
}

bool Parser::parseHexColor( QColor *col )
{
    col->setNamedColor( lexem() );
    if ( !col->isValid() ) return false;
    skipSpace();
    return true;
}

bool Parser::testAndParseUri( QString *uri )
{
    const int rewind = index;
    if ( !testFunction() ) return false;

    QString name, args;
    if ( !parseFunction( &name, &args ) ) {
        index = rewind;
        return false;
    }
    if ( name.toLower() != QLatin1String( "url" ) ) {
        index = rewind;
        return false;
    }
    *uri = args;
    removeOptionalQuotes( uri );
    return true;
}

bool Parser::testSimpleSelector()
{
    return ( testElementName()
             || ( test( HASH ) )
             || testClass()
             || testAttrib()
             || testPseudo() );
}

bool Parser::next( TokenType t )
{
    if ( hasNext() && next() == t )
        return true;
    return recordError();
}

bool Parser::test( TokenType t )
{
    if ( index >= symbols.count() )
        return false;
    if ( symbols.at( index ).token == t ) {
        ++index;
        return true;
    }
    return false;
}

QString Parser::unquotedLexem() const
{
    QString s = lexem();
    if ( lookup() == STRING ) {
        s.chop( 1 );
        s.remove( 0, 1 );
    }
    return s;
}

QString Parser::lexemUntil( TokenType t )
{
    QString lexem;
    while ( hasNext() && next() != t )
        lexem += symbol().lexem();
    return lexem;
}

bool Parser::until( TokenType target, TokenType target2 )
{
    int braceCount = 0;
    int brackCount = 0;
    int parenCount = 0;
    if ( index ) {
        switch ( symbols.at( index - 1 ).token ) {
            case LBRACE:
                ++braceCount;
                break;
            case LBRACKET:
                ++brackCount;
                break;
            case FUNCTION:
            case LPAREN:
                ++parenCount;
                break;
            default:
                ;
        }
    }
    while ( index < symbols.size() ) {
        TokenType t = symbols.at( index++ ).token;
        switch ( t ) {
            case LBRACE:
                ++braceCount;
                break;
            case RBRACE:
                --braceCount;
                break;
            case LBRACKET:
                ++brackCount;
                break;
            case RBRACKET:
                --brackCount;
                break;
            case FUNCTION:
            case LPAREN:
                ++parenCount;
                break;
            case RPAREN:
                --parenCount;
                break;
            default:
                break;
        }
        if ((( t == target ) || ( target2 != NONE && t == target2 ) )
                && ( braceCount <= 0 )
                && ( brackCount <= 0 )
                && ( parenCount <= 0 ) )
            return true;

        if (( braceCount < 0 ) || ( brackCount < 0 ) || ( parenCount < 0 ) ) {
            --index;
            break;
        }
    }
    return false;
}

bool Parser::testTokenAndEndsWith( TokenType t, const QLatin1String &str )
{
    if ( !test( t ) ) return false;
    if ( !lexem().endsWith( str, Qt::CaseInsensitive ) ) {
        prev();
        return false;
    }
    return true;
}


