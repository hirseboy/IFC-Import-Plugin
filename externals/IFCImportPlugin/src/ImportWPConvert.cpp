#include "ImportWPConvert.h"
#include "ui_ImportWPConvert.h"

#include <QFileDialog>

#include <IFCC_IFCReader.h>

#include <IBK_Path.h>

ImportWPConvert::ImportWPConvert(QWidget *parent, IFCC::IFCReader* reader) :
	QWizardPage(parent),
	ui(new Ui::ImportWPConvert),
	m_reader(reader),
	m_convertSuccessfully(false)
{
	ui->setupUi(this);
	setTitle(tr("Convert IFC entities and geometry into VICUS format"));
//	setSubTitle(tr("You can load an IFC file."));

	ui->pushButtonConvert->setEnabled(true);
	ui->checkBoxUseSpaceBoundaries->setChecked(true);
}

ImportWPConvert::~ImportWPConvert() {
	delete ui;
}


void ImportWPConvert::initializePage() {
	ui->textEdit->clear();
	int sbCount = m_reader->numberOfIFCSpaceBoundaries();
	ui->checkBoxUseSpaceBoundaries->setEnabled(sbCount > 0);
	if(sbCount == 0) {
		ui->labelSBDescription->setText(tr("There are no space boundaries in the IFC file"));
		ui->checkBoxUseSpaceBoundaries->setChecked(false);
	}
	else {
		ui->labelSBDescription->setText(tr("There are %1 space boundaries in the IFC file").arg(sbCount));
	}
}


bool ImportWPConvert::isComplete() const {
	return m_convertSuccessfully;
}


void ImportWPConvert::on_pushButtonConvert_clicked() {
	m_convertSuccessfully = false;

	ui->textEdit->setText(tr("Converting ..."));
	bool res = m_reader->convert(ui->checkBoxUseSpaceBoundaries->isChecked());
	ui->textEdit->clear();
	QStringList text;
	if(res) {
		text << tr("File converted successfully.");
		text << m_reader->messages();
		text << " ";
		text << m_reader->statistic();
		m_convertSuccessfully = true;
	}
	else {
		text << tr("Error while converting IFC file.");
		text << QString::fromStdString(m_reader->m_errorText);
	}
	ui->textEdit->setText(text.join("\n"));

	emit completeChanged();
}

