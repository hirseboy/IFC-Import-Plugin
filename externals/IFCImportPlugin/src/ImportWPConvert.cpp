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

}

ImportWPConvert::~ImportWPConvert() {
	delete ui;
}

bool ImportWPConvert::isComplete() const {
	return m_convertSuccessfully;
}


void ImportWPConvert::on_pushButtonConvert_clicked() {
	m_convertSuccessfully = false;

	ui->textEdit->setText(tr("Converting ..."));
	bool res = m_reader->convert();
	ui->textEdit->clear();
	QStringList text;
	if(res) {
		text << tr("File converted successfully.");
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
