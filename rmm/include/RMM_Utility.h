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

int		RCteStrToEnum(const QCString & aStr);
void	RCteEnumToStr(int aEnum, QCString & aStr);

int		RTypeStrToEnum(const QCString & aStr);
void	RTypeEnumToStr(int aEnum, QCString & aStr);

int		RSubtypeStrToEnum(const QCString & aStr);
void	RSubtypeEnumToStr(int aEnum, QCString & aStr);

int		RToCrLfEol(const QCString & aSrcStr, QCString & aDestStr);
int		RToLfEol(const QCString & aSrcStr, QCString & aDestStr);
int		RToCrEol(const QCString & aSrcStr, QCString & aDestStr);
int		RToLocalEol(const QCString & aSrcStr, QCString & aDestStr);

int		REncodeBase64(const QCString & aSrcStr, QCString & aDestStr);
int		RDecodeBase64(const QCString & aSrcStr, QCString & aDestStr);

int		REncodeQuotedPrintable(const QCString & aSrcStr, QCString & aDestStr);
int		RDecodeQuotedPrintable(const QCString & aSrcStr, QCString & aDestStr);

#endif
