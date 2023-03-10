#include "ImportWPRead.h"
#include "ui_ImportWPRead.h"

#include <QFileDialog>

#include <IFCC_IFCReader.h>

#include <IBK_Path.h>

ImportWPRead::ImportWPRead(QWidget *parent, IFCC::IFCReader* reader) :
	QWizardPage(parent),
	ui(new Ui::ImportWPRead),
	m_reader(reader),
	m_readSuccessfully(false)
{
	ui->setupUi(this);
	setTitle(tr("Read IFC file"));
	setSubTitle(tr("You can load an IFC file."));

	ui->pushButtonRead->setEnabled(false);
	ui->checkBoxIgnoreReadError->setChecked(true);

}

ImportWPRead::~ImportWPRead() {
	delete ui;
}

bool ImportWPRead::isComplete() const {
	return m_readSuccessfully;
}

void ImportWPRead::on_toolButtonOpenIFCFile_clicked() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open ifc file"), QString(), tr("ifc STEP file (*.ifc)"));
	if(!filename.isEmpty()) {
		ui->lineEditIFCFile->setText(filename);
		ui->pushButtonRead->setEnabled(true);
	}
	else {
		ui->lineEditIFCFile->clear();
		ui->pushButtonRead->setEnabled(false);
	}
	ui->textEdit->clear();
}


void ImportWPRead::on_pushButtonRead_clicked() {
	m_readSuccessfully = true;
	if(ui->lineEditIFCFile->text().isEmpty())
		return;


	ui->textEdit->setText(tr("Reading ..."));
	ui->textEdit->update();
	IBK::Path ifcfilename(ui->lineEditIFCFile->text().toStdString());
	bool ignoreError = ui->checkBoxIgnoreReadError->isChecked();
	bool res = m_reader->read(ifcfilename, ignoreError);
	int buildings = 0;
	int spaces = 0;
	QString fatalError;
	bool hasFatalErrors = !m_reader->checkEssentialIFCs(fatalError, buildings, spaces);
	if(res && !hasFatalErrors) {
		int number = m_reader->totalNumberOfIFCEntities();
		ui->textEdit->clear();
		QStringList text;
		text << tr("File read successfully with %1 IFC entities.<br>").arg(number);
		text << tr("Project contains %1 buildings and %2 spaces.<br>").arg(buildings).arg(spaces);
		ui->textEdit->setHtml(text.join("\n"));
	}
	else {
		QStringList text;
		if(hasFatalErrors) {
			text << tr("<font color=\"#FF0000\">Fatal error</font><br>");
			text << tr("Missing objects<br>");
			text << fatalError << "<br>";
		}
		if(!res){
			if(!m_reader->m_errorText.empty()) {
				if(!ignoreError)
					text << tr("<font color=\"#FF0000\">Read Errors:</font><br>");
				else
					text << tr("Read Errors:<br>");
				text << QString::fromStdString(m_reader->m_errorText) << "<br>";
			}
			if(!m_reader->m_warningText.empty()) {
				text << tr("Read Warnings:<br>");
				text << QString::fromStdString(m_reader->m_warningText) << "<br>";
			}
		}
		ui->textEdit->setHtml(text.join("\n"));
		if(!ignoreError || hasFatalErrors)
			m_readSuccessfully = false;
	}
	emit completeChanged();
}

