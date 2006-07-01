/*
    ktnefattach.h

    Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

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

#ifndef KTNEFATTACH_H
#define	KTNEFATTACH_H

#include <QString>
#include <QMap>
#include <QVariant>
#include <ktnef/ktnefpropertyset.h>
#include <kdepimmacros.h>

class KTNEFProperty;

class KDE_EXPORT KTNEFAttach : public KTNEFPropertySet
{
public:
	enum ParseState { Unparsed = 0x0000, TitleParsed = 0x0001, DataParsed = 0x0002, InfoParsed = 0x0004};

	KTNEFAttach();
	~KTNEFAttach();

	void setTitleParsed();
	void setDataParsed();
	void unsetDataParser();
	void setInfoParsed();
	bool titleParsed() const;
	bool dataParsed() const;
	bool infoParsed() const;
	bool checkState(int state) const;

	int offset() const;
	void setOffset(int n);
	int size() const;
	void setSize(int s);
	int displaySize() const;
	void setDisplaySize(int s);
	QString name() const;
	void setName(const QString& str);
	int index() const;
	void setIndex(int i);
	QString fileName() const;
	void setFileName(const QString& str);
	QString displayName() const;
	void setDisplayName(const QString& str);
	QString mimeTag() const;
	void setMimeTag(const QString& str);
	QString extension() const;
	void setExtension(const QString& str);

private:
	class AttachPrivate;
	AttachPrivate *d;
};

#endif
