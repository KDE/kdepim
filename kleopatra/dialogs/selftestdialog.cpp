/* -*- mode: c++; c-basic-offset:4 -*-
    dialogs/selftestdialog.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include <config-kleopatra.h>

#include "selftestdialog.h"

#include "ui_selftestdialog.h"

#include <selftest/selftest.h>

#include <QAbstractTableModel>

#include <boost/shared_ptr.hpp>

#include <cassert>
#include <vector>

using namespace Kleo;
using namespace Kleo::Dialogs;
using namespace boost;

namespace {

    class Model : public QAbstractTableModel {
        Q_OBJECT
    public:
        explicit Model( QObject * parent=0 )
            : QAbstractTableModel( parent ),
              m_tests()
        {

        }

        enum Column {
            TestName,
            TestResult,

            NumColumns
        };

        const shared_ptr<SelfTest> & fromModelIndex( const QModelIndex & idx ) const {
            const unsigned int row = idx.row();
            if ( row < m_tests.size() )
                return m_tests[row];
            static const shared_ptr<SelfTest> null;
            return null;
        }

        /* reimp */ int rowCount( const QModelIndex & idx ) const { return idx.isValid() ? 0 : m_tests.size() ; }
        /* reimp */ int columnCount( const QModelIndex & ) const { return NumColumns; }

        /* reimp */ QVariant data( const QModelIndex & idx, int role ) const {
            const unsigned int row = idx.row();
            if ( idx.isValid() && row < m_tests.size() )
                switch ( role ) {
                case Qt::DisplayRole:
                case Qt::ToolTipRole:
                    switch ( idx.column() ) {
                    case TestName:
                        return m_tests[row]->name();
                    case TestResult:
                        if ( m_tests[row]->passed() )
                            return i18n("Passed");
                        else
                            if ( role == Qt::ToolTipRole )
                                return m_tests[row]->longError();
                            else
                                return m_tests[row]->shortError();
                    }
                    break;
                case Qt::BackgroundRole:
                    return QColor( m_tests[row]->passed() ? Qt::green : Qt::red );
                }
            return QVariant();
        }

        /* reimp */ QVariant headerData( int section, Qt::Orientation o, int role ) const {
            if ( o == Qt::Horizontal &&
                 section >= 0 && section < NumColumns &&
                 role == Qt::DisplayRole )
                switch ( section ) {
                case TestName:   return i18n("Test Name");
                case TestResult: return i18n("Result");
                }
            return QVariant();
        }

        void clear() {
            if ( m_tests.empty() )
                return;
            beginRemoveRows( QModelIndex(), 0, m_tests.size() - 1 );
            m_tests.clear();
            endRemoveRows();
        }

        void append( const std::vector< shared_ptr<SelfTest> > & tests ) {
            if ( tests.empty() )
                return;
            beginInsertRows( QModelIndex(), m_tests.size(), m_tests.size() + tests.size() );
            m_tests.insert( m_tests.end(), tests.begin(), tests.end() );
            endInsertRows();
        }

        void reloadData() {
            if ( !m_tests.empty() )
                emit dataChanged( index( 0, 0 ), index( m_tests.size() - 1, NumColumns - 1 ) );
        }

        const shared_ptr<SelfTest> & at( unsigned int idx ) const {
            return m_tests.at( idx );
        }

    private:
        std::vector< shared_ptr<SelfTest> > m_tests;
    };

}

class SelfTestDialog::Private {
    friend class ::Kleo::Dialogs::SelfTestDialog;
    SelfTestDialog * const q;
public:
    explicit Private( SelfTestDialog * qq )
        : q( qq ),
          ui( q )
    {
        ui.resultsTV->setModel( &model );

        connect( ui.resultsTV->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                 q, SLOT(slotSelectionChanged()) );
    }

private:
    void slotSelectionChanged() {
        const int row = selectedRowIndex();
        if ( row < 0 ) {
            ui.proposedCorrectiveActionLB->setText( i18n("(select test first)" ) );
            ui.proposedCorrectiveActionGB->setEnabled( false );
        } else {
            ui.proposedCorrectiveActionGB->setEnabled( true );
            ui.proposedCorrectiveActionLB->setText( model.at(row)->proposedFix() );
            ui.doItPB->setEnabled( !model.at(row)->passed() && model.at(row)->canFixAutomatically() );
        }
    }
    void slotDoItClicked() {
        if ( const shared_ptr<SelfTest> st = model.fromModelIndex( selectedRow() ) )
            if ( st->fix() )
                model.reloadData();
    }

private:
    QModelIndex selectedRow() const {
        const QItemSelectionModel * const ism = ui.resultsTV->selectionModel();
        if ( !ism )
            return QModelIndex();
        const QModelIndexList mil = ism->selectedRows();
        return mil.empty() ? QModelIndex() : mil.front() ;
    }
    int selectedRowIndex() const {
        return selectedRow().row();
    }

private:
    Model model;

    struct UI : public Ui_SelfTestDialog {

        QPushButton * rerunPB;

        explicit UI( SelfTestDialog * qq )
            : Ui_SelfTestDialog(),
              rerunPB( new QPushButton( i18n("Rerun Tests") ) )
        {
            setupUi( qq );

            buttonBox->addButton( rerunPB, QDialogButtonBox::ActionRole );
            buttonBox->button( QDialogButtonBox::Ok )->setText( i18n("Continue") );

            connect( rerunPB, SIGNAL(clicked()),
                     qq, SIGNAL(updateRequested()) );
        }
    } ui;
};

SelfTestDialog::SelfTestDialog( QWidget * p, Qt::WindowFlags f )
    : QDialog( p, f ), d( new Private( this ) )
{
    setAutomaticMode( false );
}

SelfTestDialog::SelfTestDialog( const std::vector< shared_ptr<SelfTest> > & tests, QWidget * p, Qt::WindowFlags f )
    : QDialog( p, f ), d( new Private( this ) )
{
    addSelfTests( tests );
    setAutomaticMode( false );
}

SelfTestDialog::~SelfTestDialog() {}

void SelfTestDialog::clear() {
    d->model.clear();
}

void SelfTestDialog::addSelfTest( const shared_ptr<SelfTest> & test ) {
    d->model.append( std::vector< shared_ptr<SelfTest> >( 1, test ) );
}

void SelfTestDialog::addSelfTests( const std::vector< shared_ptr<SelfTest> > & tests ) {
    d->model.append( tests );
}

void SelfTestDialog::setRunAtStartUp( bool on ) {
    d->ui.runAtStartUpCB->setChecked( on );
}

bool SelfTestDialog::runAtStartUp() const {
    return d->ui.runAtStartUpCB->isChecked();
}

void SelfTestDialog::setAutomaticMode( bool automatic ) {
    d->ui.buttonBox->button( QDialogButtonBox::Ok     )->setVisible(  automatic );
    d->ui.buttonBox->button( QDialogButtonBox::Cancel )->setVisible(  automatic );
    d->ui.buttonBox->button( QDialogButtonBox::Close  )->setVisible( !automatic );
}

#include "selftestdialog.moc"
#include "moc_selftestdialog.cpp"
