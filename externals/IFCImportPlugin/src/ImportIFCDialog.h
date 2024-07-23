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

	enum MatchingMainType {
		MMT_FullMatching,
		MMT_MediumMatching,
		MMT_NoMatching
	};

	explicit ImportIFCDialog(QWidget *parent, IFCC::IFCReader* reader);
	~ImportIFCDialog();

private slots:

	void on_toolButtonOpenIFCFile_clicked();

	void on_lineEditIFCFile_textChanged(const QString &arg1);

	void on_pushButtonConvert_clicked();

	void on_checkBoxRemoveDoubleSBs_clicked();

	void on_radioButtonMatchingFull_clicked();

	void on_radioButtonMatchingFirst_clicked();

	void on_radioButtonMatchingNConstructions_clicked();

	void on_radioButtonMatchingNo_clicked();

	void on_checkBoxUseSpaceBoundaries_clicked();

	void on_checkBoxAdvanced_clicked();

	void on_comboBoxMatchingType_currentIndexChanged(int index);

private:
	bool read();

	/*! Create text from convert results and add it to the edit field.*/
	void setText();

	void initElements();

	void setMatching(MatchingMainType type);

	QString elementTypeText(IFCC::BuildingElementTypes type) const;

	void initConvertOptions();

	Ui::ImportIFCDialog *ui;

	IFCC::IFCReader*	m_reader;
	MatchingMainType	m_matchingType = MMT_MediumMatching;
	bool				m_convertSuccessfully = false;
};

#endif // ImportIFCDialogH
