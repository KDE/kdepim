/*
	This file is part of libkcal.

	Copyright (c) 2003,4 Cornelius Schumacher <schumacher@kde.org>
	Copyright (c) 2007 Mike Arthur <mike@mikearthur.co.uk>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public License
	along with this library; see the file COPYING.LIB.  If not, write to
	the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301, USA.
*/

#include <typeinfo>

#include <QLabel>
#include <QLayout>
#include <QGridLayout>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdialog.h>
#include <kurlrequester.h>
#include <klineedit.h>

#include <kcal/resourcecachedconfig.h>

#include "resourceblog.h"

#include "resourceblogconfig.h"

using namespace KCal;

KCAL_RESOURCEBLOG_EXPORT ResourceBlogConfig::ResourceBlogConfig(QWidget* parent)
	: KRES::ConfigWidget(parent)
{
	resize(245, 115);
	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setSpacing(KDialog::spacingHint());

	QLabel *label = new QLabel(i18n("XML-RPC URL:"), this);

	mUrl = new KUrlRequester(this);
	mUrl->setMode(KFile::File);
	mainLayout->addWidget(label, 1, 0);
	mainLayout->addWidget(mUrl, 1, 1);

	label = new QLabel(i18n("User:"), this);
	mUser = new KLineEdit(this);

	mainLayout->addWidget(label, 2, 0);
	mainLayout->addWidget(mUser, 2, 1);

	label = new QLabel(i18n("Password:"), this);
	mPassword = new KLineEdit(this);
	mPassword->setEchoMode(QLineEdit::Password);

	mainLayout->addWidget(label, 3, 0);
	mainLayout->addWidget(mPassword, 3, 1);

	/* TODO: Add API list selector */

	mReloadConfig = new ResourceCachedReloadConfig(this);
	mainLayout->addWidget(mReloadConfig, 4, 0, 1, 2);

	mSaveConfig = new ResourceCachedSaveConfig(this);
	mainLayout->addWidget(mSaveConfig, 5, 0, 1, 2);
}

void ResourceBlogConfig::loadSettings(KRES::Resource *resource)
{
	ResourceBlog *res = static_cast<ResourceBlog *>(resource);
	if (res) {
		mUrl->setUrl(res->url().url());
		mReloadConfig->loadSettings(res);
		kDebug(5700) << "reloaded" << endl;
		mSaveConfig->loadSettings(res);
		kDebug(5700) << "saved" << endl;
	}
	else
		kError(5700) << "ResourceBlogConfig::loadSettings(): no ResourceBlog, cast failed" << endl;
}

void ResourceBlogConfig::saveSettings(KRES::Resource *resource)
{
	ResourceBlog *res = static_cast<ResourceBlog*>(resource);
	if (res) {
		mUrl->setUrl(res->url().url());
		mReloadConfig->saveSettings(res);
		mSaveConfig->saveSettings(res);
	}
	else
		kError(5700) << "ResourceBlogConfig::saveSettings(): no ResourceBlog, cast failed" << endl;
}

#include "resourceblogconfig.moc"
