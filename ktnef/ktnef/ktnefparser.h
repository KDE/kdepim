/*
    ktnefparser.h

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

#ifndef KTNEFPARSER_H
#define	KTNEFPARSER_H

#include <QString>
#include <QMap>
#include <QIODevice>
#include <kdepimmacros.h>

class KTNEFAttach;
class KTNEFMessage;
class KTNEFProperty;

class KDE_EXPORT KTNEFParser
{
public:
	KTNEFParser();
	~KTNEFParser();

	bool openFile(const QString& filename);
	bool openDevice( QIODevice *device );
	bool extractFile(const QString& filename);
	bool extractFileTo(const QString& filename, const QString& dirname);
	bool extractAll();
	void setDefaultExtractDir(const QString& dirname);
	KTNEFMessage* message() const;

private:
	bool decodeAttachment();
	bool decodeMessage();
	bool extractAttachmentTo(KTNEFAttach *att, const QString& dirname);
	bool parseDevice();
	void checkCurrent(int state);
	bool readMAPIProperties(QMap<int,KTNEFProperty*>& pros, KTNEFAttach *attach = 0);
	void deleteDevice();

private:
	class ParserPrivate;
	ParserPrivate *d;
};

#endif
