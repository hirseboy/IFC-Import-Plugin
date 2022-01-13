#ifndef IMPORTWPREAD_H
#define IMPORTWPREAD_H

#include <QWizardPage>

namespace IFCC {
	class IFCReader;
}

namespace Ui {
class ImportWPRead;
}

class ImportWPRead : public QWizardPage
{
	Q_OBJECT

public:
	explicit ImportWPRead(QWidget *parent, IFCC::IFCReader* reader);
	~ImportWPRead();

	/*! Checks for valid inputs. */
	virtual bool isComplete() const override;

protected:

private slots:

	void on_toolButtonOpenIFCFile_clicked();

	void on_pushButtonRead_clicked();

private:
	Ui::ImportWPRead *ui;

	IFCC::IFCReader*	m_reader;

	bool				m_readSuccessfully;
};

#endif // IMPORTWPREAD_H
