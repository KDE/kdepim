/*
    ktnefattach.h

    Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#ifndef KTNEFATTACH_H
#define	KTNEFATTACH_H

#include <qstring.h>
#include <qmap.h>
#include <qvariant.h>
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
