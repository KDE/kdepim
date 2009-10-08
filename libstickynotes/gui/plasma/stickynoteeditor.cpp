/*-
 * Copyright 2009 KDAB and Guillermo A. Amaral B. <gamaral@amaral.com.mx>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
 
#include "stickynoteeditor.h"

#include "stickynoteitem.h"

/* StickyNoteEditor */

StickyNoteEditor::StickyNoteEditor(StickyNoteItem &_item,
    QWidget *_parent, Qt::WindowFlags _f)
: QDialog(_parent, _f), m_item(&_item)
{
	connect(m_item, SIGNAL(appliedContent(const QString &)),
	    this, SLOT(on_item_appliedContent(const QString &)));
	connect(m_item, SIGNAL(appliedSubject(const QString &)),
	    this, SLOT(on_item_appliedSubject(const QString &)));
	connect(m_item, SIGNAL(bound(void)),
	    this, SLOT(on_item_bound(void)));
	connect(m_item, SIGNAL(unbound(void)),
	    this, SLOT(reject(void)));

	setupUi(this);
}

StickyNoteEditor::~StickyNoteEditor(void)
{
}

void
StickyNoteEditor::accept(void)
{
	m_item->setContent(contentEdit->toHtml());
	m_item->setSubject(subjectEdit->text());

	QDialog::accept();
}

void
StickyNoteEditor::on_boldBtn_clicked(void)
{
	if (QFont::Normal == contentEdit->fontWeight())
		contentEdit->setFontWeight(QFont::Bold);
	else
		contentEdit->setFontWeight(QFont::Normal);
}

void
StickyNoteEditor::on_centerJustifyBtn_clicked(void)
{
	contentEdit->setAlignment(Qt::AlignCenter);
}

void
StickyNoteEditor::on_fontComboBox_currentFontChanged(const QFont &_font)
{
	contentEdit->setFontFamily(_font.family());
}

void
StickyNoteEditor::on_fontSizeBox_valueChanged(int _i)
{
	contentEdit->setFontPointSize(_i);
}

void
StickyNoteEditor::on_italicBtn_clicked(void)
{
	contentEdit->setFontItalic(!contentEdit->fontItalic());
}

void
StickyNoteEditor::on_item_appliedContent(const QString &_content)
{
	contentEdit->setHtml(_content);
}

void
StickyNoteEditor::on_item_appliedSubject(const QString &_subject)
{
	subjectEdit->setText(_subject);
}

void
StickyNoteEditor::on_item_bound(void)
{
	contentEdit->setHtml(m_item->content());
	subjectEdit->setText(m_item->subject());
}

void
StickyNoteEditor::on_leftJustifyBtn_clicked(void)
{
	contentEdit->setAlignment(Qt::AlignLeft);
}

void
StickyNoteEditor::on_rightJustifyBtn_clicked(void)
{
	contentEdit->setAlignment(Qt::AlignRight);
}

void
StickyNoteEditor::on_underlineBtn_clicked(void)
{
	contentEdit->setFontUnderline(!contentEdit->fontUnderline());
}

