#include "ExportWizard.h"
#include "ui_ExportWizard.h"

#include "ExportWPRead.h"

ExportWizard::ExportWizard(QWidget *parent, VICUS::Project* project) :
	QWizard(parent),
	ui(new Ui::ExportWizard),
	m_project(project),
	m_pageRead(new ExportWPRead(this, project))
{
	ui->setupUi(this);

	removePage(0);

	setWizardStyle(QWizard::ModernStyle);

	setWindowTitle(tr("Export GEG Assistant"));
	setMinimumWidth(800);

	addPage(m_pageRead);
}

ExportWizard::~ExportWizard() {
	delete ui;
}

