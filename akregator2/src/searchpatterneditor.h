/*
    This file is part of Akregator2.

    Copyright (C) 2012 Dan Vrátil <dvratil@redhat.com>

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

#ifndef AKREGATOR2_SEARCHPATTERN_H
#define AKREGATOR2_SEARCHPATTERN_H

#include <QWidget>
#include <Nepomuk2/Query/Term>
#include <KUrl>

class QVBoxLayout;
class QButtonGroup;
class QRadioButton;

namespace Akregator2 {

class SearchPatternRow;

class SearchPatternEditor : public QWidget
{
    Q_OBJECT

  public:
    explicit SearchPatternEditor( QWidget* parent = 0 );
    virtual ~SearchPatternEditor();

    void setPattern( const QVariant &pattern );
    QVariant pattern() const;

    void clear();

    Nepomuk2::Query::Term negateTerm( const Nepomuk2::Query::Term &term );
    QString nepomukQuery( const KUrl::List &parents ) const;

  private Q_SLOTS:
    void slotAddPatternRow();
    void slotRemovePatternRow();

  private:
    void updatePatternsButtons();

    QVBoxLayout *m_mainLayout;
    QList<SearchPatternRow*> m_patterns;
    QButtonGroup *m_operator;

};

} // namespace Akregator2

#endif // AKREGATOR2_SEARCHPATTERN_H
