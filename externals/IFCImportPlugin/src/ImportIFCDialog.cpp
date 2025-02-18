#include "ImportIFCDialog.h"
#include "ui_ImportIFCDialog.h"

#include <QFileDialog>

#include <IFCC_IFCReader.h>
#include <IFCC_Helper.h>

ImportIFCDialog::ImportIFCDialog(QWidget *parent, IFCC::IFCReader* reader) :
	QDialog(parent),
	ui(new Ui::ImportIFCDialog),
	m_reader(reader)
{
	ui->setupUi(this);
	ui->widgetConvert->setVisible(false);
	ui->pushButtonConvert->setEnabled(true);
	ui->checkBoxUseSpaceBoundaries->setChecked(false);
	ui->checkBoxWriteBuildingElements->setChecked(m_reader->convertOptions().m_writeBuildingElements);
	ui->checkBoxWriteConstructions->setChecked(m_reader->convertOptions().m_writeConstructionElements);
	ui->checkBoxWriteOpenings->setChecked(m_reader->convertOptions().m_writeOpeningElements);
	ui->checkBoxWriteAllOthers->setChecked(m_reader->convertOptions().m_writeOtherElements);

	for(auto type : IFCC::constructionTypes()) {
		QListWidgetItem *item = new QListWidgetItem(elementTypeText(type));
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(Qt::Checked);
		item->setData(Qt::UserRole, (int)type);
		ui->listWidgetConstructionTypes->addItem(item);
	}

	for(auto type : IFCC::constructionSimilarTypes()) {
		QListWidgetItem *item = new QListWidgetItem(elementTypeText(type));
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(Qt::Checked);
		item->setData(Qt::UserRole, (int)type);
		ui->listWidgetConstructionTypes->addItem(item);
	}

	for(auto type : IFCC::constructionTypes()) {
		QListWidgetItem *item = new QListWidgetItem(elementTypeText(type));
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(Qt::Checked);
		item->setData(Qt::UserRole, (int)type);
		ui->listWidgetOpeningSearchElements->addItem(item);
	}

	ui->comboBoxMatchingType->addItem(tr("Full matching"), MMT_FullMatching);
	ui->comboBoxMatchingType->addItem(tr("Medium matching"), MMT_MediumMatching);
	ui->comboBoxMatchingType->addItem(tr("No matching"), MMT_NoMatching);
	ui->comboBoxMatchingType->setCurrentIndex(1);
	setMatching(MMT_MediumMatching);

	ui->doubleSpinBoxMinimumArea->setValue(m_reader->convertOptions().m_minimumSurfaceArea*10000);
	ui->doubleSpinBoxMinimumDistance->setValue(m_reader->convertOptions().m_distanceEps*1000);
	ui->doubleSpinBoxMatchOpeningDistance->setValue(m_reader->convertOptions().m_openingDistance);
	ui->doubleSpinBoxStandardWallThickness->setValue(m_reader->convertOptions().m_standardWallThickness);

	ui->checkBoxAdvanced->setChecked(false);
	ui->tabWidgetAdvanced->setVisible(false);

	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

ImportIFCDialog::~ImportIFCDialog()
{
	delete ui;
}

void ImportIFCDialog::setFilename(QString filename) {
	ui->lineEditIFCFile->setText(filename);
}

void ImportIFCDialog::on_toolButtonOpenIFCFile_clicked() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open ifc file"), QString(), tr("ifc STEP file (*.ifc)"));
	if(!filename.isEmpty()) {
		ui->lineEditIFCFile->setText(filename);
	}
	else {
		ui->lineEditIFCFile->clear();
		ui->textEdit->setHtml(tr("No filename."));

	}
//	ui->textEdit->clear();
}

void ImportIFCDialog::on_lineEditIFCFile_textChanged(const QString &arg1) {
	QFileInfo finfo(arg1);
	if(finfo.isFile()) {
		ui->textEdit->setHtml(tr("Filename is valid. Start reading."));
		qApp->processEvents();
		bool res = read();
		ui->widgetConvert->setVisible(res);
		if(res) {
			initConvertOptions();
			ui->textEdit->setText(tr("Reading successful"));
		}
	}
	else {
		ui->textEdit->setHtml(tr("This is not a valid filename."));
	}
}

void ImportIFCDialog::on_pushButtonConvert_clicked() {
	ui->textEdit->setText(tr("Converting ..."));

	initElements();
	m_convertSuccessfully = m_reader->convert(ui->checkBoxUseSpaceBoundaries->isChecked());
	setText();

	if(ui->checkBoxIgnorErrors->isChecked())
		m_convertSuccessfully = true;
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_convertSuccessfully);
}


void ImportIFCDialog::on_checkBoxRemoveDoubleSBs_clicked() {
	m_reader->setRemoveDoubledSBs(ui->checkBoxRemoveDoubleSBs->isChecked());
}


void ImportIFCDialog::on_radioButtonMatchingFull_clicked() {
	ui->listWidgetConstructionTypes->setEnabled(true);
	ui->spinBoxMatchingNConstructions->setEnabled(false);
}


void ImportIFCDialog::on_radioButtonMatchingFirst_clicked() {
	ui->listWidgetConstructionTypes->setEnabled(true);
	ui->spinBoxMatchingNConstructions->setEnabled(false);
}


void ImportIFCDialog::on_radioButtonMatchingNConstructions_clicked() {
	ui->listWidgetConstructionTypes->setEnabled(true);
	ui->spinBoxMatchingNConstructions->setEnabled(true);
}


void ImportIFCDialog::on_radioButtonMatchingNo_clicked() {
	ui->listWidgetConstructionTypes->setEnabled(false);
	ui->spinBoxMatchingNConstructions->setEnabled(false);
}


void ImportIFCDialog::on_checkBoxUseSpaceBoundaries_clicked() {
	ui->tabWidgetAdvanced->setEnabled(!ui->checkBoxUseSpaceBoundaries->isChecked());
	ui->comboBoxMatchingType->setEnabled(!ui->checkBoxUseSpaceBoundaries->isChecked());
	ui->checkBoxAdvanced->setEnabled(!ui->checkBoxUseSpaceBoundaries->isChecked());
	if(ui->checkBoxUseSpaceBoundaries->isChecked()) {
		ui->labelMatchingDescription->setText(tr("No matching"));
	}
	else {
		MatchingMainType type = static_cast<MatchingMainType>(ui->comboBoxMatchingType->currentData().toInt());
		setMatching(type);
	}
}

void ImportIFCDialog::on_comboBoxMatchingType_currentIndexChanged(int index) {
	MatchingMainType type = static_cast<MatchingMainType>(ui->comboBoxMatchingType->currentData().toInt());
	setMatching(type);
}

bool ImportIFCDialog::read() {
	if(ui->lineEditIFCFile->text().isEmpty()) {
		ui->textEdit->setText(tr("No IFC file."));
		return false;
	}


	ui->textEdit->setText(tr("Reading ..."));
	ui->textEdit->update();
	IBK::Path ifcfilename(ui->lineEditIFCFile->text().toStdString());
	bool ignoreError = true;
	bool res = m_reader->read(ifcfilename, ignoreError);
	int buildings = 0;
	int spaces = 0;
	QString fatalError;
	bool hasFatalErrors = !m_reader->checkEssentialIFCs(fatalError, buildings, spaces);
	if(res && !hasFatalErrors) {
		int number = m_reader->totalNumberOfIFCEntities();
		ui->textEdit->clear();
		QStringList text;
		text << tr("File read successfully with %1 IFC entities.").arg(number);
		text << tr("Project contains %1 buildings and %2 spaces.").arg(buildings).arg(spaces);
		ui->textEdit->setHtml(text.join("<br>"));
		qApp->processEvents();
	}
	else {
		QStringList text;
		if(hasFatalErrors) {
			text << tr("<font color=\"#FF0000\">Fatal error</font>");
			text << tr("Missing objects");
			text << fatalError;
		}
		if(!res){
			if(!m_reader->m_errorText.empty()) {
				QString errorText = QString::fromStdString(m_reader->m_errorText);
				errorText.replace("\n", "<br>");
				if(!ignoreError)
					text << tr("<font color=\"#FF0000\">Read Errors:</font>");
				else
					text << tr("Read Errors:");
				text << errorText;
			}
			if(!m_reader->m_warningText.empty()) {
				text << tr("Read Warnings:");
				QString warningText = QString::fromStdString(m_reader->m_warningText);
				warningText.replace("\n", "<br>");
				text << warningText;
			}
		}
		ui->textEdit->setHtml(text.join("<br>"));
		qApp->processEvents();
		if(!ignoreError || hasFatalErrors)
			return false;
	}
	return true;
}

void ImportIFCDialog::setText() {

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
//	intersectedSpaceIds = m_reader->checkForIntersectedSpace();
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

	ui->textEdit->clear();
	QStringList text;

	// print out convert errors
	const std::vector<IFCC::ConvertError>& errors = m_reader->convertErrors();
	if(!errors.empty()) {
		text << tr("<font color=\"#FF0000\">Conversion errors:</font>");
		for( const auto& err : errors) {
			text << QString("%1 for object '%2' with id: %3").arg(QString::fromStdString(err.m_errorText))
					.arg(QString::fromStdString(IFCC::objectTypeToString(err.m_objectType))).arg(err.m_objectID);
		}
		text << "<br>";
	}

	if(m_convertSuccessfully) {
		text << tr("Converting was successful");
		if(!m_reader->m_errorText.empty()) {
			text << tr("<font color=\"#FF0000\">Errors while converting:</font>");
			QString errTxt = QString::fromStdString(m_reader->m_errorText);
			errTxt.replace("\n","<br>");
			text << errTxt;
			text << "";
		}
		text << tr("File converted successfully.");
//		text << m_reader->messages() << "";
		text << "";
//		text << m_reader->statistic();
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
	ui->textEdit->setHtml(text.join("<br>"));
}

void ImportIFCDialog::initElements() {
	m_reader->clearElementsForSpaceBoundaries();
	for(int i=0; i<ui->listWidgetConstructionTypes->count(); ++i) {
		QListWidgetItem* item = ui->listWidgetConstructionTypes->item(i);
		if(item->checkState() == Qt::Checked)
			m_reader->setElementsForSpaceBoundaries(static_cast<IFCC::BuildingElementTypes>(item->data(Qt::UserRole).toInt()), true);
	}

	QSet<IFCC::BuildingElementTypes> noSearchForOpeningsInTypes;
	for(int i=0; i<ui->listWidgetOpeningSearchElements->count(); ++i) {
		QListWidgetItem* item = ui->listWidgetOpeningSearchElements->item(i);
		if(item->checkState() != Qt::Checked)
			noSearchForOpeningsInTypes << static_cast<IFCC::BuildingElementTypes>(item->data(Qt::UserRole).toInt());
	}
	m_reader->addNoSearchForOpenings(noSearchForOpeningsInTypes);

	if(ui->radioButtonMatchingFull->isChecked())
		m_reader->setConvertMatchingType(IFCC::ConvertOptions::CM_MatchEachConstruction);
	else if(ui->radioButtonMatchingFirst->isChecked())
		m_reader->setConvertMatchingType(IFCC::ConvertOptions::CM_MatchOnlyFirstConstruction);
	else if(ui->radioButtonMatchingNConstructions->isChecked())
		m_reader->setConvertMatchingType(IFCC::ConvertOptions::CM_MatchFirstNConstructions);
	else
		m_reader->setConvertMatchingType(IFCC::ConvertOptions::CM_NoMatching);

	m_reader->setMatchingDistances(ui->doubleSpinBoxMatchConstructionFactor->value(),
								   ui->doubleSpinBoxStandardWallThickness->value(),
								   ui->doubleSpinBoxMatchOpeningDistance->value());
	m_reader->setWritingBuildingElements(ui->checkBoxWriteConstructions->isChecked(),
										 ui->checkBoxWriteBuildingElements->isChecked(),
										 ui->checkBoxWriteOpenings->isChecked(),
										 ui->checkBoxWriteAllOthers->isChecked());

	// with conversions into basic SI units m and m2
	m_reader->setMinimumCheckValues(ui->doubleSpinBoxMinimumDistance->value() / 1000.0, ui->doubleSpinBoxMinimumArea->value() / 10000.0);
	m_reader->setUseCSGForOpenings(ui->checkBoxOpeningCSGSearch->isChecked());
}

void ImportIFCDialog::setMatching(MatchingMainType type) {
	bool hasSBs = ui->checkBoxUseSpaceBoundaries->isChecked();
	switch(type) {
		case MMT_FullMatching: {
				ui->radioButtonMatchingFull->setChecked(true);
				break;
			}
		case MMT_MediumMatching: {
				ui->radioButtonMatchingFirst->setChecked(true);
				break;
			}
		case MMT_NoMatching: {
				ui->radioButtonMatchingNo->setChecked(true);
				break;
			}
	}
	if(!hasSBs) {
		switch(type) {
			case MMT_FullMatching: {
					ui->labelMatchingDescription->setText(tr("Check each construction for matches with space surfaces."));
					break;
				}
			case MMT_MediumMatching: {
					ui->labelMatchingDescription->setText(tr("Check only the most possible constructions for matching with space surfaces."));
					break;
				}
			case MMT_NoMatching: {
					ui->labelMatchingDescription->setText(tr("No matching test. All created space boundaries doesn't have constructions. No windows or doors possible."));
					break;
				}
		}

	}
}

QString ImportIFCDialog::elementTypeText(IFCC::BuildingElementTypes type) const {
	switch(type) {
		case IFCC::BET_Beam: return tr("Beam");
		case IFCC::BET_Chimney: return tr("Chimney");
		case IFCC::BET_Column: return tr("Column");
		case IFCC::BET_Covering: return tr("Covering");
		case IFCC::BET_CurtainWall: return tr("Curtain Wall");
		case IFCC::BET_Door: return tr("Door");
		case IFCC::BET_Footing: return tr("Footing");
		case IFCC::BET_Member: return tr("Member");
		case IFCC::BET_Pile: return tr("Pile");
		case IFCC::BET_Plate: return tr("Plate");
		case IFCC::BET_Railing: return tr("Railing");
		case IFCC::BET_Ramp: return tr("Ramp");
		case IFCC::BET_RampFlight: return tr("RampFlight");
		case IFCC::BET_Roof: return tr("Roof");
		case IFCC::BET_ShadingDevice: return tr("Shading Device");
		case IFCC::BET_Slab: return tr("Slab");
		case IFCC::BET_Stair: return tr("Stair");
		case IFCC::BET_StairFlight: return tr("Stair Flight");
		case IFCC::BET_Wall: return tr("Wall");
		case IFCC::BET_Window: return tr("Window");
		case IFCC::BET_CivilElement: return tr("Civil Element");
		case IFCC::BET_DistributionElement: return tr("Distribution Element");
		case IFCC::BET_ElementAssembly: return tr("Element Assembly");
		case IFCC::BET_ElementComponent: return tr("Element Component");
		case IFCC::BET_FeatureElement: return tr("Feature Element");
		case IFCC::BET_FurnishingElement: return tr("Furnishing Element");
		case IFCC::BET_GeographicalElement: return tr("Geographical Element");
		case IFCC::BET_TransportElement: return tr("Transport Element");
		case IFCC::BET_VirtualElement: return tr("VirtualElement");
		case IFCC::BET_BuildingElementPart: return tr("Building Element Part");
		default: return tr("");
	}
}

void ImportIFCDialog::initConvertOptions() {
	ui->textEdit->clear();
	int sbCount = m_reader->numberOfIFCSpaceBoundaries();
	bool useSpaceBoundaries = sbCount > 0;
	ui->checkBoxUseSpaceBoundaries->setEnabled(useSpaceBoundaries);
	ui->checkBoxUseSpaceBoundaries->setChecked(useSpaceBoundaries);
	ui->comboBoxMatchingType->setEnabled(!useSpaceBoundaries);
	ui->checkBoxAdvanced->setEnabled(!useSpaceBoundaries);
	if(!useSpaceBoundaries) {
		ui->labelSBDescription->setText(tr("There are no space boundaries in the IFC file"));
	}
	else {
		ui->labelSBDescription->setText(tr("There are %1 space boundaries in the IFC file").arg(sbCount));
	}

	ui->doubleSpinBoxMatchConstructionFactor->setValue(m_reader->convertOptions().m_distanceFactor);
	ui->doubleSpinBoxMatchOpeningDistance->setValue(m_reader->convertOptions().m_openingDistance);
	ui->checkBoxWriteBuildingElements->setChecked(m_reader->convertOptions().m_writeBuildingElements);
	ui->checkBoxWriteConstructions->setChecked(m_reader->convertOptions().m_writeConstructionElements);
	ui->checkBoxWriteOpenings->setChecked(m_reader->convertOptions().m_writeOpeningElements);
	ui->checkBoxWriteAllOthers->setChecked(m_reader->convertOptions().m_writeOtherElements);
}


void ImportIFCDialog::on_checkBoxAdvanced_clicked() {
	ui->tabWidgetAdvanced->setVisible(ui->checkBoxAdvanced->isChecked());
}


