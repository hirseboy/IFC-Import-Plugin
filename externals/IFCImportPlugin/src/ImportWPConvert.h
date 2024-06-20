#ifndef ImportWPConvert_H
#define ImportWPConvert_H

#include <QWizardPage>

#include <IFCC_Types.h>


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

	enum MatchingMainType {
		MMT_FullMatching,
		MMT_MediumMatching,
		MMT_NoMatching
	};

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

	void on_pushButtonSaveLog_clicked();

	void on_radioButtonMatchingFull_clicked();

	void on_radioButtonMatchingFirst_clicked();

	void on_radioButtonMatchingNConstructions_clicked();

	void on_radioButtonMatchingNo_clicked();

	void on_checkBoxUseSpaceBoundaries_clicked();

	void on_checkBoxAdvancedSettings_toggled(bool checked);

	void on_comboBoxMatchingType_currentIndexChanged(int index);

private:
	void initializePage() override;

	/*! Create text from convert results and add it to the edit field.*/
	void setText();

	void initElements();

	void setMatching(MatchingMainType type);

	QString elementTypeText(IFCC::BuildingElementTypes type) const;

	Ui::ImportWPConvert *ui;

	IFCC::IFCReader*	m_reader;

	bool m_convertSuccessfully;

	MatchingMainType	m_matchingType = MMT_MediumMatching;
};

#endif // ImportWPConvert_H
