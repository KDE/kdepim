/*
 * kmail: KDE mail client
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef kmmsginfo_h
#define kmmsginfo_h

#include <config.h>
#include <sys/types.h>
#include "kmmsgbase.h"

class KMMessage;

class KMMsgInfo: public KMMsgBase
{
public:
  KMMsgInfo(KMFolder* parent, off_t off=0, short len=0);
  virtual ~KMMsgInfo();

  /** left for old style index files */
  void compat_fromOldIndexString(const TQCString& str, bool toUtf8);


  /** Initialize with given values and set dirty flag to FALSE. */
  virtual void init(const TQCString& subject, const TQCString& from,
                    const TQCString& to, time_t date,
		    KMMsgStatus status, const TQCString& xmark,
                    const TQCString& replyToId,
                    const TQCString& replyToAuxId,
                    const TQCString& msgId,
		    KMMsgEncryptionState encryptionState,
		    KMMsgSignatureState signatureState,
		    KMMsgMDNSentState mdnSentState,
                    const TQCString& prefCharset,
		    off_t folderOffset=0, size_t msgSize=0,
            size_t msgSizeServer = 0, ulong UID = 0);

  /** Initialize with given values and set dirty flag to FALSE. */
  virtual void init(const TQCString& subject, const TQCString& from,
                    const TQCString& to, time_t date,
		    KMMsgStatus status, const TQCString& xmark,
                    const TQCString& replyToId,
                    const TQCString& replyToAuxId,
                    const TQCString& msgId,
		    const TQCString& fileName,
		    KMMsgEncryptionState encryptionState,
		    KMMsgSignatureState signatureState,
		    KMMsgMDNSentState mdnSentState,
		    const TQCString& prefCharset,
                    size_t msgSize=0,
            size_t msgSizeServer = 0, ulong UID = 0);

  /** Inherited methods (see KMMsgBase for description): */
  virtual TQString subject(void) const;
  virtual TQString fromStrip(void) const;
  virtual TQString from() const;
  virtual TQString toStrip(void) const;
  virtual TQString to() const;
  virtual TQString xmark(void) const;
  virtual TQString replyToIdMD5(void) const;
  virtual TQString replyToAuxIdMD5() const;
  virtual TQString strippedSubjectMD5() const;
  virtual bool subjectIsPrefixed() const;
  virtual TQString msgIdMD5(void) const;
  virtual TQString fileName(void) const;
  virtual KMMsgStatus status(void) const;
  virtual KMMsgEncryptionState encryptionState() const;
  virtual KMMsgSignatureState signatureState() const;
  virtual KMMsgMDNSentState mdnSentState() const;
  virtual off_t folderOffset(void) const;
  virtual size_t msgSize(void) const;
  virtual size_t msgSizeServer(void) const;
  virtual time_t date(void) const;
  virtual ulong UID(void) const;
  void setMsgSize(size_t sz);
  void setMsgSizeServer(size_t sz);
  void setFolderOffset(off_t offs);
  void setFileName(const TQString& file);
  virtual void setStatus(const KMMsgStatus status, int idx = -1);
  virtual void setDate(time_t aUnixTime);
  virtual void setSubject(const TQString&);
  virtual void setXMark(const TQString&);
  virtual void setReplyToIdMD5(const TQString&);
  virtual void setReplyToAuxIdMD5( const TQString& );
  virtual void initStrippedSubjectMD5();
  virtual void setMsgIdMD5(const TQString&);
  virtual void setEncryptionState( const KMMsgEncryptionState, int idx = -1 );
  virtual void setSignatureState( const KMMsgSignatureState, int idx = -1 );
  virtual void setMDNSentState( const KMMsgMDNSentState, int idx = -1 );
  virtual void setUID(ulong);
  virtual void setFrom( const TQString &from );
  virtual void setTo( const TQString &to );

  /** Grr.. c++! */
  virtual void setStatus(const char* s1, const char* s2=0) { KMMsgBase::setStatus(s1, s2); }
  virtual void setDate(const char* s1) { KMMsgBase::setDate(s1); }

  virtual bool dirty(void) const;

  /** Copy operators. */
  KMMsgInfo& operator=(const KMMessage&);

private:
  // Currently unused
  KMMsgInfo& operator=(const KMMsgInfo&);
  KMMsgInfo(const KMMsgInfo&);

  // WARNING: Do not add new member variables to the class. Add them to kd
  class KMMsgInfoPrivate;
  KMMsgInfoPrivate *kd;
};

typedef KMMsgInfo* KMMsgInfoPtr;

#endif /*kmmsginfo_h*/
