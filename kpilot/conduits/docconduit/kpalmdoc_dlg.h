#ifndef CONVERTERDLG_H
#define CONVERTERDLG_H

#include <kdialogbase.h>
class ConverterDlgBase;
class DOCConverter;

class ConverterDlg : public KDialogBase
{
    Q_OBJECT

public:
	ConverterDlg( QWidget *parent=0, const QString& caption=0);
	~ConverterDlg();

protected slots:
	virtual void slotClose();
	void slotToText();
	void slotToDoc();
	void slotDirectories(bool dir);
	void slotUser1();
protected:
	void writeSettings();
	void readSettings();

	// These two functions convert one single file to or from a pdb database
	bool convertTextToPDB(QString docdir, QString docfile,
		QString pdbdir, QString pdbfile, DOCConverter*conv);
	bool convertPDBToText(QString pdbdir, QString pdbfile,
		QString docdir, QString docfile, DOCConverter*conv);


	// The actual dialog widget (designer created) holding all controls
	ConverterDlgBase*dlg;
	// Settings
	bool askOverwrite;
	bool verbose;
};

#endif // CONVERTERDLG_H
