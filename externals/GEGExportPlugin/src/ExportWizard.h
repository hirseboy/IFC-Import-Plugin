#ifndef ExportWizardH
#define ExportWizardH

#include <QWizard>

namespace VICUS {
	class Project;
}

namespace Ui {
	class ExportWizard;
}

class ExportWPRead;

class ExportWizard : public QWizard
{
	Q_OBJECT

public:
	explicit ExportWizard(QWidget *parent, VICUS::Project* project);
	~ExportWizard();

private:

	Ui::ExportWizard *	ui;
	VICUS::Project* m_project;

	ExportWPRead*	m_pageRead;
};

#endif // ExportWizardH
