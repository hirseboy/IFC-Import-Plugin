#ifndef ExportWPReadH
#define ExportWPReadH

#include <QWizardPage>

#include "GEGZone.h"

namespace VICUS {
	class Project;
}

namespace Ui {
class ExportWPRead;
}

class ExportWPRead : public QWizardPage
{
	Q_OBJECT

public:
	explicit ExportWPRead(QWidget *parent, VICUS::Project* project);
	~ExportWPRead();

	/*! Checks for valid inputs. */
	virtual bool isComplete() const override;

protected:

private slots:

	void on_checkBoxCollectZones_clicked();

	void on_pushButtonExport_clicked();

private:
	Ui::ExportWPRead *ui;
	VICUS::Project*			m_project;
	std::vector<GEGZone>	m_zones;
	QStringList				m_errors;

	bool update();
};

#endif // ExportWPReadH
