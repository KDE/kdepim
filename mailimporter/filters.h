/***************************************************************************
                          filters.h  -  description
                             -------------------
    begin                : Fri Jun 30 2000
    copyright            : (C) 2000 by Hans Dijkema
    email                : kmailcvt@hum.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FILTERS_HXX
#define FILTERS_HXX

#ifndef MAX_LINE
#define MAX_LINE 4096
#endif

#include <Akonadi/Collection>
#include <KMime/KMimeMessage>


#include "filterinfo.h"
#include "filters.h"
#include "mailimporter_export.h"

namespace MailImporter {
class MAILIMPORTER_EXPORT Filter
{
public:
  explicit Filter( const QString& name, const QString& author,
                   const QString& info = QString() );
  virtual ~Filter();
  virtual void import() = 0;
  
  QString author() const;
  QString name() const;
  QString info() const;

  void setFilterInfo( MailImporter::FilterInfo* info );


protected:

  /**
   * Adds a single subcollection to the given base collection and returns it.
   * Use parseFolderString() instead if you want to create hierachies of collections.
   */
  Akonadi::Collection addSubCollection( const Akonadi::Collection &baseCollection,
                                        const QString &newCollectionPathName );

  /**
   * Creates a hierachy of collections based on the given path string. The collection
   * hierachy will be placed under the root collection.
   * For example, if the folderParseString "foo/bar/test" is passsed to this method, it
   * will make sure the root collection has a subcollection named "foo", which in turn
   * has a subcollection named "bar", which again has a subcollection named "test".
   * The "test" collection will be returned.
   * An invalid collection will be returned in case of an error.
   */
  Akonadi::Collection parseFolderString( const QString &folderParseString );

  bool addAkonadiMessage( const Akonadi::Collection &collection,
                          const KMime::Message::Ptr& message );

  bool addMessage( const QString& folder,
                   const QString& msgFile,
                   const QString& msgStatusFlags = QString());

  /**
   * Checks for duplicate messages in the collection by message ID.
   * returns true if a duplicate was detected.
   * NOTE: Only call this method if a message ID exists, otherwise
   * you could get false positives.
   */
  bool checkForDuplicates( const QString& msgID,
                           const Akonadi::Collection& msgCollection,
                           const QString& messageFolder );
  bool addMessage_fastImport( const QString& folder,
                              const QString& msgFile,
                              const QString& msgStatusFlags = QString());
protected:
  int m_count_duplicates; //to count all duplicate messages
  MailImporter::FilterInfo *m_filterInfo;
  
private: 
  bool doAddMessage( const QString& folderName,
                     const QString& msgPath,
                     bool duplicateCheck = false );
  class Private;
  Private *const d;
};



/**
 * Glorified QString[N] for (a) understandability (b) older gcc compatibility.
 */
template <unsigned int size> class FolderStructureBase
{
  public:
  typedef QString NString[size];
  /** Constructor. Need a default constructor for QValueList. */
  FolderStructureBase() {} 

  /** Constructor. Turn N QStrings into a folder structure
   *   description.
   */
  FolderStructureBase(const NString &s)
    {
      for(unsigned int i=0; i<size; i++) d[i]=s[i];
    } 

  /** Copy Constructor. */
  FolderStructureBase(const FolderStructureBase &s)
    {
      for(unsigned int i=0; i<size; i++) d[i]=s[i];
    } 

  /** Assignment operator. Does the same thing as
   *   the copy constructor.
   */
  FolderStructureBase &operator =(const FolderStructureBase &s)
    {
      for(unsigned int i=0; i<size; i++) d[i]=s[i];
      return *this;
    } 

  /** Access the different fields. There doesn't seem to
   *   be a real semantics for the fields.
   */
  const QString operator [](unsigned int i) const
    {
      if (i<size) return d[i]; else return QString();
    } 

  /** Access the different fields, for writing. */
  QString &operator [](unsigned int i)
    {
      Q_ASSERT(i<size);
      if (i<size) return d[i]; else return d[0];
    } 
  private:
  QString d[size];
};
}

#endif

