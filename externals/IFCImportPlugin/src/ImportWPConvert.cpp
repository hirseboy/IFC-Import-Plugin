#include "ImportWPConvert.h"
#include "ui_ImportWPConvert.h"

#include <QFileDialog>
#include <QMessageBox>

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
	bool ignoreErrors = ui->checkBoxIgnorErrors->isChecked();
	if(ignoreErrors)
		return true;

	return m_convertSuccessfully;
}

void ImportWPConvert::setText() {
	std::vector<std::pair<int,int>> equalSBs;
	int equalSBCount = m_reader->checkForEqualSpaceBoundaries(equalSBs);
	std::vector<std::pair<int,std::vector<int>>> multiSubsurfaces;
	int multiSubCount = m_reader->checkForUniqueSubSurfacesInSpaces(multiSubsurfaces);
	std::set<std::pair<int,int>> intersectedSpaceIds = m_reader->checkForIntersectedSpace();
	std::map<int,size_t> instersectionCounts;
	for(auto isId : intersectedSpaceIds) {
		++instersectionCounts[isId.first];
	}
	int spaceIntersectCount = intersectedSpaceIds.size();
	if(m_convertSuccessfully && (equalSBCount > 0 || multiSubCount > 0 || spaceIntersectCount)) {
		m_convertSuccessfully = false;
	}
	ui->textEdit->clear();
	QStringList text;
	if(m_convertSuccessfully) {
		if(!m_reader->m_errorText.empty()) {
			text << tr("<font color=\"#FF0000\">Errors while converting:</font>");
			QString errTxt = QString::fromStdString(m_reader->m_errorText);
			text << errTxt.split("");
			text << "";
		}
		const std::vector<IFCC::ConvertError>& errors = m_reader->convertErrors();
		if(!errors.empty()) {
			text << tr("<font color=\"#FF0000\">Conversion errors:</font>");
			for( const auto& err : errors) {
				text << QString("%1 for object '%2' with id: %3").arg(QString::fromStdString(err.m_errorText))
						.arg(QString::fromStdString(IFCC::objectTypeToString(err.m_objectType))).arg(err.m_objectID);
			}
			text << "<br>";
		}
		text << tr("File converted successfully.");
		text << m_reader->messages() << "";
		text << "";
		text << m_reader->statistic();
	}
	else {
		text << tr("<font color=\"#FF0000\">Error while converting IFC file.</font>");
		if(spaceIntersectCount > 0) {
			text << tr("%1 spaces intersections found.").arg(spaceIntersectCount);
			if(ui->checkBoxSpaceIntersectionDetails->isChecked()) {
				text << tr("Space intersections.<br>");
				for(const auto& it : intersectedSpaceIds) {
					QString name1 = m_reader->nameForId(it.first, IFCC::IFCReader::NIT_Space);
					QString name2 = m_reader->nameForId(it.second, IFCC::IFCReader::NIT_Space);
					text << tr("'%1' with '%2'.").arg(name1).arg(name2);
				}
			}
			else {
				for(const auto& it : instersectionCounts) {
					text << tr("Space %1 with %2 spaces intersected.").arg(it.first).arg(it.second);
				}
			}
		}
		else if(equalSBCount > 0) {
			text << tr("%1 space boundaries with identical surfaces found.").arg(equalSBCount);
		}
		else if(multiSubCount > 0) {
			text << tr("%1 opening space boundaries found which are used more than once in one space.").arg(multiSubCount);
		}
		else {
			text << QString::fromStdString(m_reader->m_errorText);
		}
	}
	ui->textEdit->setHtml(text.join("<br>"));
}

void ImportWPConvert::on_pushButtonConvert_clicked() {
	ui->textEdit->setText(tr("Converting ..."));
	try {
		m_convertSuccessfully = m_reader->convert(ui->checkBoxUseSpaceBoundaries->isChecked());
	}
	catch(IBK::Exception &ex) {
		QMessageBox::warning(this, tr("Conversion error"), tr("Could not convert IFC-File. See Error below:\n%1").arg(ex.what()));
		return;
	}

	setText();

	emit completeChanged();
}

void ImportWPConvert::on_checkBoxRemoveDoubleSBs_clicked() {
	m_reader->setRemoveDoubledSBs(ui->checkBoxRemoveDoubleSBs->isChecked());
}


void ImportWPConvert::on_checkBoxIgnorErrors_toggled(bool checked) {
	emit completeChanged();
}


void ImportWPConvert::on_checkBoxSpaceIntersectionDetails_clicked() {
	setText();
}

