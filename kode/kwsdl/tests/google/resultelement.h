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
#ifndef RESULTELEMENT_H
#define RESULTELEMENT_H

class QString;
class DirectoryCategory;

class ResultElement
{
  public:
    void setSummary( TQString* summary );
    TQString*summary() const;
    void setURL( TQString* uRL );
    TQString*uRL() const;
    void setSnippet( TQString* snippet );
    TQString*snippet() const;
    void setTitle( TQString* title );
    TQString*title() const;
    void setCachedSize( TQString* cachedSize );
    TQString*cachedSize() const;
    void setRelatedInformationPresent( bool* relatedInformationPresent );
    bool*relatedInformationPresent() const;
    void setHostName( TQString* hostName );
    TQString*hostName() const;
    void setDirectoryCategory( DirectoryCategory* directoryCategory );
    DirectoryCategory*directoryCategory() const;
    void setDirectoryTitle( TQString* directoryTitle );
    TQString*directoryTitle() const;
    ResultElement();
    ~ResultElement();
  
  private:
    TQString*mSummary;
    TQString*mURL;
    TQString*mSnippet;
    TQString*mTitle;
    TQString*mCachedSize;
    bool*mRelatedInformationPresent;
    TQString*mHostName;
    DirectoryCategory*mDirectoryCategory;
    TQString*mDirectoryTitle;
};

#endif
