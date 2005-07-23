/*
    cryptoconfigmodule_p.h

    This file is part of libkleopatra
    Copyright (c) 2004,2005 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

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

#ifndef CRYPTOCONFIGMODULE_P_H
#define CRYPTOCONFIGMODULE_P_H

#include <qtabwidget.h>
#include <qhbox.h>
#include <qcheckbox.h>
#include <kurl.h>

class KLineEdit;
class KIntNumInput;
class KURLRequester;
class QPushButton;
class QGridLayout;

namespace Kleo {

  class CryptoConfig;
  class CryptoConfigComponent;
  class CryptoConfigGroup;
  class CryptoConfigEntry;
  class CryptoConfigComponentGUI;
  class CryptoConfigGroupGUI;
  class CryptoConfigEntryGUI;

  /**
   * A widget corresponding to a component in the crypto config
   */
  class CryptoConfigComponentGUI : public QWidget {
    Q_OBJECT

  public:
    CryptoConfigComponentGUI( CryptoConfigModule* module, Kleo::CryptoConfigComponent* component,
                              QWidget* parent, const char* name = 0 );

    bool save();
    void load();
    void defaults();

  private:
    Kleo::CryptoConfigComponent* mComponent;
    QValueList<CryptoConfigGroupGUI *> mGroupGUIs;
  };

  /**
   * A class managing widgets corresponding to a group in the crypto config
   */
  class CryptoConfigGroupGUI : public QObject {
    Q_OBJECT

  public:
    CryptoConfigGroupGUI( CryptoConfigModule* module, Kleo::CryptoConfigGroup* group,
                          QGridLayout * layout, QWidget* parent, const char* name = 0 );

    bool save();
    void load();
    void defaults();

  private:
    Kleo::CryptoConfigGroup* mGroup;
    QValueList<CryptoConfigEntryGUI *> mEntryGUIs;
  };

  /**
   * Factory for CryptoConfigEntryGUI instances
   * Not a real factory, but can become one later.
   */
  class CryptoConfigEntryGUIFactory {
  public:
    static CryptoConfigEntryGUI* createEntryGUI(
      CryptoConfigModule* module,
      Kleo::CryptoConfigEntry* entry, const QString& entryName,
      QGridLayout * layout, QWidget* widget, const char* name = 0 );
  };

  /**
   * Base class for the widget managers tied to an entry in the crypto config
   */
  class CryptoConfigEntryGUI : public QObject {
    Q_OBJECT
  public:
    CryptoConfigEntryGUI( CryptoConfigModule* module,
                          Kleo::CryptoConfigEntry* entry,
                          const QString& entryName,
                          const char* name = 0 );
    virtual ~CryptoConfigEntryGUI() {}

    void load() { doLoad(); mChanged = false; }
    void save() { Q_ASSERT( mChanged ); doSave(); mChanged = false; }
    void resetToDefault();

    QString description() const;
    bool isChanged() const { return mChanged; }

  signals:
    void changed();

  protected slots:
    void slotChanged() {
      mChanged = true;
      emit changed();
    }

  protected:
    virtual void doSave() = 0;
    virtual void doLoad() = 0;

    Kleo::CryptoConfigEntry* mEntry;
    QString mName;
    bool mChanged;
  };

  /**
   * A widget manager for a string entry in the crypto config
   */
  class CryptoConfigEntryLineEdit : public CryptoConfigEntryGUI {
    Q_OBJECT

  public:
    CryptoConfigEntryLineEdit( CryptoConfigModule* module,
                               Kleo::CryptoConfigEntry* entry,
                               const QString& entryName,
                               QGridLayout * layout,
                               QWidget* parent, const char* name = 0 );

    virtual void doSave();
    virtual void doLoad();
  private:
    KLineEdit* mLineEdit;
  };

  /**
   * A widget manager for a path entry in the crypto config
   */
  class CryptoConfigEntryPath : public CryptoConfigEntryGUI {
    Q_OBJECT

  public:
    CryptoConfigEntryPath( CryptoConfigModule* module,
                           Kleo::CryptoConfigEntry* entry,
                           const QString& entryName,
                           QGridLayout * layout,
                           QWidget* parent, const char* name = 0 );

    virtual void doSave();
    virtual void doLoad();
  private:
    KURLRequester* mUrlRequester;
  };

  /**
   * A widget manager for a directory path entry in the crypto config
   */
  class CryptoConfigEntryDirPath : public CryptoConfigEntryGUI {
    Q_OBJECT

  public:
    CryptoConfigEntryDirPath( CryptoConfigModule* module,
                              Kleo::CryptoConfigEntry* entry,
                              const QString& entryName,
                              QGridLayout * layout,
                              QWidget* parent, const char* name = 0 );

    virtual void doSave();
    virtual void doLoad();
  private:
    KURLRequester* mUrlRequester;
  };

  /**
   * A widget manager for an URL entry in the crypto config
   */
  class CryptoConfigEntryURL : public CryptoConfigEntryGUI {
    Q_OBJECT

  public:
    CryptoConfigEntryURL( CryptoConfigModule* module,
                          Kleo::CryptoConfigEntry* entry,
                          const QString& entryName,
                          QGridLayout * layout,
                          QWidget* parent, const char* name = 0 );

    virtual void doSave();
    virtual void doLoad();
  private:
    KURLRequester * mUrlRequester;
  };

  /**
   * A widget manager for an int/uint entry in the crypto config
   */
  class CryptoConfigEntrySpinBox : public CryptoConfigEntryGUI {
    Q_OBJECT

  public:
    CryptoConfigEntrySpinBox( CryptoConfigModule* module,
                              Kleo::CryptoConfigEntry* entry,
                              const QString& entryName,
                              QGridLayout * layout,
                              QWidget* parent, const char* name = 0 );
    virtual void doSave();
    virtual void doLoad();
  private:
    enum { Int, UInt, ListOfNone } mKind;
    KIntNumInput* mNumInput;
  };

  /**
   * A widget manager for a bool entry in the crypto config
   */
  class CryptoConfigEntryCheckBox : public CryptoConfigEntryGUI {
    Q_OBJECT

  public:
    CryptoConfigEntryCheckBox( CryptoConfigModule* module,
                               Kleo::CryptoConfigEntry* entry,
                               const QString& entryName,
                               QGridLayout * layout,
                               QWidget* parent, const char* name = 0 );
    virtual void doSave();
    virtual void doLoad();
  private:
    QCheckBox* mCheckBox;
  };

  /**
   * A widget manager for a bool entry in the crypto config
   */
  class CryptoConfigEntryLDAPURL : public CryptoConfigEntryGUI {
    Q_OBJECT

  public:
    CryptoConfigEntryLDAPURL( CryptoConfigModule* module,
                              Kleo::CryptoConfigEntry* entry,
                              const QString& entryName,
                              QGridLayout * layout,
                              QWidget* parent, const char* name = 0 );
    virtual void doSave();
    virtual void doLoad();
  private slots:
    void slotOpenDialog();
  private:
    void setURLList( const KURL::List& urlList );
    QLabel* mLabel;
    QPushButton* mPushButton;
    KURL::List mURLList;
  };
}

#endif // CRYPTOCONFIGMODULE_P_H
