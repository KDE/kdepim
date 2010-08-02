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
#ifndef GOOGLESEARCHRESULT_H
#define GOOGLESEARCHRESULT_H

class TQString;
class ResultElementArray;
class DirectoryCategoryArray;

class GoogleSearchResult
{
  public:
    void setDocumentFiltering( bool* documentFiltering );
    bool*documentFiltering() const;
    void setSearchComments( TQString* searchComments );
    TQString*searchComments() const;
    void setEstimatedTotalResultsCount( int* estimatedTotalResultsCount );
    int*estimatedTotalResultsCount() const;
    void setEstimateIsExact( bool* estimateIsExact );
    bool*estimateIsExact() const;
    void setResultElements( ResultElementArray* resultElements );
    ResultElementArray*resultElements() const;
    void setSearchQuery( TQString* searchQuery );
    TQString*searchQuery() const;
    void setStartIndex( int* startIndex );
    int*startIndex() const;
    void setEndIndex( int* endIndex );
    int*endIndex() const;
    void setSearchTips( TQString* searchTips );
    TQString*searchTips() const;
    void setDirectoryCategories( DirectoryCategoryArray* directoryCategories );
    DirectoryCategoryArray*directoryCategories() const;
    void setSearchTime( double* searchTime );
    double*searchTime() const;
    GoogleSearchResult();
    ~GoogleSearchResult();
  
  private:
    bool*mDocumentFiltering;
    TQString*mSearchComments;
    int*mEstimatedTotalResultsCount;
    bool*mEstimateIsExact;
    ResultElementArray*mResultElements;
    TQString*mSearchQuery;
    int*mStartIndex;
    int*mEndIndex;
    TQString*mSearchTips;
    DirectoryCategoryArray*mDirectoryCategories;
    double*mSearchTime;
};

#endif
