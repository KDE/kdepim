/*
    hierarchyanalyser.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "hierarchyanalyser.h"

#include <algorithm>
#include <iterator>

HierarchyAnalyser::HierarchyAnalyser( QObject * parent, const char * name )
  : QObject( parent, name )
{

}

HierarchyAnalyser::~HierarchyAnalyser() {
  
}

void HierarchyAnalyser::slotNextKey( const GpgME::Key & key ) {
  if ( key.isNull() )
    return;
  if ( key.isRoot() || !key.chainID() || !*key.chainID() )
    // root keys have themselves as issuer - we don't want them to
    // have parents, though:
    mSubjectsByIssuer[0].push_back( key );
  else
    mSubjectsByIssuer[key.chainID()].push_back( key );
}

const std::vector<GpgME::Key> & HierarchyAnalyser::subjectsForIssuer( const char * issuer_dn ) const {
  static const std::vector<GpgME::Key> empty;
  std::map< QCString, std::vector<GpgME::Key> >::const_iterator it =
    mSubjectsByIssuer.find( issuer_dn );
  return it == mSubjectsByIssuer.end() ? empty : it->second ;
}

std::vector<GpgME::Key> HierarchyAnalyser::subjectsForIssuerRecursive( const char * issuer_dn ) const {
  std::vector<GpgME::Key> keys = subjectsForIssuer( issuer_dn );
  for ( unsigned int i = 0 ; i < keys.size() ; ++i ) // can't use iterators here, since appending would invalidate them
    if ( const char * fpr = keys[i].primaryFingerprint() ) {
      const std::vector<GpgME::Key> & tmp = subjectsForIssuer( fpr );
      std::copy( tmp.begin(), tmp.end(), std::back_inserter( keys ) );
    }
  return keys;
}


#include "hierarchyanalyser.moc"
