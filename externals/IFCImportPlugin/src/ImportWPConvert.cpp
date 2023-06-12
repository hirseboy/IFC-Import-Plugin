#include "ImportWPConvert.h"
#include "ui_ImportWPConvert.h"

#include <QFileDialog>
#include <QTextStream>

#include <IFCC_IFCReader.h>
#include <IFCC_Helper.h>
#include <IFCC_Logger.h>
#include <IFCC_Types.h>

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

	QListWidgetItem *item = new QListWidgetItem(tr("Wall"));
	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	item->setCheckState(Qt::Checked);
	item->setData(Qt::UserRole, (int)IFCC::BET_Wall);
	ui->listWidgetConstructionTypes->addItem(item);

	item = new QListWidgetItem(tr("Slab"));
	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	item->setCheckState(Qt::Checked);
	item->setData(Qt::UserRole, (int)IFCC::BET_Slab);
	ui->listWidgetConstructionTypes->addItem(item);

	item = new QListWidgetItem(tr("Beam"));
	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	item->setCheckState(Qt::Checked);
	item->setData(Qt::UserRole, (int)IFCC::BET_Beam);
	ui->listWidgetConstructionTypes->addItem(item);

	item = new QListWidgetItem(tr("Covering"));
	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	item->setCheckState(Qt::Checked);
	item->setData(Qt::UserRole, (int)IFCC::BET_Covering);
	ui->listWidgetConstructionTypes->addItem(item);

	item = new QListWidgetItem(tr("Footing"));
	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	item->setCheckState(Qt::Checked);
	item->setData(Qt::UserRole, (int)IFCC::BET_Footing);
	ui->listWidgetConstructionTypes->addItem(item);

	ui->groupBoxMatchingSettings->setEnabled(false);
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
		ui->groupBoxMatchingSettings->setEnabled(true);
	}
	else {
		ui->labelSBDescription->setText(tr("There are %1 space boundaries in the IFC file").arg(sbCount));
	}

	ui->doubleSpinBoxMatchConstructionFactor->setValue(m_reader->convertOptions().m_distanceFactor);
	ui->doubleSpinBoxMatchOpeningDistance->setValue(m_reader->convertOptions().m_openingDistance);
}


bool ImportWPConvert::isComplete() const {
	IFCC::Logger::instance() << "isComplete";

	bool ignoreErrors = ui->checkBoxIgnorErrors->isChecked();
	if(ignoreErrors)
		return true;

	return m_convertSuccessfully;
}

void ImportWPConvert::setText() {
	IFCC::Logger::instance() << "setText start";

	// check for equal space boundaries
	std::vector<std::pair<int,int>> equalSBs;
	int equalSBCount = 0;
	equalSBCount = m_reader->checkForEqualSpaceBoundaries(equalSBs);

	//check for unique sub surfaces
	std::vector<std::pair<int,std::vector<int>>> multiSubsurfaces;
	int multiSubCount = 0;
	multiSubCount = m_reader->checkForUniqueSubSurfacesInSpaces(multiSubsurfaces);

	// check for intersected spaces
	std::set<std::pair<int,int>> intersectedSpaceIds;
	intersectedSpaceIds = m_reader->checkForIntersectedSpace();
	std::map<int,size_t> instersectionCounts;
	for(auto isId : intersectedSpaceIds) {
		++instersectionCounts[isId.first];
	}
	int spaceIntersectCount = intersectedSpaceIds.size();

	// check for shared space boundaries
	std::set<std::pair<int,int>> sharedSpaceBoundaries;
	sharedSpaceBoundaries = m_reader->checkForSpaceWithSameSpaceBoundaries();
	std::map<int,size_t> sharedSpaceBoundaryCounts;
	for(auto isId : sharedSpaceBoundaries) {
		++sharedSpaceBoundaryCounts[isId.first];
	}
	int sharedSpaceBoundaryCount = intersectedSpaceIds.size();

	int notRelatedOpenings = m_reader->checkForNotRelatedOpenings();


	if(m_convertSuccessfully && (equalSBCount > 0 || multiSubCount > 0 || spaceIntersectCount > 0 || sharedSpaceBoundaryCount > 0)) {
		m_convertSuccessfully = false;
	}

	std::vector<int> wrongInstances = m_reader->checkForWrongSurfaceIds();
	if(!wrongInstances.empty())
		m_convertSuccessfully = false;

	IFCC::Logger::instance() << "setText 1";

	ui->textEdit->clear();
	QStringList text;

	// print out convert errors
	const std::vector<IFCC::ConvertError>& errors = m_reader->convertErrors();
	if(!errors.empty()) {
		IFCC::Logger::instance() << "setText 31";
		text << tr("<font color=\"#FF0000\">Conversion errors:</font>");
		for( const auto& err : errors) {
			text << QString("%1 for object '%2' with id: %3").arg(QString::fromStdString(err.m_errorText))
					.arg(QString::fromStdString(IFCC::objectTypeToString(err.m_objectType))).arg(err.m_objectID);
		}
		text << "<br>";
	}

	if(m_convertSuccessfully) {
		IFCC::Logger::instance() << "setText 2";
		if(!m_reader->m_errorText.empty()) {
			IFCC::Logger::instance() << "setText 21";
			text << tr("<font color=\"#FF0000\">Errors while converting:</font>");
			QString errTxt = QString::fromStdString(m_reader->m_errorText);
			text << errTxt.split("");
			text << "";
		}
		IFCC::Logger::instance() << "setText 3";
		IFCC::Logger::instance() << "setText 4";
		text << tr("File converted successfully.");
//		text << m_reader->messages() << "";
		text << "";
//		text << m_reader->statistic();
	}
	else {
		IFCC::Logger::instance() << "setText 5";
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
		if(sharedSpaceBoundaryCount > 0) {
			text << tr("%1 spaces with shared space boundaries found.").arg(sharedSpaceBoundaryCount);
			for(const auto& it : sharedSpaceBoundaryCounts) {
				text << tr("Space %1 shares with %2 spaces at least one space boundary.").arg(it.first).arg(it.second);
			}
		}
		if(equalSBCount > 0) {
			text << tr("%1 space boundaries with identical surfaces found.").arg(equalSBCount);
		}
		if(multiSubCount > 0) {
			text << tr("%1 opening space boundaries found which are used more than once in one space.").arg(multiSubCount);
		}
		if(wrongInstances.size() > 0) {
			text << tr("%1 component instances found which have non valid surface ids.").arg(wrongInstances.size());
		}
		if(notRelatedOpenings > 0) {
			text << tr("%1 openings don't have a connection to a space boundary.").arg(notRelatedOpenings);
		}
		if(!m_reader->m_errorText.empty()) {
			text << QString::fromStdString(m_reader->m_errorText);
		}
	}
	IFCC::Logger::instance() << "setText 6 " << text.size() ;
	ui->textEdit->setHtml(text.join("<br>"));
	IFCC::Logger::instance() << "setText 7";
}

void ImportWPConvert::initElements() {
	m_reader->clearElementsForSpaceBoundaries();
	for(int i=0; i<ui->listWidgetConstructionTypes->count(); ++i) {
		QListWidgetItem* item = ui->listWidgetConstructionTypes->item(i);
		if(item->checkState() == Qt::Checked)
			m_reader->setElementsForSpaceBoundaries(static_cast<IFCC::BuildingElementTypes>(item->data(Qt::UserRole).toInt()), true);
	}

	if(ui->radioButtonMatchingFull->isChecked())
		m_reader->setConvertMatchingType(IFCC::ConvertOptions::CM_MatchEachConstruction);
	else if(ui->radioButtonMatchingFirst->isChecked())
		m_reader->setConvertMatchingType(IFCC::ConvertOptions::CM_MatchOnlyFirstConstruction);
	else if(ui->radioButtonMatchingNConstructions->isChecked())
		m_reader->setConvertMatchingType(IFCC::ConvertOptions::CM_MatchFirstNConstructions);
	else
		m_reader->setConvertMatchingType(IFCC::ConvertOptions::CM_NoMatching);

	m_reader->setMatchingDistances(ui->doubleSpinBoxMatchConstructionFactor->value(), ui->doubleSpinBoxMatchOpeningDistance->value());
}

void ImportWPConvert::on_pushButtonConvert_clicked() {
	ui->textEdit->setText(tr("Converting ..."));

	initElements();
	m_convertSuccessfully = m_reader->convert(ui->checkBoxUseSpaceBoundaries->isChecked());

	IFCC::Logger::instance() << "convert successful " << m_convertSuccessfully;
	setText();

	IFCC::Logger::instance() << "setText";

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


void ImportWPConvert::on_pushButtonSaveLog_clicked() {
	QString filename = QFileDialog::getSaveFileName(
							this,
							tr("Specify name for log file"),
							"IFC_convert",
							tr("Log file (*.log);;All files (*.*)"));

	if(!filename.isEmpty()) {
		QFile file(filename);
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		QTextStream stream(&file);
		stream << ui->textEdit->toPlainText();
	}
}


void ImportWPConvert::on_radioButtonMatchingFull_clicked() {
	ui->listWidgetConstructionTypes->setEnabled(true);
	ui->spinBoxMatchingNConstructions->setEnabled(false);
}


void ImportWPConvert::on_radioButtonMatchingFirst_clicked() {
	ui->listWidgetConstructionTypes->setEnabled(true);
	ui->spinBoxMatchingNConstructions->setEnabled(false);
}


void ImportWPConvert::on_radioButtonMatchingNConstructions_clicked() {
	ui->listWidgetConstructionTypes->setEnabled(true);
	ui->spinBoxMatchingNConstructions->setEnabled(true);
}


void ImportWPConvert::on_radioButtonMatchingNo_clicked() {
	ui->listWidgetConstructionTypes->setEnabled(false);
	ui->spinBoxMatchingNConstructions->setEnabled(false);
}


void ImportWPConvert::on_checkBoxUseSpaceBoundaries_clicked() {
	ui->groupBoxMatchingSettings->setEnabled(!ui->checkBoxUseSpaceBoundaries->isChecked());
}

