#ifndef ExportWizardH
#define ExportWizardH

#include <QWizard>

//#include <VICUS_Project.h>

namespace Ui {
class ExportWizard;
}

class ExportWizard : public QWizard
{
	Q_OBJECT

public:
	explicit ExportWizard(QWidget *parent);
	~ExportWizard();

private:

	Ui::ExportWizard *	ui;
};

#endif // ExportWizardH
