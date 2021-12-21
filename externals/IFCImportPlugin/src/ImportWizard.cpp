#include "ImportWizard.h"
#include "ui_ImportWizard.h"

ImportWizard::ImportWizard(QWidget *parent) :
	QWizard(parent),
	ui(new Ui::ImportWizard),
	m_project(nullptr)
{
	ui->setupUi(this);

	setWizardStyle(QWizard::ModernStyle);

	setWindowTitle(tr("New Project Assistant"));
	setMinimumWidth(800);
}

ImportWizard::~ImportWizard()
{
	delete ui;
}

void ImportWizard::setProject(VICUS::Project* project) {
	m_project = project;
}

