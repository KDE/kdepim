/*
 * annotationjobs.h
 *
 * Copyright (c) 2004 David Faure <faure@kde.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */

#ifndef ANNOTATIONJOBS_H
#define ANNOTATIONJOBS_H

#include <kio/job.h>
#include <tqvaluevector.h>

namespace KMail {

/// One entry in the annotation list: attribute name and attribute value
struct AnnotationAttribute {
  AnnotationAttribute() {} // for QValueVector
  AnnotationAttribute( const TQString& e, const TQString& n, const TQString& v )
    : entry( e ), name( n ), value( v ) {}
  TQString entry; // e.g. /comment
  TQString name;  // e.g. value.shared
  TQString value;
};

typedef TQValueVector<AnnotationAttribute> AnnotationList;

/**
 * This namespace contains functions that return jobs for annotation operations.
 *
 * The current implementation is tied to IMAP.
 * If someone wants to extend this to other protocols, turn the namespace into a class
 * and use virtual methods.
 */
namespace AnnotationJobs {

/**
 * Set an annotation entry (note that it can have multiple attributes)
 * @param slave Slave object the job should be assigned to
 * @param url URL for the annotation
 * @param entry the name of the annotation entry
 * @param attributes attribute name+value pairs
 */
KIO::SimpleJob* setAnnotation( KIO::Slave* slave, const KURL& url, const TQString& entry,
                               const TQMap<TQString,TQString>& attributes );

class MultiSetAnnotationJob;
/**
 * Set multiple annotation entries
 */
MultiSetAnnotationJob* multiSetAnnotation( KIO::Slave* slave, const KURL& url, const AnnotationList& annotations );

class GetAnnotationJob;
/**
 * Get an annotation entry
 * @param slave Slave object the job should be assigned to
 * @param url URL for the annotation
 * @param entry the name of the annotation entry
 * @param attributes attribute names
 */
GetAnnotationJob* getAnnotation( KIO::Slave* slave, const KURL& url, const TQString& entry,
                                 const TQStringList& attributes );

class MultiGetAnnotationJob;
/**
 * Get multiple annotation entries
 * Currently we assume we want to get the "value" for each, to simplify the data structure.
 */
MultiGetAnnotationJob* multiGetAnnotation( KIO::Slave* slave, const KURL& url, const TQStringList& entries );

class MultiUrlGetAnnotationJob;
/**
 * Get annotation entries for multiple folders.
 * @param paths The paths to get the annotation for 
 * @param annotation The annotation to get
 */
MultiUrlGetAnnotationJob* multiUrlGetAnnotation( KIO::Slave* slave,
                                              const KURL& baseUrl,
                                              const TQStringList& paths,
                                              const TQString& annotation );


/// for getAnnotation()
class GetAnnotationJob : public KIO::SimpleJob
{
  Q_OBJECT
public:
  GetAnnotationJob( const KURL& url, const TQString& entry, const TQByteArray &packedArgs,
                    bool showProgressInfo );

  const AnnotationList& annotations() const { return mAnnotations; }

protected slots:
  void slotInfoMessage( KIO::Job*, const TQString& );
private:
  AnnotationList mAnnotations;
  TQString mEntry;
};

/// for multiGetAnnotation
class MultiGetAnnotationJob : public KIO::Job
{
  Q_OBJECT

public:
  MultiGetAnnotationJob( KIO::Slave* slave, const KURL& url, const TQStringList& entries, bool showProgressInfo );

signals:
  // Emitted when a given annotation was found - or not found
  void annotationResult( const TQString& entry, const TQString& value, bool found );

protected slots:
  virtual void slotStart();
  virtual void slotResult( KIO::Job *job );

private:
  KIO::Slave* mSlave;
  const KURL mUrl;
  const TQStringList mEntryList;
  TQStringList::const_iterator mEntryListIterator;
};

/// for multiUrlGetAnnotation
class MultiUrlGetAnnotationJob : public KIO::Job
{
  Q_OBJECT

public:
  MultiUrlGetAnnotationJob( KIO::Slave* slave, const KURL& baseUrl,
                            const TQStringList& paths, const TQString& annotation );

  TQMap<TQString, TQString> annotations() const;

protected slots:
  virtual void slotStart();
  virtual void slotResult( KIO::Job *job );

private:
  KIO::Slave* mSlave;
  const KURL mUrl;
  const TQStringList mPathList;
  TQStringList::const_iterator mPathListIterator;
  TQString mAnnotation;
  TQMap<TQString, TQString> mAnnotations;
};

/// for multiSetAnnotation
class MultiSetAnnotationJob : public KIO::Job
{
  Q_OBJECT

public:
  MultiSetAnnotationJob( KIO::Slave* slave, const KURL& url, const AnnotationList& annotations, bool showProgressInfo );

signals:
  // Emitted when a given annotation was successfully changed
  void annotationChanged( const TQString& entry, const TQString& attribute, const TQString& value );

protected slots:
  virtual void slotStart();
  virtual void slotResult( KIO::Job *job );

private:
  KIO::Slave* mSlave;
  const KURL mUrl;
  const AnnotationList mAnnotationList;
  AnnotationList::const_iterator mAnnotationListIterator;
};

} // AnnotationJobs namespace

} // KMail namespace

#endif /* ANNOTATIONJOBS_H */

