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

#include <qstring.h>

#include <iostream>
#include <RMM_Enum.h>

	Month
strToMonth(const QCString & s)
{
	if (s.isEmpty()) return MonthJan;
	
	for (int i = 1; i <= 12; i++)
		if (stricmp(s, monthNames[i-1])) return (Month)i;

	return MonthJan;

}

	DayOfWeek
strToDayOfWeek(const QCString & s)
{
	if (s.isEmpty()) return DayNone;

	for (int i = 0; i < 7; i++)
		if (stricmp(s, dayNames[i])) return (DayOfWeek)++i;
	
	return DayNone;
}

	MimeType
mimeTypeStr2Enum(const QCString & s)
{
	if (s.isEmpty()) return MimeTypeNone;
	
	MimeType t = MimeTypeUnknown;

	switch (s[0]) {

		case 0:
			
			t = MimeTypeNone;

		case 'a':
		case 'A':
		
			if (stricmp(s, "application") == 0)
				t = MimeTypeApplication;

			else if (stricmp(s, "audio") == 0)
				t = MimeTypeAudio;

			break;
			
		case 'i':
		case 'I':

			if (stricmp(s, "image") == 0)
				t = MimeTypeImage;

			break;
			
		case 'm':
		case 'M':

			if (stricmp(s, "message") == 0)
				t = MimeTypeMessage;

			else if (stricmp(s, "multipart") == 0)
				t = MimeTypeMultiPart;
			
			break;

		case 't':
		case 'T':

			if (stricmp(s, "text") == 0)
				t = MimeTypeText;
			
			break;

		case 'v':
		case 'V':

			if (stricmp(s, "video") == 0)
				t = MimeTypeVideo;

			break;

		default:

			t = MimeTypeUnknown;
	}

	return t;
}

	QCString
mimeTypeEnum2Str(MimeType m)
{
	QCString s;
	
	switch (m) {

		case MimeTypeNone:
			s = "";
			break;
			
		case MimeTypeUnknown:
			s = "Unknown";
			break;
			
		case MimeTypeText:
			s =  "Text";
			break;
			
		case MimeTypeMultiPart:
			s =  "Multipart";
			break;
			
		case MimeTypeMessage:
			s =  "Message";
			break;
			
		case MimeTypeImage:
			s =  "Image";
			break;
			
		case MimeTypeApplication:
			s =  "Application";
			break;
			
		case MimeTypeAudio:
			s =  "Audio";
			break;
			
		case MimeTypeVideo:
			s =  "Video";
			break;
			
		case MimeTypeModel:
			s =  "Model";
			break;
	}

	return s;
}

	MimeSubType
mimeSubTypeStr2Enum(const QCString & s)
{
	if (s.isEmpty()) return MimeSubTypeNone;

	MimeSubType st = MimeSubTypeNone;
	
	switch (s[0]) {

		case 'a':
		case 'A':
		
			if (stricmp(s, "alternative") == 0)
				st = MimeSubTypeAlternative;
		
			break;
		
		case 'b':
		case 'B':
		
			if (stricmp(s, "basic") == 0)
				st = MimeSubTypeBasic;
				
			break;
			
		case 'd':
		case 'D':
		
			if (stricmp(s, "digest") == 0)
				st = MimeSubTypeDigest;

			break;
		
		case 'e':
		case 'E':
		
			if (stricmp(s, "enriched") == 0)
				st = MimeSubTypeEnriched;

			else if (stricmp(s, "external-body") == 0)
				st = MimeSubTypeExternalBody;

			break;

		case 'g':
		case 'G':
		
			if (stricmp(s, "gif") == 0)
				st = MimeSubTypeGIF;

			break;
			
		case 'h':
		case 'H':
		
			if (stricmp(s, "html") == 0)
				st = MimeSubTypeHTML;

			break;

		case 'j':
		case 'J':
		
			if (stricmp(s, "jpeg") == 0)
				st = MimeSubTypeJpeg;

			break;

		case 'm':
		case 'M':

			if (stricmp(s, "mixed") == 0)
				st = MimeSubTypeMixed;

			else if (stricmp(s, "mpeg") == 0)
				st = MimeSubTypeMPEG;

			break;

		case 'o':
		case 'O':
		
			if (stricmp(s, "octet-stream") == 0)
				st = MimeSubTypeOctetStream;

			break;
			
		case 'p':
		case 'P':

			if (stricmp(s, "plain") == 0)
				st = MimeSubTypePlain;
			
			else if (stricmp(s, "postscript") == 0)
				st = MimeSubTypePostScript;

			else if (stricmp(s, "parallel") == 0)
				st = MimeSubTypeParallel;

			else if (stricmp(s, "partial") == 0)
				st = MimeSubTypePartial;

			break;

		case 'r':
		case 'R':
		
			if (stricmp(s, "rfc822") == 0)
				st = MimeSubTypeRFC822;
				
			else if (stricmp(s, "richtext") == 0)
				st = MimeSubTypeRichtext;

			break;

		default:

			st = MimeSubTypeUnknown;
	}

	return st;
}

	QCString
mimeSubTypeEnum2Str(MimeSubType t)
{
	QCString s;
	
	switch (t) {
		
		case MimeSubTypeNone:
			s = "";
			break;
		
			
		case MimeSubTypePlain:
			s = "Plain";
			break;

		case MimeSubTypeRichtext:
			s = "Richtext";
			break;

		case MimeSubTypeEnriched:
			s = "Enriched";
			break;

		case MimeSubTypeHTML:
			s = "HTML";
			break;

		case MimeSubTypeMixed:
			s = "Mixed";
			break;

		case MimeSubTypeAlternative:
			s = "Alternative";
			break;

		case MimeSubTypeDigest:
			s = "Digest";
			break;

		case MimeSubTypeParallel:
			s = "Parallel";
			break;

		case MimeSubTypeRFC822:
			s = "RFC822";
			break;

		case MimeSubTypePartial:
			s = "Partial";
			break;

		case MimeSubTypeExternalBody:
			s = "External-body";
			break;

		case MimeSubTypePostScript:
			s = "Postscript";
			break;

		case MimeSubTypeOctetStream:
			s = "Octet-stream";
			break;

		case MimeSubTypeJpeg:
			s = "jpeg";
			break;

		case MimeSubTypeGIF:
			s = "gif";
			break;

		case MimeSubTypeBasic:
			s = "basic";
			break;

		case MimeSubTypeMPEG:
			s = "mpeg";
			break;

		case MimeSubTypeUnknown:
		default:
			s = "Unknown";
			break;
	}

	return s;
}

	CteType
RCteStr2Enum(const QCString & s)
{
	CteType cte = CteType7bit;
	
	switch (s[0]) {
	
		case '7':
			
			if (stricmp(s, "7bit") == 0)
				cte = CteType7bit;
			
			break;
			
		case '8':
			
			if (stricmp(s, "8bit") == 0)
				cte = CteType8bit;
			
			break;
			
		case 'b':
		case 'B':
		
			if (stricmp(s, "binary") == 0)
				cte = CteTypeBinary;
		
			else if (stricmp(s, "base64") == 0)
				cte = CteTypeBase64;
			
			break;
			
		case 'q':
		case 'Q':
		
			if (stricmp(s, "quoted-printable") == 0)
				cte = CteTypeQuotedPrintable;
			
			break;

		default:
			cte = CteType7bit;
			break;
	}

	return cte;
}

	QCString
cteTypeEnumToStr(CteType t)
{
	QCString s;

	switch (t) {
		
		case CteType7bit:
			s = "7bit";
			break;
			
		case CteType8bit:
			s = "8bit";
			break;
			
		case CteTypeBinary:
			s = "binary";
			break;
			
		case CteTypeBase64:
			s = "base64";
			break;
			
		case CteTypeQuotedPrintable:
			s = "quoted-printable";
			break;

		default:
			s = "";
	}

	return s;
}

