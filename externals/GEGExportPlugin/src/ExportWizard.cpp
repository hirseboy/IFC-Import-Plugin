#include "ExportWizard.h"
#include "ui_ExportWizard.h"

ExportWizard::ExportWizard(QWidget *parent) :
	QWizard(parent),
	ui(new Ui::ExportWizard)
{
	ui->setupUi(this);

	removePage(0);

	setWizardStyle(QWizard::ModernStyle);

	setWindowTitle(tr("Export GEG Assistant"));
	setMinimumWidth(800);
}

ExportWizard::~ExportWizard() {
	delete ui;
}

