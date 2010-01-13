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
 
#include "standalonenoteeditor.h"

#include "standalonenoteitem.h"
#include "ui_standalonenoteeditor.h"

using namespace StickyNotes;

namespace StickyNotes {

/* StandaloneNoteEditorPrivate */

class StandaloneNoteEditorPrivate
{
Q_DECLARE_PUBLIC(StandaloneNoteEditor)

public:
	StandaloneNoteEditorPrivate(StandaloneNoteEditor *_q);
	~StandaloneNoteEditorPrivate(void);

private:
	StandaloneNoteEditor *q_ptr;
	StandaloneNoteItem   *item;
	Ui::StandaloneNoteEditor *ui;
};

} // namespace StickyNotes

StandaloneNoteEditorPrivate::StandaloneNoteEditorPrivate(StandaloneNoteEditor *_q)
: q_ptr(_q), item(0), ui(new Ui::StandaloneNoteEditor)
{
	ui->setupUi(_q);
}

StandaloneNoteEditorPrivate::~StandaloneNoteEditorPrivate(void)
{
	delete ui;
}

/* StandaloneNoteEditor */

StandaloneNoteEditor::StandaloneNoteEditor(StandaloneNoteItem &_item,
    QWidget *_parent, Qt::WindowFlags _f)
: QDialog(_parent, _f), d_ptr(new StandaloneNoteEditorPrivate(this))
{
	Q_D(StandaloneNoteEditor);

	d->item = &_item;

	connect(d->item, SIGNAL(appliedContent(const QString &)),
	    this, SLOT(on_item_appliedContent(const QString &)));
	connect(d->item, SIGNAL(appliedSubject(const QString &)),
	    this, SLOT(on_item_appliedSubject(const QString &)));
	connect(d->item, SIGNAL(bound(void)),
	    this, SLOT(on_item_bound(void)));
	connect(d->item, SIGNAL(unbound(void)),
	    this, SLOT(reject(void)));
}

StandaloneNoteEditor::~StandaloneNoteEditor(void)
{
	delete d_ptr;
}

void
StandaloneNoteEditor::accept(void)
{
	Q_D(StandaloneNoteEditor);

	d->item->setContent(d->ui->contentEdit->toHtml());
	d->item->setSubject(d->ui->subjectEdit->text());

	QDialog::accept();
}

void
StandaloneNoteEditor::on_boldBtn_clicked(void)
{
	Q_D(StandaloneNoteEditor);

	if (QFont::Normal == d->ui->contentEdit->fontWeight())
		d->ui->contentEdit->setFontWeight(QFont::Bold);
	else
		d->ui->contentEdit->setFontWeight(QFont::Normal);
}

void
StandaloneNoteEditor::on_centerJustifyBtn_clicked(void)
{
	Q_D(StandaloneNoteEditor);

	d->ui->contentEdit->setAlignment(Qt::AlignCenter);
}

void
StandaloneNoteEditor::on_fontComboBox_currentFontChanged(const QFont &_font)
{
	Q_D(StandaloneNoteEditor);
	
	d->ui->contentEdit->setFontFamily(_font.family());
}

void
StandaloneNoteEditor::on_fontSizeBox_valueChanged(int _i)
{
	Q_D(StandaloneNoteEditor);
	
	d->ui->contentEdit->setFontPointSize(_i);
}

void
StandaloneNoteEditor::on_italicBtn_clicked(void)
{
	Q_D(StandaloneNoteEditor);

	d->ui->contentEdit->setFontItalic(!d->ui->contentEdit->fontItalic());
}

void
StandaloneNoteEditor::on_item_appliedContent(const QString &_content)
{
	d_func()->ui->contentEdit->setHtml(_content);
}

void
StandaloneNoteEditor::on_item_appliedSubject(const QString &_subject)
{
	d_func()->ui->subjectEdit->setText(_subject);
}

void
StandaloneNoteEditor::on_item_bound(void)
{
	Q_D(StandaloneNoteEditor);

	d->ui->contentEdit->setHtml(d->item->content());
	d->ui->subjectEdit->setText(d->item->subject());
}

void
StandaloneNoteEditor::on_leftJustifyBtn_clicked(void)
{
	Q_D(StandaloneNoteEditor);

	d->ui->contentEdit->setAlignment(Qt::AlignLeft);
}

void
StandaloneNoteEditor::on_rightJustifyBtn_clicked(void)
{
	Q_D(StandaloneNoteEditor);

	d->ui->contentEdit->setAlignment(Qt::AlignRight);
}

void
StandaloneNoteEditor::on_underlineBtn_clicked(void)
{
	Q_D(StandaloneNoteEditor);

	d->ui->contentEdit->setFontUnderline(!d->ui->contentEdit->fontUnderline());
}

#include "include/standalonenoteeditor.moc"

