/***************************************************************************
                          knmimecontent.h  -  description
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


#ifndef KNMIMECONTENT_H
#define KNMIMECONTENT_H

#include <qlist.h>
#include <qfile.h>

#include "knarticlebase.h"
#include "knmimeinfo.h"
#include "knarticlecollection.h"

class QTextStream;
class DwString;


class KNMimeContent : public KNArticleBase  {
	
	public:
		KNMimeContent();
		virtual ~KNMimeContent();
	
	  void initContent();
		virtual void parse();
		virtual void assemble();
		virtual void clear();
		virtual void copyContent(KNMimeContent *c);
		void clearAttachments()				  { if(ct_List) ct_List->clear(); }		
		void clearHead()								{ if(h_ead) h_ead->clear(); }		
		void clearBody()								{ if(b_ody) b_ody->clear(); }				
		void decodeText();
		//void prepareHtml();		
		void changeEncoding(int e);
		
		//get
		
		//info
		virtual articleType type()			{ return ATmimeContent; }
		KNMimeInfo* mimeInfo();
		bool isMultipart()							{ return mInfo->ctMediaType()==MTmultipart; }
		bool hasContent()							  { return (  (b_ody!=0 && !b_ody->isEmpty()) ||
		                                            (ct_List!=0 && !ct_List->isEmpty()) ); }	
		QCString ctCharset();
		QCString ctMimeType();
		QCString ctEncoding();
		QCString ctName();
		QCString ctDescription();
		int contentSize();
		int contentLineCount();
		
		//content
		KNMimeContent* textContent();
		QString htmlCode();
		void attachments(QList<KNMimeContent> *dst, bool incAlternatives=false);
				
		QCString headerLine(const char* name);				
		char* firstHeaderLine()								{ if(h_ead) return h_ead->first();
																						else return 0; }
		char* nextHeaderLine()								{ if(h_ead) return h_ead->next();
																						else return 0; }
		char* firstBodyLine()									{ if(b_ody) return b_ody->first();
																						else return 0; }
		char* nextBodyLine()									{ if(b_ody) return b_ody->next();
																						else return 0; }
		virtual DwString decodedData();
		virtual DwString encodedData();
		void toStream(QTextStream &ts);			
								
		
		//set	
		void setData(QStrList *data, bool crfl=true);
		void addBodyLine(const char* line) 	{ if(b_ody) b_ody->append(line); }
		void addHeaderLine(const char *line, bool encode=false);
		void setHeader(const char* name, const QCString &value, bool encode=false);
		void setHeader(headerType t, const QCString &value, bool encode=false);
		bool removeHeader(const char* name);
		void addContent(KNMimeContent *c, bool prepend=false);
		void removeContent(KNMimeContent *c, bool del=false);
		
		
			
						
	protected:
		QStrList      				*h_ead, *b_ody;
		QList<KNMimeContent>  *ct_List;
		KNMimeInfo *mInfo;
				
};



//=============================================================================================



class KNAttachment {

  public:
    KNAttachment(KNMimeContent *c);
    KNAttachment(const QString &path);
    ~KNAttachment();

    //get
    const QString& contentName()            { return c_tName; }
    const QString& contentMimeType()        { return c_tMimeType; }
    const QString& contentDescription()     { return c_tDescription; }
    QString contentEncoding();
    QString contentSize();
    int cte()                               { return c_te; }
    bool isFixedBase64()                    { return f_b64; }
    bool isAttached()                       { return i_sAttached; }
    bool hasChanged()                       { return h_asChanged; }
    KNMimeContent* content()                { return c_ontent; }

    //set
    void setContentMimeType(const QString &s);
    void setContentName(const QString &s)         { c_tName=s; h_asChanged=true; }
    void setContentDescription(const QString &s)  { c_tDescription=s; h_asChanged=true; }
    void setCte(int e)                            { c_te=(KNArticleBase::encoding)(e); }
    void updateContentInfo();
    void attach(KNMimeContent *c);
    void detach(KNMimeContent *c);


  protected:		
    KNMimeContent *c_ontent;
    QString c_tName, c_tMimeType, c_tDescription;
    KNArticleBase::encoding c_te;
    KNFile f_ile;
    bool  i_sAttached, h_asChanged, f_b64;
};

#endif
