/*
    qgpgmejob.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

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

#ifndef __KLEO_QGPGMEJOB_H__
#define __KLEO_QGPGMEJOB_H__

#include <gpgmepp/interfaces/progressprovider.h>
#include <gpgmepp/interfaces/passphraseprovider.h>

#include <gpgmepp/key.h>

#include <qcstring.h>

#include <vector>
#include <kdepimmacros.h>

namespace GpgME {
  class Error;
  class Context;
  class Data;
}

namespace Kleo {
  class Job;
}

namespace QGpgME {
  class QByteArrayDataProvider;
}

class QString;
class QStringList;

namespace Kleo {

  /** This is a hackish helper class to avoid code duplication in this
      backend's Kleo::Job subclasses. It contains several workarounds
      for moc/signal/slot shortcomings, most of which the author of
      this thinks are Qt bugs (lazy implementations), first and
      foremost the inability of moc to handle inheritance from
      multiple QObject-derived subclasses.

      To use it, inherit from the Job-subclass, then from this class,
      add QGPGME_JOB to just after Q OBJECT and implement
      doOperationDoneEvent() by emitting your variant of the result()
      signal there. Pass "this" as the first argument this QGpgMEJOb's
      ctor. The rest is dealt with automatically.
  */
  class KDE_EXPORT QGpgMEJob : public GpgME::ProgressProvider, public GpgME::PassphraseProvider {
  public:
    QGpgMEJob( Kleo::Job * _this, GpgME::Context * context );
    ~QGpgMEJob();

  protected:
    /*! Called on operation-done events, between emitting done() and
      calling deleteLater(). You should emit your result signal here. */
    virtual void doOperationDoneEvent( const GpgME::Error & e ) = 0;
    /*! Hooks up mCtx to be managed by the event loop interactor */
    void hookupContextToEventLoopInteractor();
    /*! Fills mPatterns from the stringlist, resets chunking to the full list */
    void setPatterns( const QStringList & sl, bool allowEmpty=false );
    /*! Returnes the number of patterns set */
    unsigned int numPatterns() const { return mNumPatterns; }
    /*! Skips to the next chunk of patterns. @return patterns() */
    const char* * nextChunk();
    /*! @return patterns, offset by the current chunk */
    const char* * patterns() const;
    /*! Set the current pattern chunksize to size and reset the chunk index to zero */
    void setChunkSize( unsigned int size );
    /*! @return current chunksize */
    unsigned int chunkSize() const { return mChunkSize; }
    /*! Creates an empty GpgME::Data/QGpgME::QByteArrayDataProvider pair */
    void createOutData();
    /*! Creates a GpgME::Data/QGpgME::QByteArrayDataProvider pair,
      filled with the contents of \a in */
    void createInData( const QByteArray & in );
    /*! Sets the list of signing keys */
    GpgME::Error setSigningKeys( const std::vector<GpgME::Key> & signers );
    /*! Call this to implement a slotOperationDoneEvent() */
    void doSlotOperationDoneEvent( GpgME::Context * context, const GpgME::Error & e );

    //
    // only boring stuff below this line...
    //

  protected:
    virtual void doEmitProgressSignal( const QString & what, int current, int total ) = 0;
    virtual void doEmitDoneSignal() = 0;
    void doSlotCancel();

  private:
    /*! \reimp from GpgME::ProgressProvider */
    void showProgress( const char * what, int type, int current, int total );
    char * getPassphrase( const char * useridHint, const char * description,
			  bool previousWasBad, bool & canceled );
    void deleteAllPatterns();

  public:
    void checkInvariants() const;

  protected:
    Kleo::Job * mThis;
    GpgME::Context * mCtx;
    GpgME::Data * mInData;
    QGpgME::QByteArrayDataProvider * mInDataDataProvider;
    GpgME::Data * mOutData;
    QGpgME::QByteArrayDataProvider * mOutDataDataProvider;
  private:
    const char* * mPatterns;
    // holds the entry - if any - in mPattern that was replaced with
    // NULL to create a temporary end-of-array marker for gpgme:
    const char * mReplacedPattern;
    unsigned int mNumPatterns;
    unsigned int mChunkSize;
    unsigned int mPatternStartIndex, mPatternEndIndex;
  };

}

#define make_slot_cancel private: void slotCancel() { QGpgMEJob::doSlotCancel(); }
#define make_progress_emitter private: void doEmitProgressSignal( const QString & what, int cur, int tot ) { emit progress( what, cur, tot ); }
#define make_done_emitter private: void doEmitDoneSignal() { emit done(); }
#define QGPGME_JOB make_slot_cancel make_progress_emitter make_done_emitter

#endif // __KLEO_QGPGMEJOB_H__
