#include "IFCC_IFCReader.h"

#include "IFCC_Helper.h"

#include <QDebug>
#include <QApplication>


#include <ifcpp/IFC4X3/include/IfcRelSpaceBoundary.h>
#include <ifcpp/IFC4X3/include/IfcWall.h>
#include <ifcpp/IFC4X3/include/IfcBeam.h>
#include <ifcpp/IFC4X3/include/IfcChimney.h>
#include <ifcpp/IFC4X3/include/IfcColumn.h>
#include <ifcpp/IFC4X3/include/IfcCovering.h>
#include <ifcpp/IFC4X3/include/IfcCurtainWall.h>
#include <ifcpp/IFC4X3/include/IfcDoor.h>
#include <ifcpp/IFC4X3/include/IfcFooting.h>
#include <ifcpp/IFC4X3/include/IfcMember.h>
#include <ifcpp/IFC4X3/include/IfcPile.h>
#include <ifcpp/IFC4X3/include/IfcPlate.h>
#include <ifcpp/IFC4X3/include/IfcRailing.h>
#include <ifcpp/IFC4X3/include/IfcRamp.h>
#include <ifcpp/IFC4X3/include/IfcRampFlight.h>
#include <ifcpp/IFC4X3/include/IfcRoof.h>
#include <ifcpp/IFC4X3/include/IfcShadingDevice.h>
#include <ifcpp/IFC4X3/include/IfcSlab.h>
#include <ifcpp/IFC4X3/include/IfcStair.h>
#include <ifcpp/IFC4X3/include/IfcStairFlight.h>
#include <ifcpp/IFC4X3/include/IfcWall.h>
#include <ifcpp/IFC4X3/include/IfcCivilElement.h>
#include <ifcpp/IFC4X3/include/IfcDistributionElement.h>
#include <ifcpp/IFC4X3/include/IfcElementAssembly.h>
#include <ifcpp/IFC4X3/include/IfcElementComponent.h>
#include <ifcpp/IFC4X3/include/IfcFurnishingElement.h>
#include <ifcpp/IFC4X3/include/IfcGeographicElement.h>
#include <ifcpp/IFC4X3/include/IfcTransportElement.h>
#include <ifcpp/IFC4X3/include/IfcVirtualElement.h>
#include <ifcpp/IFC4X3/include/IfcExternalSpatialElement.h>
#include <ifcpp/IFC4X3/include/IfcSpatialZone.h>
#include <ifcpp/IFC4X3/include/IfcBuildingElementPart.h>

#include <Carve/src/include/carve/carve.hpp>

#include <IBK_Exception.h>
#include <IBK_FileUtils.h>

#include <tinyxml.h>

#include "IFCC_MeshUtils.h"
#include "IFCC_Logger.h"

namespace IFCC {

struct ProgressCloser {
	ProgressCloser(IFCReader* reader, QString endText) :
		m_reader(reader),
		m_endText(endText)
	{}
	~ProgressCloser() {
		emit m_reader->progress(100, m_endText);
	}

	IFCReader*	m_reader;
	QString		m_endText;
};


const std::string VERSION = "1.0";

IFCReader::IFCReader() :
	m_hasError(false),
	m_hasWarning(false),
	m_readCompletedSuccessfully(false),
	m_convertCompletedSuccessfully(false),
	m_model(new BuildingModel),
	m_geometryConverter(m_model),
	m_site(0)
{
	m_geometryConverter.clearMessagesCallback();
	m_geometryConverter.resetModel();
	m_geometryConverter.getGeomSettings()->setNumVerticesPerCircle(16);
	m_geometryConverter.getGeomSettings()->setMinNumVerticesPerArc(4);

//	Logger::instance().set("g:/temp/IFC_Log.txt");
	m_progressDialog.reset(new QProgressDialog("IFC Reader...", "Abort read", 0, 100));
	m_progressDialog->setWindowModality(Qt::WindowModal);
	m_progressDialog->setMinimum(0);
	m_progressDialog->setMaximum(0);
	m_progressDialog->setValue(0);
//	m_progressDialog->setMinimumDuration(0);

	connect(this, &IFCReader::progress, this, &IFCReader::setProgress);
}

IFCReader::~IFCReader() {
}


void IFCReader::clear() {
	m_model.reset(new BuildingModel);
	m_geometryConverter.setModel(m_model);
	m_hasError = false;
	m_hasWarning = false;
	m_geometryConverter.clearMessagesCallback();
	m_geometryConverter.resetModel();
	m_geometryConverter.getGeomSettings()->setNumVerticesPerCircle(16);
	m_geometryConverter.getGeomSettings()->setMinNumVerticesPerArc(4);

	m_errorText.clear();
	m_warningText.clear();
	m_progressText.clear();
	m_readCompletedSuccessfully = false;

	clearConvertData();
}

void IFCReader::clearConvertData() {
	m_convertCompletedSuccessfully = false;

	m_site = Site(0);
	m_elementEntitesShape.clear();
	m_spatialEntitesShape.clear();
	m_spaceEntitesShape.clear();
	m_unknownEntitesShape.clear();
	m_buildingsShape.clear();
	m_storeysShape.clear();
	m_openingsShape.clear();
	m_externalSpatialShapes.clear();
	m_spatialZoneShapes.clear();
	m_siteShape.reset();
	m_buildingElements.clear();
	m_openings.clear();
	m_database.clear();
	m_instances.clear();
}

void IFCReader::setProgress(int val, QString text) {
	if(m_progressDialog) {
		m_progressDialog->setLabelText(text);
//		m_progressDialog->setValue(val);
		m_progressDialog->update();
		if(val == 100)
			m_progressDialog->reset();
		QApplication::processEvents();
	}
}

bool IFCReader::loadModelFromSTEPFile( const IBK::Path& filePath, shared_ptr<BuildingModel>& targetModel ) {
	m_hasError = false;
	// if file content needs to be loaded into a plain model, call resetModel() before loadModelFromFile
	std::string ext = filePath.extension();

	if( ext != "ifc" ) {
		m_errorText = "Wrong file format";
		m_hasError = true;
		return false;
	}

	// open file
	setlocale(LC_ALL, "");

	std::ifstream infile;
	bool res = IBK::open_ifstream(infile, filePath, std::ios_base::in);

	if( !res ) {
		m_errorText = "Could not open file: " + filePath.str();
		m_hasError = true;
		return false;
	}

	// get length of file content
	infile.imbue(std::locale(""));
	infile.seekg( 0, std::ios::end );
	std::streampos file_end_pos = infile.tellg();
	infile.seekg( 0, std::ios::beg );

	ReaderSTEP readerStep;
	readerStep.setMessageCallBack(this, &IFCReader::messageTarget);
	readerStep.loadModelFromStream(infile, file_end_pos, targetModel);
	return true;
}


bool IFCReader::read(const IBK::Path& filename, bool ignoreReadError) {
	clear();

	ProgressCloser progressCloser(this, tr("End of read"));
	m_progressDialog->show();

	emit progress(1,tr("Read IFC file"));
	m_filename = filename;
	m_readCompletedSuccessfully = true;
	try {
		bool res = loadModelFromSTEPFile(m_filename, m_geometryConverter.getBuildingModel());
		if(!ignoreReadError && !res) {
			m_readCompletedSuccessfully = false;
		}

		return !m_hasError;
	}
	catch (std::exception& e) {
		m_errorText = e.what();
		if(!ignoreReadError) {
			m_readCompletedSuccessfully = false;
			m_hasError = true;
		}

		return false;
	}
	return true;
}

void IFCReader::splitShapeData() {
	const std::map<std::string,shared_ptr<ProductShapeData>>& shapeDataMap = m_geometryConverter.getShapeInputData();

	for(const auto& shapeData : shapeDataMap) {
		const shared_ptr<ProductShapeData>& data = shapeData.second;
		std::string id = shapeData.first;
		std::string guid = data->m_entity_guid;
		const std::shared_ptr<IfcObjectDefinition> od = data->m_ifc_object_definition.lock();
		if(dynamic_pointer_cast<IfcElement>(od) != nullptr) {
			if(dynamic_pointer_cast<IfcWall>(od) != nullptr) {
				m_elementEntitesShape[BET_Wall].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcBeam>(od) != nullptr) {
				m_elementEntitesShape[BET_Beam].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcChimney>(od) != nullptr) {
				m_elementEntitesShape[BET_Chimney].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcCovering>(od) != nullptr) {
				m_elementEntitesShape[BET_Covering].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcCurtainWall>(od) != nullptr) {
				m_elementEntitesShape[BET_CurtainWall].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcDoor>(od) != nullptr) {
				m_elementEntitesShape[BET_Door].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcFooting>(od) != nullptr) {
				m_elementEntitesShape[BET_Footing].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcMember>(od) != nullptr) {
				m_elementEntitesShape[BET_Member].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcPile>(od) != nullptr) {
				m_elementEntitesShape[BET_Pile].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcPlate>(od) != nullptr) {
				m_elementEntitesShape[BET_Plate].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcRailing>(od) != nullptr) {
				m_elementEntitesShape[BET_Railing].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcRamp>(od) != nullptr) {
				m_elementEntitesShape[BET_Ramp].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcRampFlight>(od) != nullptr) {
				m_elementEntitesShape[BET_RampFlight].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcRoof>(od) != nullptr) {
				m_elementEntitesShape[BET_Roof].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcShadingDevice>(od) != nullptr) {
				m_elementEntitesShape[BET_ShadingDevice].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcSlab>(od) != nullptr) {
				m_elementEntitesShape[BET_Slab].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcStair>(od) != nullptr) {
				m_elementEntitesShape[BET_Stair].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcStairFlight>(od) != nullptr) {
				m_elementEntitesShape[BET_StairFlight].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcWindow>(od) != nullptr) {
				m_elementEntitesShape[BET_Window].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcFeatureElement>(od) != nullptr) {
				if(dynamic_pointer_cast<IfcOpeningElement>(od) != nullptr) {
					std::string guid = guidFromObject(od.get());
					m_openingsShape[guid] = data;
				}

				m_elementEntitesShape[BET_FeatureElement].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcCivilElement>(od) != nullptr) {
				m_elementEntitesShape[BET_CivilElement].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcDistributionElement>(od) != nullptr) {
				m_elementEntitesShape[BET_DistributionElement].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcElementAssembly>(od) != nullptr) {
				m_elementEntitesShape[BET_ElementAssembly].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcElementComponent>(od) != nullptr) {
				if(dynamic_pointer_cast<IfcBuildingElementPart>(od) != nullptr) {
					m_elementEntitesShape[BET_BuildingElementPart].push_back(data);
				}
				else {
					m_elementEntitesShape[BET_ElementComponent].push_back(data);
				}
			}
			else if(dynamic_pointer_cast<IfcFurnishingElement>(od) != nullptr) {
				m_elementEntitesShape[BET_FurnishingElement].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcGeographicElement>(od) != nullptr) {
				m_elementEntitesShape[BET_GeographicalElement].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcTransportElement>(od) != nullptr) {
				m_elementEntitesShape[BET_TransportElement].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcVirtualElement>(od) != nullptr) {
				m_elementEntitesShape[BET_VirtualElement].push_back(data);
			}
			else {
				m_elementEntitesShape[BET_All].push_back(data);
			}
		}
		else if(dynamic_pointer_cast<IfcSpatialStructureElement>(od) != nullptr) {
			std::string guid = guidFromObject(od.get());
			if(dynamic_pointer_cast<IfcSite>(od) != nullptr) {
				if(m_siteShape != nullptr)
					throw IBK::Exception("Second site found.", "IFCReader::splitShapeData");
				m_siteShape = data;
			}
			else if(dynamic_pointer_cast<IfcBuilding>(od) != nullptr) {
				m_buildingsShape[guid] = data;
			}
			else if(dynamic_pointer_cast<IfcBuildingStorey>(od) != nullptr) {
				m_storeysShape[guid] = data;
			}
			else if(dynamic_pointer_cast<IfcSpace>(od) != nullptr) {
				m_spaceEntitesShape[guid] = data;
			}
			else {
				m_spatialEntitesShape[guid] = data;
			}
		}
		else if(dynamic_pointer_cast<IfcExternalSpatialElement>(od) != nullptr) {
			std::string guid = guidFromObject(od.get());
			m_externalSpatialShapes[guid] = data;
		}
		else if(dynamic_pointer_cast<IfcSpatialZone>(od) != nullptr) {
			std::string guid = guidFromObject(od.get());
			m_spatialZoneShapes[guid] = data;
		}
		else {
			std::string guid = guidFromObject(od.get());
			m_unknownEntitesShape[guid] = data;
		}
	}
}

void IFCReader::updateBuildingElements() {
	Logger::instance() << "IFCReader::updateBuildingElements start";
	size_t elemCount = 0;
	for(auto& elems : m_elementEntitesShape) {
		elemCount += elems.second.size();
	}
	size_t currCount = 0;

	emit progress(0, tr("Update building elements"));

	m_buildingElements.clear();
	for(auto& elems : m_elementEntitesShape) {
		for(auto& elem : elems.second) {
			++currCount;
			if(elem.get() == nullptr)
				continue;

			std::shared_ptr<IfcElement> e = dynamic_pointer_cast<IfcElement>(elem->m_ifc_object_definition.lock());
			if(e == nullptr)
				continue;

			std::shared_ptr<BuildingElement> bElem(new BuildingElement(GUID_maker::instance().guid()));
			if(!bElem->set(e, elems.first))
				continue;

			if(isConstructionType(elems.first)) {
				m_buildingElements.m_constructionElements.push_back( bElem);
				BuildingElement& currbElem = *m_buildingElements.m_constructionElements.back();

				Logger::instance() << "update constr nr: " << currCount << " of " << elemCount;

				currbElem.getShapeOfParts(m_elementEntitesShape[BET_BuildingElementPart], m_convertErrors);
				currbElem.update(elem, m_openings, m_convertErrors, m_convertOptions);
				if(currbElem.surfaces().empty())
					m_buildingElements.m_elementsWithoutSurfaces.push_back(m_buildingElements.m_constructionElements.back());
			}
			else if(isConstructionSimilarType(elems.first)) {
				m_buildingElements.m_constructionSimilarElements.push_back(bElem);
				BuildingElement& currbElem = *m_buildingElements.m_constructionSimilarElements.back();

				Logger::instance() << "update similar " << currCount << " of " << elemCount;

				currbElem.update(elem, m_openings, m_convertErrors, m_convertOptions);
				if(currbElem.surfaces().empty())
					m_buildingElements.m_elementsWithoutSurfaces.push_back(m_buildingElements.m_constructionSimilarElements.back());
			}
			else if(isOpeningType(elems.first)) {
				m_buildingElements.m_openingElements.push_back(bElem);
				BuildingElement& currbElem = *m_buildingElements.m_openingElements.back();

				Logger::instance() << "update opening " << currCount << " of " << elemCount;

				currbElem.update(elem, m_openings, m_convertErrors, m_convertOptions);
				if(currbElem.surfaces().empty())
					m_buildingElements.m_elementsWithoutSurfaces.push_back(m_buildingElements.m_openingElements.back());
			}
			else {
				m_buildingElements.m_otherElements.push_back(bElem);
				BuildingElement& currbElem = *m_buildingElements.m_otherElements.back();

				Logger::instance() << "update other " << currCount << " of " << elemCount;

				currbElem.update(elem, m_openings, m_convertErrors, m_convertOptions);
				//						if(m_buildingElements.m_otherElements.back()->surfaces().empty())
				//							m_buildingElements.m_elementsWithoutSurfaces.push_back(m_buildingElements.m_otherElements.back());
			}
		}
	}
}

const ConvertOptions &IFCReader::convertOptions() const {
	return m_convertOptions;
}

IBK::Path IFCReader::filename() const {
	if(m_readCompletedSuccessfully)
		return m_filename;

	return IBK::Path();
}

bool IFCReader::hasElementsForSpaceBoundaries(BuildingElementTypes type) const {
	return m_convertOptions.m_elementsForSpaceBoundaries.find(type) != m_convertOptions.m_elementsForSpaceBoundaries.end();
}

void IFCReader::setElementsForSpaceBoundaries(BuildingElementTypes type, bool set) {
	if(set) {
		m_convertOptions.m_elementsForSpaceBoundaries.insert(type);
	}
	else {
		m_convertOptions.m_elementsForSpaceBoundaries.erase(type);
	}
}

void IFCReader::setMatchingDistances(double constructionFactor, double standardWallThickness, double openingDistance) {
	m_convertOptions.m_distanceFactor = constructionFactor;
	m_convertOptions.m_openingDistance = openingDistance;
	m_convertOptions.m_standardWallThickness = standardWallThickness;
}

void IFCReader::addNoSearchForOpenings(const QSet<BuildingElementTypes>& types) {
	m_convertOptions.addElementsForOpenings(types);
}

void IFCReader::setWritingBuildingElements(bool constructions, bool buildingElements, bool openings, bool other) {
	m_convertOptions.m_writeConstructionElements = constructions;
	m_convertOptions.m_writeBuildingElements = buildingElements;
	m_convertOptions.m_writeOpeningElements = openings;
	m_convertOptions.m_writeOtherElements = other;
}

void IFCReader::setMinimumCheckValues(double minimumDistance, double minimumArea) {
	m_convertOptions.m_distanceEps = minimumDistance;
	m_convertOptions.m_minimumSurfaceArea = minimumArea;
}

void IFCReader::setUseCSGForOpenings(bool useCSG) {
	m_convertOptions.m_useCSGForOpenings = useCSG;
}


bool IFCReader::convert(bool useSpaceBoundaries) {

	ProgressCloser progressCloser(this, tr("End converting"));

	m_progressDialog->show();

	if(!m_readCompletedSuccessfully) {
		m_errorText = "Cannot convert data because file not readed";
		return false;
	}

	emit progress(0, tr("Start converting"));

	Logger::instance() << "start convert";

	clearConvertData();

	m_useSpaceBoundaries = useSpaceBoundaries;

	bool subtractOpenings = false;

	try {

		emit progress(20, tr("Convert geometry"));
		// convert IFC geometric representations into Carve geometry
		const double length_in_meter = m_geometryConverter.getBuildingModel()->getUnitConverter()->getLengthInMeterFactor();
		m_geometryConverter.getGeomSettings()->setMinimumSurfaceArea(m_convertOptions.m_minimumSurfaceArea);
		m_geometryConverter.setCsgEps(1.5e-08 * length_in_meter);
		m_geometryConverter.convertGeometry(subtractOpenings, m_convertErrors);

		splitShapeData();

		emit progress(50, tr("Split shape data"));

		m_openings.clear();
		for(auto& openShape : m_openingsShape) {
			std::shared_ptr<IfcOpeningElement> o = dynamic_pointer_cast<IfcOpeningElement>(openShape.second->m_ifc_object_definition.lock());
			if(o == nullptr)
				continue;

			Opening opening(GUID_maker::instance().guid());
			if(opening.set(o)) {
				m_openings.push_back(opening);
				m_openings.back().update(openShape.second, m_convertErrors);
			}
		}

		emit progress(80, tr("Update openings"));

		try {
			emit progress(85, tr("Update building elements"));
			updateBuildingElements();

//			Logger::instance() << "updateBuildingElements";
		}
		catch (std::exception& e) {
			ConvertError err;
			err.m_objectType = OT_Unknown;
			err.m_errorText = "Exception: '" + std::string(e.what()) + "' while converting ifc file.";
			m_convertErrors.push_back(err);
			m_hasError = true;

			return false;
		}
		catch (...) {
			ConvertError err;
			err.m_objectType = OT_Unknown;
			err.m_errorText = "Unknown exception: while converting ifc file.";
			m_convertErrors.push_back(err);
			m_hasError = true;

			return false;
		}

		emit progress(90, tr("Set containing elements"));
		for(std::shared_ptr<BuildingElement>& openingElement : m_buildingElements.m_openingElements) {
			openingElement->setContainingElements(m_openings);
			openingElement->setContainedConstructionThickesses(m_buildingElements.m_constructionElements);
			openingElement->setContainedConstructionThickesses(m_buildingElements.m_constructionSimilarElements);
		}

//		Logger::instance() << "setContainingElements";

		for(const auto& elem : m_buildingElements.m_elementsWithoutSurfaces) {
			m_convertErrors.push_back({OT_BuildingElement, elem->m_id, "Building element has no surface"});
		}

		checkAndMatchOpeningsToConstructions();


		emit progress(95, tr("Collect data"));
		m_database.collectData(m_buildingElements);

		Logger::instance() << "collectData";

		emit progress(97, tr("Update storeys"));

		bool siteExist = m_siteShape != nullptr;
		if(m_siteShape == nullptr) {
		// create own site?
		}
		else {
			std::shared_ptr<IfcSpatialStructureElement> se = std::dynamic_pointer_cast<IfcSpatialStructureElement>(m_siteShape->m_ifc_object_definition.lock());
			m_site.set(se, m_siteShape, m_buildingsShape, m_convertErrors);
		}

		if(siteExist) {

			for(auto& building : m_site.m_buildings) {
				building->fetchStoreys(m_storeysShape, m_spaceEntitesShape, m_site.m_buildings.size() == 1);
				bool res = building->updateStoreys(m_elementEntitesShape, m_spaceEntitesShape, m_geometryConverter.getBuildingModel()->getUnitConverter(),
									   m_buildingElements, m_openings, m_useSpaceBoundaries, m_convertErrors, m_convertOptions);
				if( !res) {
					m_convertErrors.push_back({OT_Building, -1, "No connection between building and storeys"});
					m_errorText = "No connection between building and storeys";
					m_hasError = true;
					return false;
				}
			}


		}
		else {
			m_convertErrors.push_back({OT_Site, -1, "No site"});
			m_errorText = "No site";
			m_hasError = true;
			return false;
		}

//		Logger::instance() << "updateStoreys";

		if(m_repairFlags.m_removeDoubledSBs) {
			std::vector<std::shared_ptr<Space>> spaces = m_site.allSpaces();

			for(auto space : spaces) {
				space->removeDublicatedSpaceBoundaries(m_convertOptions);
			}
		}

		emit progress(98, tr("Collect component instances"));
		m_instances.collectComponentInstances(m_buildingElements, m_database, m_site, m_convertErrors, m_convertOptions);

//		Logger::instance() << "collectComponentInstances";

		if(!m_convertErrors.empty()) {
			m_hasError = true;
//			return false;
		}

		m_convertCompletedSuccessfully = true;

		emit progress(100, tr("Convert completed successfully"));
//		Logger::instance() << "m_convertCompletedSuccessfully";

		return true;

	}
	catch (std::exception& e) {
		ConvertError err;
		err.m_objectType = OT_Unknown;
		err.m_errorText = "Exception: '" + std::string(e.what()) + "' while converting ifc file.";
		m_convertErrors.push_back(err);
		m_hasError = true;

		return false;
	}

	return false;
}

int IFCReader::totalNumberOfIFCEntities() const {
	if(!m_readCompletedSuccessfully)
		return 0;

	return m_model->getMapIfcEntities().size();
}

int IFCReader::numberOfIFCSpaceBoundaries() const {
	if(!m_readCompletedSuccessfully)
		return 0;

	const auto& ifcMap = m_model->getMapIfcEntities();
	int count = 0;
	for(const auto& item : ifcMap) {
		shared_ptr<IfcRelSpaceBoundary> sb = dynamic_pointer_cast<IfcRelSpaceBoundary>(item.second);
		if(sb)
			++count;
	}
	return count;
}

bool IFCReader::checkEssentialIFCs(QString& errmsg, int& buildings, int& spaces) {
	buildings = 0;
	spaces = 0;
	bool siteExist = false;
	for(const auto& item : m_model->getMapIfcEntities()) {
		if(dynamic_pointer_cast<IfcSpatialStructureElement>(item.second) != nullptr) {
			if(dynamic_pointer_cast<IfcSite>(item.second) != nullptr) {
				siteExist = true;
			}
			else if(dynamic_pointer_cast<IfcBuilding>(item.second) != nullptr) {
				++buildings;
			}
			else if(dynamic_pointer_cast<IfcSpace>(item.second) != nullptr) {
				++spaces;
			}
		}
	}
	if(!siteExist) {
		errmsg = tr("No building site.");
		return false;
	}
	if(buildings == 0) {
		errmsg = tr("No buildings.");
		return false;
	}
	if(spaces == 0) {
		errmsg = tr("No spaces.");
		return false;
	}
	return true;
}

int IFCReader::checkForEqualSpaceBoundaries(std::vector<std::pair<int,int>>& equalSBs) const {
	equalSBs.clear();
	std::vector<std::shared_ptr<Space>> spaces = m_site.allSpaces();

	for(const auto& space : spaces) {
		space->checkForEqualSpaceBoundaries(equalSBs, m_convertOptions);
	}
	return equalSBs.size();
}

int IFCReader::checkForUniqueSubSurfacesInSpaces(std::vector<std::pair<int,std::vector<int>>>& res) const {
	res.clear();
	std::vector<std::shared_ptr<Space>> spaces = m_site.allSpaces();

	for(const auto& space : spaces) {
		std::vector<int> subRes = space->checkUniqueSubSurfaces();
		if(!subRes.empty())
			res.push_back({space->m_ifcId, subRes});
	}
	return res.size();
}

std::set<std::pair<int,int>> IFCReader::checkForIntersectedSpace() const {
	std::set<std::pair<int,int>> res;
	std::vector<std::shared_ptr<Space>> spaces = m_site.allSpaces();
	if(spaces.size() < 2)
		return res;

	for(size_t i=0; i<spaces.size()-1; ++i) {
		for(size_t j=i+1; j<spaces.size(); ++j) {
			if(spaces[i]->isIntersected(*spaces[j], m_convertOptions))
				res.insert({spaces[i]->m_ifcId,spaces[j]->m_ifcId});
		}
	}
	return res;
}

std::set<std::pair<int, int> > IFCReader::checkForSpaceWithSameSpaceBoundaries() const {
	std::set<std::pair<int,int>> res;
	std::vector<std::shared_ptr<Space>> spaces = m_site.allSpaces();
	if(spaces.size() < 2)
		return res;

	for(size_t i=0; i<spaces.size()-1; ++i) {
		for(size_t j=i+1; j<spaces.size(); ++j) {
			if(spaces[i]->shareSameSpaceBoundary(*spaces[j]))
				res.insert({spaces[i]->m_ifcId,spaces[j]->m_ifcId});
		}
	}
	return res;
}

std::vector<int> IFCReader::checkForWrongSurfaceIds() {
	return m_instances.checkForWrongSurfaceIds(m_site);
}

int IFCReader::checkForNotRelatedOpenings() const {
	int count = 0;
	for(const Opening& op : m_openings) {
		if(!op.hasSpaceBoundary())
			++count;
	}
	return count;
}

bool IFCReader::removeDoubledSBs() const {
	return m_repairFlags.m_removeDoubledSBs;
}

void IFCReader::setRemoveDoubledSBs(bool removeDoubledSBs) {
	m_repairFlags.m_removeDoubledSBs = removeDoubledSBs;
}

QString IFCReader::nameForId(int id, Name_Id_Type type) const {
	switch(type) {
		case NIT_Space: {
			const Space* sp = m_site.spaceWithIfcId(id);
			if(sp != nullptr) {
				if(sp->m_longName.empty())
						return QString::fromStdString(sp->m_name);
				return QString::fromStdString(sp->m_longName);
			}
		}
		case NIT_SpaceBoundary: {
			const SpaceBoundary* sp = m_site.spaceBoundaryWithId(id);
			if(sp != nullptr)
				return QString::fromStdString(sp->m_name);
		}
	}
	return QString();
}


void IFCReader::writeXML(const IBK::Path & filename) const {
	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
	doc.LinkEndChild( decl );

	TiXmlElement * root = new TiXmlElement( "VicusProject" );
	doc.LinkEndChild(root);

	root->SetAttribute("fileVersion", VERSION);

	TiXmlElement * e = new TiXmlElement("Project");
	root->LinkEndChild(e);

	m_site.writeXML(e);

	m_instances.writeXML(e);

	m_database.writeXML(e);

	if(m_convertOptions.m_writeBuildingElements) {
		TiXmlElement * pg = new TiXmlElement("PlainGeometry");
		e->LinkEndChild(pg);

		TiXmlElement * child = new TiXmlElement("Surfaces");
		for(const auto& elem : m_buildingElements.allConstructionElements()) {
			if(elem->surfaces().empty())
				continue;
			for(auto surf : elem->surfaces()) {
				surf.writeXML(child);
			}

		}
	}

	// other files

	doc.SaveFile( filename.c_str() );
}


void IFCReader::setVicusProjectText(QString& projectText) {
	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
	doc.LinkEndChild( decl );

	TiXmlElement * root = new TiXmlElement( "VicusProject" );
	doc.LinkEndChild(root);

	root->SetAttribute("fileVersion", VERSION);

	TiXmlElement * e = new TiXmlElement("Project");
	root->LinkEndChild(e);

	m_site.writeXML(e);

	m_instances.writeXML(e);

	m_database.writeXML(e);

	bool writePlainGeometry = m_convertOptions.m_writeConstructionElements;
	writePlainGeometry = writePlainGeometry || m_convertOptions.m_writeBuildingElements;
	writePlainGeometry = writePlainGeometry || m_convertOptions.m_writeOpeningElements;
	writePlainGeometry = writePlainGeometry || m_convertOptions.m_writeOtherElements;
	if(writePlainGeometry) {
		TiXmlElement * pg = new TiXmlElement("PlainGeometry");
		e->LinkEndChild(pg);

		TiXmlElement * child = new TiXmlElement("Surfaces");
		pg->LinkEndChild(child);
		if(m_convertOptions.m_writeConstructionElements) {
			for(const auto& elem : m_buildingElements.m_constructionElements) {
				if(elem->surfaces().empty())
					continue;
				for(auto surf : elem->surfaces()) {
					surf.writeXML(child);
				}
			}
		}
		if(m_convertOptions.m_writeBuildingElements) {
			for(const auto& elem : m_buildingElements.m_constructionSimilarElements) {
				if(elem->surfaces().empty())
					continue;
				for(auto surf : elem->surfaces()) {
					surf.writeXML(child);
				}
			}
		}

		if(m_convertOptions.m_writeOpeningElements) {
			for(const auto& elem : m_buildingElements.m_openingElements) {
				if(elem->surfaces().empty())
					continue;
				for(auto surf : elem->surfaces()) {
					surf.writeXML(child);
				}
			}
		}

		if(m_convertOptions.m_writeOtherElements) {
			for(const auto& elem : m_buildingElements.m_otherElements) {
				if(elem->surfaces().empty())
					continue;
				for(auto surf : elem->surfaces()) {
					surf.writeXML(child);
				}
			}
		}
	}

	// other files

	// Declare a printer
	TiXmlPrinter printer;

	// attach it to the document you want to convert in to a std::string
	doc.Accept(&printer);

	// Create a std::string and copy your document data in to the string
	std::string str = printer.CStr();
	projectText = QString::fromStdString(str);
}


struct SpaceBoundaryEvaluation {
	enum Type {
		Construction,
		Opening,
		Virtual,
		Missing,
		Unknown
	};
	Type		m_type;
	QString	m_name;
	QString	m_nameRelatedElement;
	QString	m_nameRelatedSpace;
	BuildingElementTypes	m_typeRelatedElement;

	static QString typeString(Type type) {
		switch(type) {
			case Construction: return "Construction element";
			case Opening: return "Openening element";
			case Virtual: return "Virtual";
			case Missing: return "Missing";
			case Unknown: return "Unknown connection";
		}
	}
};

QStringList IFCReader::messages() const {
	QStringList result;
	result << tr("Messages:");
	std::vector<std::shared_ptr<SpaceBoundary>> spaceBoundaries = m_site.allSpaceBoundaries();

	size_t sbCount = spaceBoundaries.size();
	if(sbCount > 0) {
		int sbConstruction = 0;
		int sbOpenings = 0;
		int sbMissing = 0;
		int sbVirtual = 0;
		for(const auto& sb : spaceBoundaries) {
			if(sb->isConstructionElement())
				++sbConstruction;
			if(sb->isOpeningElement())
				++sbOpenings;
			if(sb->isMissing())
				++sbMissing;
			if(sb->isVirtual())
				++sbVirtual;
		}
		result << tr("%1 space boundaries.").arg(sbCount);
		if(sbConstruction > 0)
			result << tr("%1 connected with construction elements").arg(sbConstruction);
		if(sbOpenings > 0)
			result << tr("%1 connected with opening elements").arg(sbOpenings);
		if(sbMissing > 0)
			result << tr("%1 without connection").arg(sbMissing);
		if(sbVirtual > 0)
			result << tr("%1 virtual surfaces").arg(sbVirtual);
	}
	else {

	}
	return result;
}

QStringList IFCReader::statistic() const {
	QStringList result;
	result << tr("Statistic:<br>");
	result << tr("%1 buildings.<br>").arg(m_site.m_buildings.size());
	for(const auto& building : m_site.m_buildings) {
		result << tr("Building %1 with %2 storeys.<br>").arg(QString::fromStdString(building->m_name))
				.arg(building->storeys().size());
		for(const auto& storey : building->storeys()) {
			result << tr("  Storey %1 with %2 spaces.<br>").arg(QString::fromStdString(storey->m_name))
					.arg(storey->spaces().size());
			for(const auto& space : storey->spaces()) {
				result << tr("    Space %1 with %2 space boundaries.<br>")
						  .arg(QString::fromStdString(space->m_name+" - "+space->m_longName))
						  .arg(space->spaceBoundaries().size());
				for(const auto& sb : space->spaceBoundaries()) {
					if(sb->isMissing()) {
						result << tr("    Space boundary %1 with missing connection.<br>").arg(QString::fromStdString(sb->m_name));
					}
					else if(sb->isVirtual()) {
						result << tr("    Space boundary %1 is virtual.<br>").arg(QString::fromStdString(sb->m_name));
					}
					else {
						result << tr("    Space boundary %1 with %2 subsurfaces.<br>").arg(QString::fromStdString(sb->m_name))
								.arg(sb->containedOpeningSpaceBoundaries().size());
					}
				}
			}
		}
	}
	result << "<br>Databases<br>";
	result << tr("\t%1 materials<br>").arg(m_database.m_materials.size());
	for(const auto& mat : m_database.m_materials) {
		result << tr("    %1 - id %2<br>").arg(QString::fromStdString(mat.second.m_name)).arg(mat.first);
	}
	result << tr("  %1 constructions<br>").arg(m_database.m_constructions.size());
	for(const auto& con : m_database.m_constructions) {
		result << tr("    Construction id %1 with %2 layers<br>").arg(con.first).arg(con.second.m_layers.size());
	}
	result << tr("  %1 windows<br>").arg(m_database.m_windows.size());
	for(const auto& win : m_database.m_windows) {
		result << tr("    Window %1 id %2<br>").arg(QString::fromStdString(win.second.m_name)).arg(win.first);
	}
	result << tr("  %1 windows<br>").arg(m_database.m_windowGlazings.size());
	for(const auto& wgl : m_database.m_windowGlazings) {
		result << tr("    Window %1 id %2<br>").arg(QString::fromStdString(wgl.second.m_name)).arg(wgl.first);
	}

	result << QString() << tr("Space boundary list <br>") << QString();

	std::vector<std::shared_ptr<SpaceBoundary>> spaceBoundaries = m_site.allSpaceBoundaries();
	std::vector<SpaceBoundaryEvaluation> sbEvals;
	for(const auto& sb : spaceBoundaries) {
		SpaceBoundaryEvaluation sbEval;
		if(sb->isConstructionElement())
			sbEval.m_type = SpaceBoundaryEvaluation::Construction;
		else if(sb->isOpeningElement())
			sbEval.m_type = SpaceBoundaryEvaluation::Opening;
		else if(sb->isVirtual())
			sbEval.m_type = SpaceBoundaryEvaluation::Virtual;
		else if(sb->isMissing())
			sbEval.m_type = SpaceBoundaryEvaluation::Missing;
		else {
			sbEval.m_type = SpaceBoundaryEvaluation::Unknown;
		}
		sbEval.m_name = QString::fromStdString(sb->m_name);
		sbEval.m_nameRelatedElement = QString::fromStdString(sb->nameRelatedElement());
		sbEval.m_nameRelatedSpace = QString::fromStdString(sb->nameRelatedSpace());
		sbEval.m_typeRelatedElement = sb->typeRelatedElement();
		QString text = sbEval.m_name + "\tis a " + QString::fromStdString(objectTypeToString(sb->typeRelatedElement()));
		text += "  connected with: " + sbEval.m_nameRelatedElement;
		text += "  contained in: " + sbEval.m_nameRelatedSpace;
		result << text + "<br>";
	}

	return result;
}

const std::vector<ConvertError>& IFCReader::convertErrors() const {
	return m_convertErrors;
}

void IFCReader::messageTarget( void* ptr, shared_ptr<StatusCallback::Message> m ) {
	if(ptr == nullptr)
		return;

	IFCReader* myself = reinterpret_cast<IFCReader*>(ptr);

	std::string reporting_function_str( m->m_reporting_function );
	std::string position;
	if( m->m_entity ) {
		position = "IFC entity: #" + std::to_string(m->m_entity->m_tag) + "=" + std::to_string(m->m_entity->classID());
	}
	if(m->m_message_type == StatusCallback::MESSAGE_TYPE_ERROR) {
		myself->m_hasError = true;
		myself->m_errorText = "Error from: " + reporting_function_str + " in " + position;
		myself->m_errorText += m->m_message_text + "\n";
	}
	if(m->m_message_type == StatusCallback::MESSAGE_TYPE_WARNING) {
		myself->m_hasWarning = true;
		myself->m_warningText = "Error from: " + reporting_function_str + " in " + position;
		myself->m_warningText += m->m_message_text + "\n";
	}
}

bool IFCReader::typeByGuid(const std::string& guid, std::pair<BuildingElementTypes,std::shared_ptr<ProductShapeData>>& res) {
	for(const auto& elemType : m_elementEntitesShape) {
		for(const auto& elem : elemType.second) {
			if(guid == elem->m_entity_guid) {
				res = std::pair<BuildingElementTypes,std::shared_ptr<ProductShapeData>>(elemType.first,elem);
				return true;
			}
		}
	}
	return false;
}

void IFCReader::checkAndMatchOpeningsToConstructions() {
	for(Opening& opening : m_openings) {
		if(opening.isConnectedToOpeningElement())
			continue;

		double currDist = 1e20;
		int constructionId = -1;
		// loop over all opening element constructions
		for(const auto& elem : m_buildingElements.m_openingElements) {
			for(size_t cosi=0; cosi<opening.surfaces().size(); ++cosi) {
				const Surface& currentOpeningSurf = opening.surfaces()[cosi];

				for(const Surface& constructionSurf : elem->surfaces()) {
					double dist = currentOpeningSurf.distanceToParallelPlane(constructionSurf, m_convertOptions.m_distanceEps);
					if(dist > m_convertOptions.m_openingDistance)
						continue;

					bool intersected = constructionSurf.isIntersected(currentOpeningSurf);
					if(intersected) {
						if(dist < currDist) {
							currDist = dist;
							constructionId = elem->m_id;
						}
					}
				} // construction surface loop
			} // opening surface loop

		} // building element id loop

		if(constructionId > -1)
			opening.addOpeningElementId(constructionId);
	} // opening loop

}

} // end namespace
