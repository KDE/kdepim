/* -*- mode: C++; c-file-style: "gnu" -*-
 * KMComposeWin Header File
 * Author: Markus Wuebben <markus.wuebben@kde.org>
 */
#ifndef __KMAIL_KMLINEEDITSPELL_H__
#define __KMAIL_KMLINEEDITSPELL_H__

#include <libkdepim/addresseelineedit.h>

class TQPopupMenu;

class KMLineEdit : public KPIM::AddresseeLineEdit
{
    Q_OBJECT
public:
    KMLineEdit(bool useCompletion, TQWidget *parent = 0,
               const char *name = 0);

signals:
    void focusUp();
    void focusDown();

protected:
    // Inherited. Always called by the parent when this widget is created.
    virtual void loadContacts();

    virtual void keyPressEvent(TQKeyEvent*);

    virtual TQPopupMenu *createPopupMenu();

private slots:
    void editRecentAddresses();

private:
    void dropEvent( TQDropEvent *event );
    void insertEmails( const TQStringList & emails );
};


class KMLineEditSpell : public KMLineEdit
{
    Q_OBJECT
public:
    KMLineEditSpell(bool useCompletion, TQWidget *parent = 0,
               const char *name = 0);
    void highLightWord( unsigned int length, unsigned int pos );
    void spellCheckDone( const TQString &s );
    void spellCheckerMisspelling( const TQString &text, const TQStringList &, unsigned int pos);
    void spellCheckerCorrected( const TQString &old, const TQString &corr, unsigned int pos);

 signals:
  void subjectTextSpellChecked();
};

#endif // __KMAIL_KMLINEEDITSPELL_H__
