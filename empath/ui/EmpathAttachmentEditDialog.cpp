/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma implementation "EmpathAttachmentEditDialog.h"
#endif

// KDE includes
#include <kfiledialog.h>
#include <kquickhelp.h>
#include <klocale.h>
#include <kapp.h>

// Local includes
#include "EmpathUIUtils.h"
#include "EmpathDefines.h"
#include "EmpathAttachmentEditDialog.h"
#include "RikGroupBox.h"

EmpathAttachmentEditDialog::EmpathAttachmentEditDialog(
		QWidget * parent,
		const char * name)
	:	QDialog(parent, name, true)
{
	empathDebug("ctor");
	
	setCaption(i18n("Attachment Edit - ") + kapp->getCaption());
	
	QLineEdit	tempLineEdit((QWidget *)0);
	Q_UINT32 h	= tempLineEdit.sizeHint().height();
	
	buttonBox_	= new KButtonBox(this);
	CHECK_PTR(buttonBox_);

	buttonBox_->setMaximumHeight(h);
	
	// Bottom button group
	pb_help_	= buttonBox_->addButton(i18n("&Help"));	
	buttonBox_->addStretch();
	pb_OK_		= buttonBox_->addButton(i18n("&OK"));
	pb_OK_->setDefault(true);
	pb_cancel_	= buttonBox_->addButton(i18n("&Cancel"));
	
	buttonBox_->layout();

	QObject::connect(pb_OK_, SIGNAL(clicked()),
			this, SLOT(s_OK()));
	
	QObject::connect(pb_cancel_, SIGNAL(clicked()),
			this, SLOT(s_cancel()));
	
	QObject::connect(pb_help_, SIGNAL(clicked()),
			this, SLOT(s_help()));

	rgb_main_	= new RikGroupBox(QString::null, 8, this, "rgb_main");
	CHECK_PTR(rgb_main_);
	
	w_main_		= new QWidget(rgb_main_, "w_main");
	CHECK_PTR(w_main_);
	
	rgb_main_->setWidget(w_main_);

	rgb_encoding_= new RikGroupBox(i18n("Encoding"), 8, w_main_, "rgb_enc");
	CHECK_PTR(rgb_encoding_);
	
	w_encoding_	= new QWidget(rgb_encoding_, "w_enc");
	CHECK_PTR(w_encoding_);
	
	rgb_encoding_->setWidget(w_encoding_);
	
	l_filename_		= new QLabel(i18n("Filename"), w_main_, "l_filename");
	CHECK_PTR(l_filename_);

	l_description_	= new QLabel(i18n("Description"), w_main_, "l_descrip");
	CHECK_PTR(l_description_);
	
	le_filename_	= new QLineEdit(w_main_, "le_filename");
	CHECK_PTR(le_filename_);
	
	KQuickHelp::add(le_filename_, i18n(
			"Pick a file to attach !"));
	
	le_description_	= new QLineEdit(w_main_, "le_description");
	CHECK_PTR(le_description_);
	
	KQuickHelp::add(le_description_, i18n(
			"Write your own description of the attachment here.\n"
			"For example, ``Here's that file you didn't want''"));
	
	pb_browse_ = new QPushButton(w_main_);
	CHECK_PTR(pb_browse_);
	
	pb_browse_->setPixmap(empathIcon("browse.png"));
	pb_browse_->setFixedSize(h, h);

	bg_encoding_ =
		new QButtonGroup(this, "bg_encoding");
	CHECK_PTR(bg_encoding_);

	bg_encoding_->hide();
	bg_encoding_->setExclusive(true);

	rb_base64_	=
		new QRadioButton(i18n("Base 64"), w_encoding_, "rb_base64_");
	CHECK_PTR(rb_base64_);
	
	KQuickHelp::add(rb_base64_, i18n(
			"Encode the attachment using base 64.\n"
			"This is generally the best encoding type to use"));

	rb_qp_		=
		new QRadioButton(i18n("Quoted printable"), w_encoding_, "rb_qp");
	CHECK_PTR(rb_qp_);
	
	KQuickHelp::add(rb_qp_, i18n(
			"Encode the attachment as quoted-printable.\n"
			"This is useful when you're not sure if your\n"
			"recipient is able to read 8 bit or base64 messages\n"
			"as it doesn't make text completely unreadable."));

	rb_8bit_	=
		new QRadioButton(i18n("8 Bit"), w_encoding_, "rb_8bit_");
	CHECK_PTR(rb_8bit_);
	
	KQuickHelp::add(rb_8bit_, i18n(
			"Encode the attachment using 8 bit.\n"
			"Actually, this doesn't do anything much\n"
			"and it's fine for just sending text."));

	rb_7bit_	=
		new QRadioButton(i18n("7 bit"), w_encoding_, "rb_7bit_");
	CHECK_PTR(rb_7bit_);
	
	KQuickHelp::add(rb_7bit_, i18n(
			"Encode the attachment as 7 bit.\n"
			"This is only useful if you want to strip\n"
			"any 8 bit characters as the message will\n"
			"be passing through an MTA that can't cope\n"
			"with 8 bit, or being read by a dumb MUA.\n"
			"If you don't understand this, don't worry."));

	rb_base64_	->setFixedHeight(h);
	rb_8bit_	->setFixedHeight(h);
	rb_7bit_	->setFixedHeight(h);
	rb_qp_		->setFixedHeight(h);
	
	rb_base64_	->setChecked(true);
	rb_8bit_	->setChecked(false);
	rb_7bit_	->setChecked(false);
	rb_qp_		->setChecked(false);

	bg_encoding_->insert(rb_base64_);
	bg_encoding_->insert(rb_8bit_);
	bg_encoding_->insert(rb_7bit_);
	bg_encoding_->insert(rb_qp_);
	

	l_type_		= new QLabel(i18n("Type"), w_main_, "l_type");
	CHECK_PTR(l_type_);

	l_subType_	= new QLabel(i18n("SubType"), w_main_, "l_subType");
	CHECK_PTR(l_subType_);
	
	cb_type_		= new QComboBox(w_main_, "cb_type");
	CHECK_PTR(cb_type_);
	
	KQuickHelp::add(cb_type_, i18n(
			"Specify the major type of the attachment."));
	
	cb_type_->insertItem("text",		0);
	cb_type_->insertItem("message",		1);
	cb_type_->insertItem("application",	2);
	cb_type_->insertItem("image",		3);
	cb_type_->insertItem("video",		4);
	cb_type_->insertItem("audio",		5);
	
	cb_subType_		= new QComboBox(true, w_main_, "cb_subType");
	CHECK_PTR(cb_subType_);
	
	KQuickHelp::add(cb_subType_, i18n(
			"Specify the minor type of the attachment.\n"
			"You may make up your own here, but precede\n"
			"it with 'X-'."));

	l_charset_		= new QLabel(i18n("Charset"), w_main_, "l_charset");
	CHECK_PTR(l_charset_);
	
	cb_charset_		= new QComboBox(w_main_, "cb_charset");
	CHECK_PTR(cb_charset_);
	
	KQuickHelp::add(cb_charset_, i18n(
			"Choose the character set that this attachment\n"
			"will be specified to be using."));

	cb_charset_->insertItem(i18n("us-ascii (English)"));
	cb_charset_->insertItem(i18n("iso-8859-1 (Latin-1)"));
	cb_charset_->insertItem(i18n("iso-8859-2 (Latin-2)"));
	cb_charset_->insertItem(i18n("iso-8859-3 (Esperanto)"));
	cb_charset_->insertItem(i18n("iso-8859-4 (Baltic)"));
	cb_charset_->insertItem(i18n("iso-8859-5 (Cyrillic)"));
	cb_charset_->insertItem(i18n("iso-8859-6 (Arabic)"));
	cb_charset_->insertItem(i18n("iso-8859-7 (Greek)"));
	cb_charset_->insertItem(i18n("iso-8859-8 (Hebrew)"));
	cb_charset_->insertItem(i18n("iso-8859-9 (Turkish)"));
	cb_charset_->insertItem(i18n("iso-8859-10 (Nordic)"));
	cb_charset_->insertItem(i18n("KOI-8R (Russian)"));

	// Layouts

	layout_				= new QGridLayout(this,			2, 1, 10, 10);
	CHECK_PTR(layout_);
	
	mainLayout_			= new QGridLayout(w_main_,		5, 5, 10, 10);
	CHECK_PTR(mainLayout_);

	encodingLayout_		= new QGridLayout(w_encoding_,	2, 2, 10, 10);
	CHECK_PTR(encodingLayout_);
	
	layout_->addWidget(rgb_main_,	0, 0);
	layout_->addWidget(buttonBox_,	1, 0);
	
	mainLayout_->addWidget(l_filename_,				0, 0);
	mainLayout_->addMultiCellWidget(le_filename_,	0, 0, 1, 3);
	mainLayout_->addWidget(pb_browse_,				0, 4);
	
	mainLayout_->addWidget(l_description_,			1, 0);
	mainLayout_->addMultiCellWidget(le_description_,1, 1, 1, 4);
	
	mainLayout_->addWidget(l_type_,					2, 0);
	mainLayout_->addWidget(cb_type_,				2, 1);
	mainLayout_->addWidget(l_subType_,				2, 2);
	mainLayout_->addMultiCellWidget(cb_subType_,	2, 2, 3, 4);
	
	mainLayout_->addMultiCellWidget(rgb_encoding_,	3, 3, 0, 4);

	mainLayout_->addWidget(l_charset_,				4, 0);
	mainLayout_->addMultiCellWidget(cb_charset_,	4, 4, 1, 4);
	
	encodingLayout_->addWidget(rb_base64_,	0, 0);
	encodingLayout_->addWidget(rb_qp_,		0, 1);
	encodingLayout_->addWidget(rb_8bit_,	1, 0);
	encodingLayout_->addWidget(rb_7bit_,	1, 1);
	
	mainLayout_->activate();
	encodingLayout_->activate();
	
	layout_->activate();
	
	QObject::connect(
		cb_type_, SIGNAL(activated(int)),
		this, SLOT(s_typeChanged(int)));
	
	textSubTypes_.append("Plain");
	textSubTypes_.append("RichText");
	textSubTypes_.append("HTML");
	
	messageSubTypes_.append("RFC822");
	messageSubTypes_.append("Digest");
	messageSubTypes_.append("Parallel");
	messageSubTypes_.append("Partial");
	messageSubTypes_.append("External-Body");
	
	applicationSubTypes_.append("Octet-Stream");
	applicationSubTypes_.append("X-cpio");
	applicationSubTypes_.append("X-DVI");
	applicationSubTypes_.append("X-perl");
	applicationSubTypes_.append("X-tar");
	applicationSubTypes_.append("X-deb");
	applicationSubTypes_.append("X-rar-compressed");
	applicationSubTypes_.append("X-LaTeX");
	applicationSubTypes_.append("X-sh");
	applicationSubTypes_.append("X-shar");
	applicationSubTypes_.append("X-tar-gz");
	applicationSubTypes_.append("X-tcl");
	applicationSubTypes_.append("X-TeX");
	applicationSubTypes_.append("X-troff");
	applicationSubTypes_.append("X-zip");
	applicationSubTypes_.append("X-VRML");
	
	imageSubTypes_.append("JPEG");
	imageSubTypes_.append("GIF");
	imageSubTypes_.append("PNG");
	imageSubTypes_.append("TIFF");
	imageSubTypes_.append("X-XBitmap");
	imageSubTypes_.append("X-XPixmap");
	imageSubTypes_.append("X-CMU-Raster");
	imageSubTypes_.append("X-Portable-Anymap");
	imageSubTypes_.append("X-Portable-Bitmap");
	imageSubTypes_.append("X-Portable-Graymap");
	imageSubTypes_.append("X-Portable-Pixmap");
	imageSubTypes_.append("X-RGB");
	
	videoSubTypes_.append("MPEG");
	videoSubTypes_.append("QuickTime");
	videoSubTypes_.append("FLI");
	videoSubTypes_.append("GL");
	videoSubTypes_.append("X-SGI-Movie");
	videoSubTypes_.append("X-MSVideo");
	
	audioSubTypes_.append("MIDI");
	audioSubTypes_.append("ULAW");
	audioSubTypes_.append("X-AIFF");
	audioSubTypes_.append("X-WAV");
	
	s_typeChanged(2);
	cb_type_->setCurrentItem(2);
}

EmpathAttachmentEditDialog::~EmpathAttachmentEditDialog()
{
	empathDebug("dtor");
}
		
	void
EmpathAttachmentEditDialog::s_OK()
{
	accept();
}

	void
EmpathAttachmentEditDialog::s_cancel()
{
	reject();
}

	void
EmpathAttachmentEditDialog::s_help()
{
}

	void
EmpathAttachmentEditDialog::s_browse()
{
	QString filename = KFileDialog::getOpenFileName();
	
	if (filename.isEmpty())
		return;

	le_filename_->setText(filename);
}

	void
EmpathAttachmentEditDialog::s_typeChanged(int idx)
{
	cb_subType_->clear();

	switch (idx) {
		
		case 0:
			cb_subType_->insertStringList(textSubTypes_);
			break;

		case 1:
			cb_subType_->insertStringList(messageSubTypes_);
			break;

		case 2:
			cb_subType_->insertStringList(applicationSubTypes_);
			break;

		case 3:
			cb_subType_->insertStringList(imageSubTypes_);
			break;

		case 4:
			cb_subType_->insertStringList(videoSubTypes_);
			break;

		case 5:
			cb_subType_->insertStringList(audioSubTypes_);
			break;

		default:
			break;
	}
}

	EmpathAttachmentSpec
EmpathAttachmentEditDialog::spec()
{
	QString encoding;
	
	if (rb_base64_->isChecked())
		encoding = QString::fromLatin1("Base64");
	
	else if (rb_8bit_->isChecked())
		encoding = QString::fromLatin1("8bit");
	
	else if (rb_7bit_->isChecked())
		encoding = QString::fromLatin1("7bit");
	
	else if (rb_qp_->isChecked())
		encoding = QString::fromLatin1("Quoted-Printable");
	
	spec_.setFilename(le_filename_->text());
	spec_.setDescription(le_description_->text());
	spec_.setEncoding(encoding);
	spec_.setType(cb_type_->currentText());
	spec_.setSubType(cb_subType_->currentText());
	spec_.setCharset(cb_charset_->currentText());

	return spec_;
}

	void
EmpathAttachmentEditDialog::setSpec(const EmpathAttachmentSpec & s)
{
	spec_ = s;
	
	switch (spec_.filename().at(0).latin1()) {

		case '8':
			rb_8bit_->setChecked(false);
			break;
			
		case '7':
			rb_7bit_->setChecked(false);
			break;
		
		case 'Q':
		case 'q':
			rb_qp_->setChecked(false);
			break;
		
		case 'B':
		case 'b':
		default:
			rb_base64_->setChecked(true);
			break;
	}
	
	le_filename_->setText(spec_.filename());
	le_description_->setText(spec_.description());
//	cb_type_->setCurrentItem(s.type_);
//	cb_subType_->setCurrentItem(s.subType_);
}

