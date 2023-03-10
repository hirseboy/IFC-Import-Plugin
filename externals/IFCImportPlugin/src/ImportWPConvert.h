#ifndef ImportWPConvert_H
#define ImportWPConvert_H

#include <QWizardPage>

namespace IFCC {
	class IFCReader;
}

namespace Ui {
class ImportWPConvert;
}

class ImportWPConvert : public QWizardPage
{
	Q_OBJECT

public:
	explicit ImportWPConvert(QWidget *parent, IFCC::IFCReader* reader);
	~ImportWPConvert();

	/*! Checks for valid inputs. */
	virtual bool isComplete() const override;

protected:

private slots:

	void on_pushButtonConvert_clicked();

	void on_checkBoxRemoveDoubleSBs_clicked();

	void on_checkBoxIgnorErrors_toggled(bool checked);

	void on_checkBoxSpaceIntersectionDetails_clicked();

private:
	void initializePage() override;

	/*! Create text from convert results and add it to the edit field.*/
	void setText();

	Ui::ImportWPConvert *ui;

	IFCC::IFCReader*	m_reader;

	bool m_convertSuccessfully;
};

#endif // ImportWPConvert_H
