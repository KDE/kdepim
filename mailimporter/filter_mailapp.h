/***************************************************************************
                          filter_mailapp.h  -  OS X Mail App import
                             -------------------
    copyright            : (C) 2004 by Chris Howells
    email                : howells@kde.org

    Derived from code by:
    copyright            : (C) 2003 by Laurence Anderson
    email                : l.d.anderson@warwick.ac.uk

 ***************************************************************************/

#ifndef MAILIMPORTER_FILTER_MAILAPP_HXX
#define MAILIMPORTER_FILTER_MAILAPP_HXX

#include "filters.h"
/**
 *imports mbox archives messages into KMail
 *@author Chris Howells
 */
namespace MailImporter {
class MAILIMPORTER_EXPORT FilterMailApp : public Filter
{
public:
    explicit FilterMailApp();
    ~FilterMailApp();

    void import();
    void importMails( const QString & maildir );
private:
    QStringList mMboxFiles;
    void traverseDirectory(const QString &);
};
}

#endif
