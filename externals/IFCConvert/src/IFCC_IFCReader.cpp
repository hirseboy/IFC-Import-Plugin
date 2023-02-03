#include "IFCC_IFCReader.h"

#include "IFCC_Helper.h"

#include <QDebug>

#include <ifcpp/IFC4/include/IfcRelSpaceBoundary.h>
#include <ifcpp/IFC4/include/IfcWall.h>
#include <ifcpp/IFC4/include/IfcBeam.h>
#include <ifcpp/IFC4/include/IfcChimney.h>
#include <ifcpp/IFC4/include/IfcColumn.h>
#include <ifcpp/IFC4/include/IfcCovering.h>
#include <ifcpp/IFC4/include/IfcCurtainWall.h>
#include <ifcpp/IFC4/include/IfcDoor.h>
#include <ifcpp/IFC4/include/IfcFooting.h>
#include <ifcpp/IFC4/include/IfcMember.h>
#include <ifcpp/IFC4/include/IfcPile.h>
#include <ifcpp/IFC4/include/IfcPlate.h>
#include <ifcpp/IFC4/include/IfcRailing.h>
#include <ifcpp/IFC4/include/IfcRamp.h>
#include <ifcpp/IFC4/include/IfcRampFlight.h>
#include <ifcpp/IFC4/include/IfcRoof.h>
#include <ifcpp/IFC4/include/IfcShadingDevice.h>
#include <ifcpp/IFC4/include/IfcSlab.h>
#include <ifcpp/IFC4/include/IfcStair.h>
#include <ifcpp/IFC4/include/IfcStairFlight.h>
#include <ifcpp/IFC4/include/IfcWall.h>
#include <ifcpp/IFC4/include/IfcCivilElement.h>
#include <ifcpp/IFC4/include/IfcDistributionElement.h>
#include <ifcpp/IFC4/include/IfcElementAssembly.h>
#include <ifcpp/IFC4/include/IfcElementComponent.h>
#include <ifcpp/IFC4/include/IfcFurnishingElement.h>
#include <ifcpp/IFC4/include/IfcGeographicElement.h>
#include <ifcpp/IFC4/include/IfcTransportElement.h>
#include <ifcpp/IFC4/include/IfcVirtualElement.h>
#include <ifcpp/IFC4/include/IfcExternalSpatialElement.h>
#include <ifcpp/IFC4/include/IfcSpatialZone.h>

#include <IBK_Exception.h>

#include <tinyxml.h>

namespace IFCC {

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


bool IFCReader::read(const IBK::Path& filename, bool ignoreReadError) {
	clear();

	m_filename = filename;
	m_readCompletedSuccessfully = true;
	try {
		ReaderSTEP readerStep;
		readerStep.setMessageCallBack(this, &IFCReader::messageTarget);
		readerStep.loadModelFromFile(m_filename.wstr(), m_geometryConverter.getBuildingModel());
		if(!ignoreReadError && m_hasError) {
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
		std::wstring guid = data->m_entity_guid;
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
				m_elementEntitesShape[BET_ElementComponent].push_back(data);
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

bool IFCReader::convert(bool useSpaceBoundaries) {

	if(!m_readCompletedSuccessfully) {
		m_errorText = "Cannot convert data because file not readed";
		return false;
	}

	clearConvertData();

	m_useSpaceBoundaries = useSpaceBoundaries;

	bool subtractOpenings = false;

	try {

		// convert IFC geometric representations into Carve geometry
		const double length_in_meter = m_geometryConverter.getBuildingModel()->getUnitConverter()->getLengthInMeterFactor();
		m_geometryConverter.setCsgEps(1.5e-08 * length_in_meter);
		m_geometryConverter.convertGeometry(subtractOpenings, m_convertErrors);

		splitShapeData();

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

		m_buildingElements.clear();
		for(auto& elems : m_elementEntitesShape) {
			for(auto& elem : elems.second) {
				std::shared_ptr<IfcElement> e = dynamic_pointer_cast<IfcElement>(elem->m_ifc_object_definition.lock());
				if(e == nullptr)
					continue;

				std::shared_ptr<BuildingElement> bElem(new BuildingElement(GUID_maker::instance().guid()));
				if(bElem->set(e, elems.first)) {
					if(isConstructionType(elems.first)) {
						m_buildingElements.m_constructionElements.push_back( bElem);
						m_buildingElements.m_constructionElements.back()->update(elem, m_openings, m_convertErrors);
						if(m_buildingElements.m_constructionElements.back()->surfaces().empty())
							m_buildingElements.m_elementsWithoutSurfaces.push_back(m_buildingElements.m_constructionElements.back());
					}
					else if(isConstructionSimilarType(elems.first)) {
						m_buildingElements.m_constructionSimilarElements.push_back(bElem);
						m_buildingElements.m_constructionSimilarElements.back()->update(elem, m_openings, m_convertErrors);
						if(m_buildingElements.m_constructionSimilarElements.back()->surfaces().empty())
							m_buildingElements.m_elementsWithoutSurfaces.push_back(m_buildingElements.m_constructionSimilarElements.back());
					}
					else if(isOpeningType(elems.first)) {
						m_buildingElements.m_openingElements.push_back(bElem);
						m_buildingElements.m_openingElements.back()->update(elem, m_openings, m_convertErrors);
						if(m_buildingElements.m_openingElements.back()->surfaces().empty())
							m_buildingElements.m_elementsWithoutSurfaces.push_back(m_buildingElements.m_openingElements.back());
					}
					else {
						m_buildingElements.m_otherElements.push_back(bElem);
						m_buildingElements.m_otherElements.back()->update(elem, m_openings, m_convertErrors);
//						if(m_buildingElements.m_otherElements.back()->surfaces().empty())
//							m_buildingElements.m_elementsWithoutSurfaces.push_back(m_buildingElements.m_otherElements.back());
					}
				}
				else
					continue;
			}
		}

		for(std::shared_ptr<BuildingElement>& openingElement : m_buildingElements.m_openingElements) {
			openingElement->setContainingElements(m_openings);
			openingElement->setContainedConstructionThickesses(m_buildingElements.m_constructionElements);
			openingElement->setContainedConstructionThickesses(m_buildingElements.m_constructionSimilarElements);
		}

		for(const auto& elem : m_buildingElements.m_elementsWithoutSurfaces) {
			m_convertErrors.push_back({OT_BuildingElement, elem->m_id, "Building element has no surface"});
		}


		m_database.collectData(m_buildingElements);

		if(m_siteShape != nullptr) {
			std::shared_ptr<IfcSpatialStructureElement> se = std::dynamic_pointer_cast<IfcSpatialStructureElement>(m_siteShape->m_ifc_object_definition.lock());
			m_site.set(se, m_siteShape, m_buildingsShape, m_convertErrors);
			for(auto& building : m_site.m_buildings) {
				building->fetchStoreys(m_storeysShape, m_spaceEntitesShape);
				building->updateStoreys(m_elementEntitesShape, m_spaceEntitesShape, m_geometryConverter.getBuildingModel()->getUnitConverter(),
									   m_buildingElements, m_openings, m_useSpaceBoundaries, m_convertErrors);
			}
		}
		else {
			m_convertErrors.push_back({OT_Site, -1, "No site"});
			m_errorText = "No site";
			m_hasError = true;
			return false;
		}

		if(m_repairFlags.m_removeDoubledSBs) {
			std::vector<std::shared_ptr<Space>> spaces = m_site.allSpaces();

			for(auto space : spaces) {
				space->removeDublicatedSpaceBoundaries();
			}
		}

		m_instances.collectComponentInstances(m_buildingElements, m_database, m_site, m_convertErrors);

		if(!m_convertErrors.empty()) {
			m_hasError = true;
//			return false;
		}

		m_convertCompletedSuccessfully = true;

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
		space->checkForEqualSpaceBoundaries(equalSBs);
	}
	return equalSBs.size();
}

int IFCReader::checkForUniqueSubSurfacesInSpaces(std::vector<std::pair<int,std::vector<int>>>& res) const {
	res.clear();
	std::vector<std::shared_ptr<Space>> spaces = m_site.allSpaces();

	for(const auto& space : spaces) {
		std::vector<int> subRes = space->checkUniqueSubSurfaces();
		if(!subRes.empty())
			res.push_back({space->m_id, subRes});
	}
	return res.size();
}

int IFCReader::checkForIntersectedSpace() const {
	std::vector<std::shared_ptr<Space>> spaces = m_site.allSpaces();
	if(spaces.size() < 2)
		return 0;

	int count = 0;
	for(size_t i=0; i<spaces.size()-1; ++i) {
		for(size_t j=i+1; j<spaces.size(); ++j) {
			if(spaces[i]->isIntersected(*spaces[j]))
				++count;
		}
	}
	return count;
}

bool IFCReader::flipPolygons() const {
	return m_repairFlags.m_flipPolygons;
}

void IFCReader::setFlipPolygons(bool flipPolygons) {
	m_repairFlags.m_flipPolygons = flipPolygons;
}

bool IFCReader::removeDoubledSBs() const {
	return m_repairFlags.m_removeDoubledSBs;
}

void IFCReader::setRemoveDoubledSBs(bool removeDoubledSBs) {
	m_repairFlags.m_removeDoubledSBs = removeDoubledSBs;
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

	m_site.writeXML(e, m_repairFlags.m_flipPolygons);

	m_instances.writeXML(e);

	m_database.writeXML(e);

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

	m_site.writeXML(e, m_repairFlags.m_flipPolygons);

	m_instances.writeXML(e);

	m_database.writeXML(e);

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
	if( m->m_entity )
	{
		position = "IFC entity: #" + std::to_string(m->m_entity->m_entity_id) + "=" + m->m_entity->className();
	}
	if(m->m_message_type == StatusCallback::MESSAGE_TYPE_ERROR) {
		myself->m_hasError = true;
		myself->m_errorText = "Error from: " + reporting_function_str + " in " + position;
		myself->m_errorText += ws2s(m->m_message_text);
	}
	if(m->m_message_type == StatusCallback::MESSAGE_TYPE_WARNING) {
		myself->m_hasWarning = true;
		myself->m_warningText = "Error from: " + reporting_function_str + " in " + position;
		myself->m_warningText += ws2s(m->m_message_text);
	}
}

bool IFCReader::typeByGuid(const std::string& guid, std::pair<BuildingElementTypes,std::shared_ptr<ProductShapeData>>& res) {
	for(const auto& elemType : m_elementEntitesShape) {
		for(const auto& elem : elemType.second) {
			if(guid == ws2s(elem->m_entity_guid)) {
				res = std::pair<BuildingElementTypes,std::shared_ptr<ProductShapeData>>(elemType.first,elem);
				return true;
			}
		}
	}
	return false;
}

} // end namespace
