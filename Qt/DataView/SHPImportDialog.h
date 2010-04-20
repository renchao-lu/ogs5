/**
 * \file SHPImportDialog.h
 * 25/01/2010 KR Initial implementation
 */

#ifndef SHPIMPORTDIALOG_H
#define SHPIMPORTDIALOG_H

#include <QtGui/QMainWindow>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QRadioButton>

class SHPInterface;
class GEOModels;

/**
 * \brief Dialog for selecting which information should be loaded from a shape file.
 */
class SHPImportDialog : public QDialog
{
	Q_OBJECT

public:
	/// Constructor
	SHPImportDialog(std::string filename, GEOModels* geoModels, QDialog* parent = 0);
	~SHPImportDialog();

	QDialogButtonBox* _buttonBox;	/// The buttons used in this dialog.


private:
	/// Constructs a dialog window based on the information found in the selected shape file
	void setupDialog();

	QLineEdit *_listName;
	QRadioButton *_choice1, *_choice2;;
	std::string _filename;
	short _fileType;
	SHPInterface* _shpInterface;

private slots:
	/// Instructions if the OK-Button has been pressed.
	void accept();

	/// Instructions if the Cancel-Button has been pressed.
	void reject();

signals:
	void shpLoaded(QString);
};

#endif //SHPIMPORTDIALOG_H
