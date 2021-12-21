#ifndef ImportWizardH
#define ImportWizardH

#include <QWizard>

#include <VICUS_Project.h>

namespace Ui {
class ImportWizard;
}

class ImportWizard : public QWizard
{
	Q_OBJECT

public:
	explicit ImportWizard(QWidget *parent = nullptr);
	~ImportWizard();

	void setProject(VICUS::Project*	project);

private:
	Ui::ImportWizard *ui;

	VICUS::Project*	m_project;
};

#endif // ImportWizardH
