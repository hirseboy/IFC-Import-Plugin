#ifndef ImportWizardH
#define ImportWizardH

#include <QWizard>

#include <VICUS_Project.h>

class ImportWPRead;
class ImportWPConvert;

namespace IFCC {
	class IFCReader;
}

namespace Ui {
class ImportWizard;
}

class ImportWizard : public QWizard
{
	Q_OBJECT

public:
	explicit ImportWizard(QWidget *parent, IFCC::IFCReader* reader);
	~ImportWizard();

	void setProject(VICUS::Project*	project);

private:

	Ui::ImportWizard *	ui;

	VICUS::Project*		m_project;

	ImportWPRead*		m_pageRead;
	ImportWPConvert*	m_pageConvert;

	IFCC::IFCReader*	m_reader;
};

#endif // ImportWizardH
