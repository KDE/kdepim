/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rikkus@postmaster.co.uk
	
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

#ifndef RMM_UTILITY_H
#define RMM_UTILITY_H

void	RInitialize();
void	RFinalize();

int		RCteStrToEnum(const QString & aStr);
void	RCteEnumToStr(int aEnum, QString & aStr);

int		RTypeStrToEnum(const QString & aStr);
void	RTypeEnumToStr(int aEnum, QString & aStr);

int		RSubtypeStrToEnum(const QString & aStr);
void	RSubtypeEnumToStr(int aEnum, QString & aStr);

int		RToCrLfEol(const QString & aSrcStr, QString & aDestStr);
int		RToLfEol(const QString & aSrcStr, QString & aDestStr);
int		RToCrEol(const QString & aSrcStr, QString & aDestStr);
int		RToLocalEol(const QString & aSrcStr, QString & aDestStr);

int		REncodeBase64(const QString & aSrcStr, QString & aDestStr);
int		RDecodeBase64(const QString & aSrcStr, QString & aDestStr);

int		REncodeQuotedPrintable(const QString & aSrcStr, QString & aDestStr);
int		RDecodeQuotedPrintable(const QString & aSrcStr, QString & aDestStr);

#endif
