/*
   Copyright (C) 2015-2016 Montel Laurent <montel@kde.org>

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

#include "sieveeditoremptytabwidgetlabel.h"
#include <KLocalizedString>

SieveEditorEmptyTabWidgetLabel::SieveEditorEmptyTabWidgetLabel(QWidget *parent)
    : QLabel(parent)
{
    init();
}

SieveEditorEmptyTabWidgetLabel::~SieveEditorEmptyTabWidgetLabel()
{

}

void SieveEditorEmptyTabWidgetLabel::init()
{
    //TODO improve text
    const QString placeholderText = QStringLiteral("<html><body style=\"color:#909090; font-size:14px\">"
                                    "<div align='center'>"
                                    "<div style=\"font-size:20px\">%1</div>"
                                    "<div></div>"
                                    "<li>%2"
                                    "<div style=\"font-size:20px\">%3</div>"
                                    "<div></div>"
                                    "<li>%4"
                                    "<div style=\"font-size:20px\">%5</div>"
                                    "<li>%6"
                                    "<div></div>"
                                    "</div>"
                                    "</body></html>").arg(i18n("Debug a script:"), i18nc("These action in from menu tools submenu debug sieve script", "Tools > Debug Sieve Script"),
                                            i18n("Create Rules Graphically:"), i18nc("Action is from menu tools, submenu autogenerate script", "Tools > Autogenerate script"),
                                            i18n("Import script:"), i18nc("Action is from file menu, import submenu", "File > Import"));
    setText(placeholderText);
}
