#include "ImportWPConvert.h"
#include "ui_ImportWPConvert.h"

#include <QFileDialog>

#include <IFCC_IFCReader.h>
#include <IFCC_Helper.h>

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
	m_reader->setFlipPolygons(ui->checkBoxFlipSurfacePolygons->isChecked());
	return m_convertSuccessfully;
}


void ImportWPConvert::on_pushButtonConvert_clicked() {
	m_convertSuccessfully = false;

	ui->textEdit->setText(tr("Converting ..."));
	bool res = m_reader->convert(ui->checkBoxUseSpaceBoundaries->isChecked());
	std::vector<std::pair<int,int>> equalSBs;
	int equalSBCount = m_reader->checkForEqualSpaceBoundaries(equalSBs);
	std::vector<std::pair<int,std::vector<int>>> multiSubsurfaces;
	int multiSubCount = m_reader->checkForUniqueSubSurfacesInSpaces(multiSubsurfaces);
	int spaceIntersectCount = m_reader->checkForIntersectedSpace();
	if(res && (equalSBCount > 0 || multiSubCount > 0 || spaceIntersectCount)) {
		res = false;
	}
	ui->textEdit->clear();
	QStringList text;
	if(res) {
		if(!m_reader->m_errorText.empty()) {
			text << tr("<font color=\"#FF0000\">Errors while converting:</font><br>");
			QString errTxt = QString::fromStdString(m_reader->m_errorText);
			text << errTxt.split("<br>");
			text << "<br>";
		}
		const std::vector<IFCC::ConvertError>& errors = m_reader->convertErrors();
		if(!errors.empty()) {
			text << tr("<font color=\"#FF0000\">Conversion errors:</font><br>");
			for( const auto& err : errors) {
				text << QString("%1 for object '%2' with id: %3<br>").arg(QString::fromStdString(err.m_errorText))
						.arg(QString::fromStdString(IFCC::objectTypeToString(err.m_objectType))).arg(err.m_objectID);
			}
			text << "<br>";
		}
		text << tr("File converted successfully.<br>");
		text << m_reader->messages() << "<br>";
		text << "<br>";
		text << m_reader->statistic();
		m_convertSuccessfully = true;
	}
	else {
		text << tr("<font color=\"#FF0000\">Error while converting IFC file.</font><br>");
		if(spaceIntersectCount > 0) {
			text << tr("%1 spaces are intersected to other spaces.<br>").arg(spaceIntersectCount);
		}
		else if(equalSBCount > 0) {
			text << tr("%1 space boundaries with identical surfaces found.<br>").arg(equalSBCount);
		}
		else if(multiSubCount > 0) {
			text << tr("%1 opening space boundaries found which are used more than once in one space.<br>").arg(multiSubCount);
		}
		else {
			text << QString::fromStdString(m_reader->m_errorText) << "<br>";
		}
	}
	ui->textEdit->setHtml(text.join("\n"));

	emit completeChanged();
}

void ImportWPConvert::on_checkBoxRemoveDoubleSBs_clicked() {
	m_reader->setRemoveDoubledSBs(ui->checkBoxRemoveDoubleSBs->isChecked());
}

