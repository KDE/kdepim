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

#ifndef RMM_UTILITY_H
#define RMM_UTILITY_H

#include <RMM_Enum.h>

QCString			RToCrLfEol				(const QCString &);
QCString			RToLfEol				(const QCString &);
QCString			RToCrEol				(const QCString &);
QCString			RToLocalEol				(const QCString &);

QCString			REncodeBase64			(const QCString &);
QCString			RDecodeBase64			(const QCString &);

QCString			REncodeQuotedPrintable	(const QCString &);
QCString			RDecodeQuotedPrintable	(const QCString &);

#endif
