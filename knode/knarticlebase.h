/***************************************************************************
                          knarticlebase.h  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef KNARTICLEBASE_H
#define KNARTICLEBASE_H

#include <qstring.h>
#include <qstrlist.h>


class KNArticleBase {
	
	public:
		
		enum articleType 	{ 	ATfetch,
													ATsaved,
													ATcontrol,
													ATmimeContent };
							
	  enum mediaType    { 	MTtext, MTimage, MTaudio,
	  									 		MTvideo, MTapplication,
	  											MTmultipart, MTmessage, MTcustom };
	
	  enum subType			{ 	STplain, SThtml, STenriched, 										
	  											STgif, STjpeg,             									
	  											STbasic,                 										
	  											STmpeg,                   									
	  											STPostScript, SToctetStream,   								
                    	  	STmixed, STalternative, STparallel, STdigest,
                    	  	STrfc822, STpartial, STexternalBody,
                    	  	STcustom };

    enum encoding			{ 	ECsevenBit=0, ECeightBit=1, ECquotedPrintable=2,
    											ECbase64=3, ECuuencode=4, ECbinary=5, ECnone=6 };
    										
    enum disposition	{ 	DPinline, DPattached };

    enum articleStatus {	AStoPost=0, AStoMail=1, ASposted=2,
    											ASmailed=3, ASsaved=4, AStemp=5,
    											AScanceled=6 , ASunknown=7 };

  	enum headerType {			HTmessageId=10, HTfrom=20 , HTsubject=30, HTcontrol=35,
  	                      HTsupersedes=36,HTto=40, HTnewsgroups=50, HTfup2=60,
  	                      HTreplyTo=65, HTdate=70, HTreferences=75, HTlines=80,
  	                      HTorga=85, HTmimeVersion=90, HTcontentType=100,
  	                      HTencoding=110, HTdescription=115, HTdisposition=120,
  	                      HTuserAgent=140, HTxknstatus=150, HTxkntempfile=160,
  	                      HTunknown=200 }; 									

		enum controlType {		CTcancel=0, CTsupersede=2, CTunknown=1 };
		
		enum contentCategory  { CCsingle=10, CCalternativePart=20, CCmixedPart=30,
		                        CCcontainer=40 };
		

    KNArticleBase()  {}
		~KNArticleBase() {}                	

		static const QCString defaultCharset()              { return defaultChSet; }
		static encoding defaultTextEncoding()               { return defaultTEncoding; }
				
		static void setAllow8bitHeaders(bool b)	            { allow8bit=b; }
		static void setDefaultCharset(const QCString &s)	  { defaultChSet=s.upper(); }
		static void setDefaultTextEncoding(encoding e)      { defaultTEncoding=e; }
		
		static QCString uniqueString();
		static QCString multiPartBoundary();
		                	                	
		static QCString decodeQuotedPrintable(const QCString str);
		static QCString encodeQuotedPrintable(const QCString str);
		static QCString decodeQuotedPrintableString(const QCString str);
		
		static QCString decodeBase64(const QCString str);
		static QCString encodeBase64(const QCString str);
		
		static QCString decodeRFC1522String(const QCString aStr);
		static QCString encodeRFC1522String(const QCString aStr);
		
		static bool stripCRLF(char *str);
		static void removeQuots(QCString &str);
		
		static QCString articleStatusToString(articleStatus s);
		static articleStatus stringToArticleStatus(const char *s);
		
		static QCString headerTypeToString(headerType t);
		static headerType stringToHeaderType(const char *s);
		
		static QCString encodingToString(encoding e);
		static encoding stringToEncoding(const char *s);                	

	protected:
	  static bool allow8bit;
	  static QCString defaultChSet;
	  static encoding defaultTEncoding;

    class FromLineParser {
				
				public:
					FromLineParser(const QCString &fLine);
					FromLineParser(const char *fLine);
					~FromLineParser();
										
					void parse();
					QCString& from()		{ return f_rom; }
					QCString& email()	{ return e_mail; }
					
					bool hasValidEmail(); 	
					bool hasValidFrom();		
					bool isBroken()				{ return is_broken; }
				
				protected:
					QCString f_rom, e_mail, src;
					bool is_broken;
					
			};
			
			
		class MultiPartParser {
			
			public:
				MultiPartParser(QStrList *l, const QCString &b);
				~MultiPartParser();
				
				
				QStrList* begin()	{ pos=0; return nextPart(); }
				QStrList* nextPart();
								
			protected:
				bool isStartBoundary(const char *line);
				bool isEndBoundary(const char *line);
								
				QCString startBoundary, endBoundary;
				QStrList *p_art, *src;
				
				int pos;
		};
		
		class UUParser {
			
			public:
				UUParser(QStrList *l, const QCString &s);
				~UUParser();
				
				void parse();
				bool isUUencoded()       	            { return (bin!=0); }
				bool isPartial()                      { return (partNr>-1 && totalNr>-1); }
				int numberOfPart()                    { return partNr; }
				int totalNumberOfParts()              { return totalNr; }
				QStrList* textPart()			            { return text; }
				QStrList* binaryPart()		            { return bin; }
				const QCString& fileName()	          { return fName; }
				const QCString& assumedMimeType()     { return mimeType; }
				
			protected:
				QStrList *src, *text, *bin;
				QCString fName, mimeType, subject;
				int partNr, totalNr;
				
		};
		
		class ReferenceLine {
		
		  public:
		    ReferenceLine();
		    ~ReferenceLine();
		
		    void setLine(const QCString &l)     { l_ine=l; }
		    void append(const QCString &r);
		    void clear()                        { l_ine.resize(0); }
		    		
		    const QCString& line()              { return l_ine; }
		    QCString first();
		    QCString next();
		    QCString at(int i);
		    int count();
		    bool isEmpty()                      { return ( l_ine.isEmpty() ); }
		
		  protected:
		    QCString l_ine;
		    int pos;
		};
		
};


#endif
