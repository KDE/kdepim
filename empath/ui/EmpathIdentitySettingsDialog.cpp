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

// Qt includes
#include <qfile.h>
#include <qtextstream.h>

// KDE includes
#include <klocale.h>
#include <kconfig.h>
#include <kapp.h>
#include <kfiledialog.h>

// Local includes
#include "EmpathIdentitySettingsDialog.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "RikGroupBox.h"
		
EmpathIdentitySettingsDialog::EmpathIdentitySettingsDialog(
		QWidget * parent,
		const char * name)
	:	QWidget(parent, name)
{
	empathDebug("ctor");
	
	rgb_main_	= new RikGroupBox("", 8, this, "rgb_font");
	CHECK_PTR(rgb_main_);
	
	w_main_		= new QWidget(rgb_main_, "w_main");
	CHECK_PTR(w_main_);
	
	rgb_main_->setWidget(w_main_);
	
	rgb_sigPreview_	= new RikGroupBox(i18n("Signature Preview"), 8,
			w_main_, "rgb_sigPreview");
	CHECK_PTR(rgb_sigPreview_);
	
	w_sigPreview_	= new QWidget(rgb_sigPreview_, "w_sigPreview");
	CHECK_PTR(w_sigPreview_);

	rgb_sigPreview_->setWidget(w_sigPreview_);
	
	QLineEdit	tempLineEdit((QWidget *)0);
	Q_UINT32 h	= tempLineEdit.sizeHint().height();

	l_name_	=
		new QLabel(i18n("Your name"), w_main_, "l_name");
	CHECK_PTR(l_name_);

	l_name_->setMaximumHeight(h);
	
	le_chooseName_	=
		new QLineEdit(w_main_, "le_chooseName");
	CHECK_PTR(le_chooseName_);

	le_chooseName_->setFixedHeight(h);
	
	l_email_	=
		new QLabel(i18n("email address"), w_main_, "l_email");
	CHECK_PTR(l_email_);
	
	l_email_->setFixedHeight(h);
	
	le_chooseEmail_	=
		new QLineEdit(w_main_, "le_chooseEmail");
	CHECK_PTR(le_chooseEmail_);
	
	le_chooseEmail_->setFixedHeight(h);
	
	l_replyTo_	=
		new QLabel(i18n("Reply-to address (if different from email address)"),
				w_main_, "l_replyTo");
	CHECK_PTR(l_replyTo_);
	
	l_replyTo_->setFixedHeight(h);
	
	le_chooseReplyTo_	=
		new QLineEdit(w_main_, "le_chooseReplyTo");
	CHECK_PTR(le_chooseReplyTo_);
	
	le_chooseReplyTo_->setFixedHeight(h);
	
	l_org_	=
		new QLabel(i18n("Organisation"), w_main_, "l_org");
	CHECK_PTR(l_org_);
	
	l_org_->setFixedHeight(h);
	
	le_chooseOrg_	=
		new QLineEdit(w_main_, "le_chooseOrg");
	CHECK_PTR(le_chooseOrg_);
	
	le_chooseOrg_->setFixedHeight(h);
	
	l_sig_	=
		new QLabel(i18n("Signature file"), w_main_, "l_sig");
	CHECK_PTR(l_sig_);
	
	l_sig_->setFixedHeight(h);
	
	le_chooseSig_	=
		new QLineEdit(w_main_, "le_chooseSig");
	CHECK_PTR(le_chooseSig_);

	le_chooseSig_->setFixedHeight(h);
	
	pb_chooseSig_	=
		new QPushButton(w_main_, "pb_chooseSig");
	CHECK_PTR(pb_chooseSig_);
	
	pb_chooseSig_->setText("...");
	pb_chooseSig_->setFixedSize(h, h);
	
	QObject::connect(pb_chooseSig_, SIGNAL(clicked()),
			this, SLOT(s_chooseSig()));
	
	pb_editSig_		=
		new QPushButton(w_main_, "pb_editSig");
	CHECK_PTR(pb_editSig_);

	pb_editSig_->setText(i18n("Edit"));
	
	pb_editSig_->setFixedSize(pb_editSig_->sizeHint());

	QObject::connect(pb_editSig_, SIGNAL(clicked()),
			this, SLOT(s_editSig()));

	mle_sigPreview_	=
		new QMultiLineEdit(w_sigPreview_, "mle_sigPreview");
	CHECK_PTR(mle_sigPreview_);

	mle_sigPreview_->setReadOnly(true);
	QFont f("TypeWriter");
	f.setStyleHint(QFont::TypeWriter);
	mle_sigPreview_->setFont(f);
	mle_sigPreview_->setText(i18n("No signature set"));
	
	// Layouts
	
	topLevelLayout_		= new QGridLayout(this, 1, 1, 10, 10);
	CHECK_PTR(topLevelLayout_);

	mainGroupLayout_	= new QGridLayout(w_main_, 11, 3, 0, 10);
	CHECK_PTR(mainGroupLayout_);
	
	sigPreviewGroupLayout_	= new QGridLayout(w_sigPreview_, 1, 1, 0, 10);
	CHECK_PTR(sigPreviewGroupLayout_);

	sigPreviewGroupLayout_->addWidget(mle_sigPreview_, 0, 0);
	sigPreviewGroupLayout_->activate();

	topLevelLayout_->addWidget(rgb_main_, 0, 0);
	
	mainGroupLayout_->setColStretch(0, 9);
	mainGroupLayout_->setColStretch(1, 0);
	mainGroupLayout_->setColStretch(2, 3);
	
	mainGroupLayout_->setRowStretch(0, 0);
	mainGroupLayout_->setRowStretch(1, 0);
	mainGroupLayout_->setRowStretch(2, 0);
	mainGroupLayout_->setRowStretch(3, 0);
	mainGroupLayout_->setRowStretch(4, 0);
	mainGroupLayout_->setRowStretch(5, 0);
	mainGroupLayout_->setRowStretch(6, 0);
	mainGroupLayout_->setRowStretch(7, 0);
	mainGroupLayout_->setRowStretch(8, 0);
	mainGroupLayout_->setRowStretch(9, 0);
	mainGroupLayout_->setRowStretch(10, 10);

	mainGroupLayout_->addMultiCellWidget(l_name_,			0, 0, 0, 2);
	mainGroupLayout_->addMultiCellWidget(le_chooseName_,	1, 1, 0, 2);
	mainGroupLayout_->addMultiCellWidget(l_email_,			2, 2, 0, 2);
	mainGroupLayout_->addMultiCellWidget(le_chooseEmail_,	3, 3, 0, 2);
	mainGroupLayout_->addMultiCellWidget(l_replyTo_,		4, 4, 0, 2);
	mainGroupLayout_->addMultiCellWidget(le_chooseReplyTo_,	5, 5, 0, 2);
	mainGroupLayout_->addMultiCellWidget(l_org_,			6, 6, 0, 2);
	mainGroupLayout_->addMultiCellWidget(le_chooseOrg_,		7, 7, 0, 2);
	mainGroupLayout_->addMultiCellWidget(l_sig_,			8, 8, 0, 2);
	mainGroupLayout_->addWidget(le_chooseSig_,				9, 0);
	mainGroupLayout_->addWidget(pb_chooseSig_,				9, 1);
	mainGroupLayout_->addWidget(pb_editSig_,				9, 2);

	mainGroupLayout_->addMultiCellWidget(rgb_sigPreview_,	10, 10, 0, 2);
	
	mainGroupLayout_->activate();
	
	topLevelLayout_->activate();
}

	void
EmpathIdentitySettingsDialog::s_chooseSig()
{
	
	QString tempSig = KFileDialog::getOpenFileName();
	
	if (tempSig.length() != 0)
		sig_ = tempSig.data();
	le_chooseSig_->setText(sig_);
	
	// Preview the sig
	QFile f(sig_);
	if ( f.open(IO_ReadOnly) ) {
		QTextStream t(&f);
		QString s;
		while (!t.eof()) {
			s = s + t.readLine() + '\n';
		}
		f.close();
		mle_sigPreview_->setText(s);
	}
}

	void
EmpathIdentitySettingsDialog::saveData()
{
	empathDebug("saveData() called");
	KConfig * c = kapp->getConfig();
	c->setGroup(GROUP_IDENTITY);

#define CWE c->writeEntry
	CWE( KEY_NAME,				le_chooseName_->text()		);
	CWE( KEY_EMAIL,				le_chooseEmail_->text()		);
	CWE( KEY_REPLY_TO,			le_chooseReplyTo_->text()	);
	CWE( KEY_ORGANISATION,		le_chooseOrg_->text()		);
	CWE( KEY_SIG_PATH,			le_chooseSig_->text()		);
#undef CWE
}

	void
EmpathIdentitySettingsDialog::loadData()
{
	empathDebug("loadData() called");
	KConfig * c = kapp->getConfig();
	c->setGroup(GROUP_IDENTITY);
	
	le_chooseName_->setText(c->readEntry(KEY_NAME));
	le_chooseEmail_->setText(c->readEntry(KEY_EMAIL));
	le_chooseReplyTo_->setText(c->readEntry(KEY_REPLY_TO));
	le_chooseOrg_->setText(c->readEntry(KEY_ORGANISATION));
	le_chooseSig_->setText(c->readEntry(KEY_SIG_PATH));

	if (QString(le_chooseSig_->text()).length() != 0) {
		// Preview the sig
		QFile f(le_chooseSig_->text());
		if ( f.open(IO_ReadOnly) ) {
			QTextStream t(&f);
			QString s;
			while (!t.eof()) {
				s = s + t.readLine() + '\n';
			}
			f.close();
			mle_sigPreview_->setText(s);
		}
	}
}

	void
EmpathIdentitySettingsDialog::s_editSig()
{
}


