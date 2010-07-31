/*
    This file is part of KDE.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
    
    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "resultelement.h"

#include <serializer.h>
#include <tqstring.h>
#include <directorycategory.h>

void ResultElement::setSummary( TQString* summary )
{
  mSummary = summary;
}

TQString*ResultElement::summary() const
{
   return mSummary;
}

void ResultElement::setURL( TQString* uRL )
{
  mURL = uRL;
}

TQString*ResultElement::uRL() const
{
   return mURL;
}

void ResultElement::setSnippet( TQString* snippet )
{
  mSnippet = snippet;
}

TQString*ResultElement::snippet() const
{
   return mSnippet;
}

void ResultElement::setTitle( TQString* title )
{
  mTitle = title;
}

TQString*ResultElement::title() const
{
   return mTitle;
}

void ResultElement::setCachedSize( TQString* cachedSize )
{
  mCachedSize = cachedSize;
}

TQString*ResultElement::cachedSize() const
{
   return mCachedSize;
}

void ResultElement::setRelatedInformationPresent( bool* relatedInformationPresent )
{
  mRelatedInformationPresent = relatedInformationPresent;
}

bool*ResultElement::relatedInformationPresent() const
{
   return mRelatedInformationPresent;
}

void ResultElement::setHostName( TQString* hostName )
{
  mHostName = hostName;
}

TQString*ResultElement::hostName() const
{
   return mHostName;
}

void ResultElement::setDirectoryCategory( DirectoryCategory* directoryCategory )
{
  mDirectoryCategory = directoryCategory;
}

DirectoryCategory*ResultElement::directoryCategory() const
{
   return mDirectoryCategory;
}

void ResultElement::setDirectoryTitle( TQString* directoryTitle )
{
  mDirectoryTitle = directoryTitle;
}

TQString*ResultElement::directoryTitle() const
{
   return mDirectoryTitle;
}

ResultElement::ResultElement()
{
  mSummary = 0;
  mURL = 0;
  mSnippet = 0;
  mTitle = 0;
  mCachedSize = 0;
  mRelatedInformationPresent = 0;
  mHostName = 0;
  mDirectoryCategory = 0;
  mDirectoryTitle = 0;
}

ResultElement::~ResultElement()
{
  delete mSummary;
  mSummary = 0;
  delete mURL;
  mURL = 0;
  delete mSnippet;
  mSnippet = 0;
  delete mTitle;
  mTitle = 0;
  delete mCachedSize;
  mCachedSize = 0;
  delete mRelatedInformationPresent;
  mRelatedInformationPresent = 0;
  delete mHostName;
  mHostName = 0;
  delete mDirectoryCategory;
  mDirectoryCategory = 0;
  delete mDirectoryTitle;
  mDirectoryTitle = 0;
}


