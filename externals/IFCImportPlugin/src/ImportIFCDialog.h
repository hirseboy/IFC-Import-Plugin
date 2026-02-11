#ifndef ImportIFCDialogH
#define ImportIFCDialogH

#include <QDialog>

#include <IFCC_Types.h>

namespace IFCC {
	class IFCReader;
}

namespace Ui {
class ImportIFCDialog;
}

class ImportIFCDialog : public QDialog
{
	Q_OBJECT

public:

	/*! Predefined conversion scenarios that map to specific ConvertOptions settings. */
	enum ConversionScenario {
		CS_UseSpaceBoundaries,		///< Use space boundaries from the IFC file directly.
		CS_FullMatching,			///< Check every construction element for matches with space surfaces.
		CS_MediumMatching,			///< Check only the highest-priority construction for each space surface.
		CS_NoMatching				///< Create space boundaries without construction matching.
	};

	explicit ImportIFCDialog(QWidget *parent, IFCC::IFCReader* reader);

	~ImportIFCDialog();

	void setFilename(QString filename);

private slots:

	void on_toolButtonOpenIFCFile_clicked();

	void on_lineEditIFCFile_textChanged(const QString &arg1);

	void on_pushButtonConvert_clicked();

	void on_checkBoxRemoveDoubleSBs_clicked();

	void on_radioButtonMatchingFull_clicked();

	void on_radioButtonMatchingFirst_clicked();

	void on_radioButtonMatchingNConstructions_clicked();

	void on_radioButtonMatchingNo_clicked();

	void on_radioButtonScenarioSB_clicked();

	void on_radioButtonScenarioFull_clicked();

	void on_radioButtonScenarioMedium_clicked();

	void on_radioButtonScenarioNone_clicked();

	void on_checkBoxAdvanced_clicked();

private:
	bool read();

	/*! Create text from convert results and add it to the edit field.*/
	void setText();

	void initElements();

	/*! Update the scenario description label and configure advanced tab defaults. */
	void setScenario(ConversionScenario scenario);

	QString elementTypeText(IFCC::BuildingElementTypes type) const;

	void initConvertOptions();

	Ui::ImportIFCDialog *ui;

	IFCC::IFCReader*	m_reader;
	ConversionScenario	m_scenario = CS_MediumMatching;
	bool				m_convertSuccessfully = false;
};

#endif // ImportIFCDialogH
