#ifndef KANDYVIEW_H
#define KANDYVIEW_H

#include <qwidget.h>

class QPainter;
class QTextStream;
class QDataStream;
class QDomDocument;
class QDomElement;
class QTextEdit;
class QListView;

class KURL;

class Modem;
class ATCommand;
class MobileGui;
class CommandScheduler;
class CmdPropertiesDialog;

/**
  This is the main view class for Kandy.
 
  @short Main view
  @author Cornelius Schumacher <schumacher@kde.org>
*/
class KandyView : public QWidget
{
    Q_OBJECT
  public:
	/**
	 * Default constructor
	 */
    KandyView(CommandScheduler *,QWidget *parent);

	/**
	 * Destructor
	 */
    virtual ~KandyView();

    /**
      Import phonebook from mobile phone and save it to Kab. This function
      returns before the job is actually done.
    */
    void importPhonebook();

    /**
     * Print this view to any medium -- paper or not
     */
    void print(QPainter *, int height, int width);

    bool loadFile(const QString& filename);
    bool saveFile(const QString& filename);

    void setModified(bool modified=true);
    bool isModified();

  public slots:
    void addCommand();
    void executeCommand();
    void deleteCommand();
    void editCommand();

  signals:
    /**
     * Use this signal to change the content of the statusbar
     */
    void signalChangeStatusbar(const QString& text);

    /**
     * Use this signal to change the content of the caption
     */
    void signalChangeCaption(const QString& text);

    void modifiedChanged(bool);

  protected slots:
    void appendOutput(const char *line);

    void setResult(ATCommand *);

  private slots:
    void slotSetTitle(const QString& title);
    void processLastLine();

  private:
    QString mLastInput;

    CommandScheduler *mScheduler;

    bool mModified;

    QListView *mCommandList;

    QTextEdit *mInput;
    QTextEdit *mOutput;
    QTextEdit *mResultView;
};

#endif // KANDYVIEW_H
