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
	Ui::ExportWPRead *												ui;
	VICUS::Project*													m_project;
	std::vector<GEGZone>											m_zones;
	std::map<int,GEGConstruction>									m_constructions;
	std::map<GEGSurface::GEGSurfaceType, std::vector<GEGSurface>>	m_surfaces;
	QStringList														m_errors;
	QStringList														m_warnings;

	bool update();

	void exportCsv(QString filename);
};

#endif // ExportWPReadH
