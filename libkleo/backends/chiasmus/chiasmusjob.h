/*  -*- mode: C++; c-file-style: "gnu" -*-
    chiasmusjob.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2005 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
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


#ifndef __KLEO_CHIASMUSJOB_H__
#define __KLEO_CHIASMUSJOB_H__

#include "kleo/specialjob.h"

#include <QStringList>

#include <gpgme++/context.h>


namespace Kleo {

  class SymCryptRunProcessBase;

  /**
     @short SpecialJob for Chiasmus operations
  */
  class ChiasmusJob : public Kleo::SpecialJob {
    Q_OBJECT
    Q_ENUMS( Mode )
    Q_PROPERTY( Mode mode READ mode )
    Q_PROPERTY( QString key READ key WRITE setKey )
    Q_PROPERTY( QString options READ options WRITE setOptions )
    Q_PROPERTY( QByteArray input READ input WRITE setInput )
    Q_PROPERTY( QByteArray result READ result )
  public:
    enum Mode {
      Encrypt, Decrypt
    };
    explicit ChiasmusJob( Mode op );
    ~ChiasmusJob();

    /*!\reimp SpecialJob */
    GpgME::Error start();
    /*!\reimp SpecialJob */
    GpgME::Error exec();

    /*!\reimp Kleo::Job */
    void showErrorDialog( QWidget *, const QString & ) const;

    Mode mode() const { return mMode; }

    QString key() const { return mKey; }
    void setKey( const QString & key ) { mKey = key; }

    QString options() const { return mOptions; }
    void setOptions( const QString & options ) { mOptions = options; }

    QByteArray input() const { return mInput; }
    void setInput( const QByteArray & input ) { mInput = input; }

    //using SpecialJob::result;
    QByteArray result() const { return mOutput; }

  public slots:
    void slotCancel();

  private slots:
    GpgME::Error finished();
    void slotTimeout();

  private:
    bool checkPreconditions() const;
    GpgME::Error setup();
    void showChiasmusOutput();

  private:
    SymCryptRunProcessBase * mSymCryptRun;
    QString mKey;
    QString mOptions;
    QByteArray mInput, mOutput;
    GpgME::Error mError;
    QString mStderr;
    bool mCanceled;
    bool mTimeout;
    const Mode mMode;
  };

}


#endif // __KLEO_CHIASMUSJOB_H__
