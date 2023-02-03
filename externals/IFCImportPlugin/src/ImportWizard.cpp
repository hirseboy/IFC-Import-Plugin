#include "ImportWizard.h"
#include "ui_ImportWizard.h"

#include "ImportWPRead.h"
#include "ImportWPConvert.h"

ImportWizard::ImportWizard(QWidget *parent, IFCC::IFCReader* reader) :
	QWizard(parent),
	ui(new Ui::ImportWizard),
	m_pageRead(new ImportWPRead(this, reader)),
	m_pageConvert(new ImportWPConvert(this, reader)),
	m_reader(reader)
{
	ui->setupUi(this);

	removePage(0);

	setWizardStyle(QWizard::ModernStyle);

	setWindowTitle(tr("Import IFC Assistant"));
	setMinimumWidth(800);

	addPage(m_pageRead);
	addPage(m_pageConvert);
}

ImportWizard::~ImportWizard() {
	delete ui;
}

