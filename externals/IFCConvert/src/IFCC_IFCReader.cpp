#include "IFCC_IFCReader.h"

#include "IFCC_Helper.h"

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
	m_model(new BuildingModel),
	m_geometryConverter(m_model),
	m_hasError(false),
	m_hasWarning(false),
	m_readCompletedSuccessfully(false),
	m_convertCompletedSuccessfully(false),
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
	m_site = Site(0);
	m_geometryConverter.clearMessagesCallback();
	m_geometryConverter.resetModel();
	m_geometryConverter.getGeomSettings()->setNumVerticesPerCircle(16);
	m_geometryConverter.getGeomSettings()->setMinNumVerticesPerArc(4);

	m_errorText.clear();
	m_warningText.clear();
	m_progressText.clear();
	m_readCompletedSuccessfully = false;
	m_convertCompletedSuccessfully = false;

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
			// i have an element
			if(dynamic_pointer_cast<IfcWall>(od) != nullptr) {
				m_elementEntitesShape[OT_Wall].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcBeam>(od) != nullptr) {
				m_elementEntitesShape[OT_Beam].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcChimney>(od) != nullptr) {
				m_elementEntitesShape[OT_Chimney].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcCovering>(od) != nullptr) {
				m_elementEntitesShape[OT_Covering].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcCurtainWall>(od) != nullptr) {
				m_elementEntitesShape[OT_CurtainWall].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcDoor>(od) != nullptr) {
				m_elementEntitesShape[OT_Door].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcFooting>(od) != nullptr) {
				m_elementEntitesShape[OT_Footing].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcMember>(od) != nullptr) {
				m_elementEntitesShape[OT_Member].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcPile>(od) != nullptr) {
				m_elementEntitesShape[OT_Pile].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcPlate>(od) != nullptr) {
				m_elementEntitesShape[OT_Plate].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcRailing>(od) != nullptr) {
				m_elementEntitesShape[OT_Railing].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcRamp>(od) != nullptr) {
				m_elementEntitesShape[OT_Ramp].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcRampFlight>(od) != nullptr) {
				m_elementEntitesShape[OT_RampFlight].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcRoof>(od) != nullptr) {
				m_elementEntitesShape[OT_Roof].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcShadingDevice>(od) != nullptr) {
				m_elementEntitesShape[OT_ShadingDevice].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcSlab>(od) != nullptr) {
				m_elementEntitesShape[OT_Slab].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcStair>(od) != nullptr) {
				m_elementEntitesShape[OT_Stair].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcStairFlight>(od) != nullptr) {
				m_elementEntitesShape[OT_StairFlight].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcWindow>(od) != nullptr) {
				m_elementEntitesShape[OT_Window].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcFeatureElement>(od) != nullptr) {
				if(dynamic_pointer_cast<IfcOpeningElement>(od) != nullptr) {
					std::string guid = guidFromObject(od.get());
					m_openingsShape[guid] = data;
				}

				m_elementEntitesShape[OT_FeatureElement].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcCivilElement>(od) != nullptr) {
				m_elementEntitesShape[OT_CivilElement].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcDistributionElement>(od) != nullptr) {
				m_elementEntitesShape[OT_DistributionElement].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcElementAssembly>(od) != nullptr) {
				m_elementEntitesShape[OT_ElementAssembly].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcElementComponent>(od) != nullptr) {
				m_elementEntitesShape[OT_ElementComponent].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcFurnishingElement>(od) != nullptr) {
				m_elementEntitesShape[OT_FurnishingElement].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcGeographicElement>(od) != nullptr) {
				m_elementEntitesShape[OT_GeographicalElement].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcTransportElement>(od) != nullptr) {
				m_elementEntitesShape[OT_TransportElement].push_back(data);
			}
			else if(dynamic_pointer_cast<IfcVirtualElement>(od) != nullptr) {
				m_elementEntitesShape[OT_VirtualElement].push_back(data);
			}
			else {
				m_elementEntitesShape[OT_All].push_back(data);
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

void IFCReader::IFCReader::updateSpaceConnections() {

}

bool IFCReader::convert(bool useSpaceBoundaries) {
	m_useSpaceBoundaries = useSpaceBoundaries;

	m_convertCompletedSuccessfully = false;
	if(!m_readCompletedSuccessfully) {
		m_errorText = "Cannot convert data because file not readed";
		return false;
	}

	bool subtractOpenings = false;

	std::string errtxt;
	try {

		// convert IFC geometric representations into Carve geometry
		const double length_in_meter = m_geometryConverter.getBuildingModel()->getUnitConverter()->getLengthInMeterFactor();
		m_geometryConverter.setCsgEps(1.5e-08 * length_in_meter);
		m_geometryConverter.convertGeometry(subtractOpenings);

		splitShapeData();

		m_openings.clear();
		for(auto& openShape : m_openingsShape) {
			std::shared_ptr<IfcOpeningElement> o = dynamic_pointer_cast<IfcOpeningElement>(openShape.second->m_ifc_object_definition.lock());
			if(o == nullptr)
				continue;

			Opening opening(GUID_maker::instance().guid());
			if(opening.set(o)) {
				m_openings.push_back(opening);
				m_openings.back().update(openShape.second);
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
						m_buildingElements.m_constructionElements.back()->update(elem, m_openings);
					}
					else if(isConstructionSimilarType(elems.first)) {
						m_buildingElements.m_constructionSimilarElements.push_back(bElem);
						m_buildingElements.m_constructionSimilarElements.back()->update(elem, m_openings);
					}
					else if(isOpeningType(elems.first)) {
						m_buildingElements.m_openingElemnts.push_back(bElem);
						m_buildingElements.m_openingElemnts.back()->update(elem, m_openings);
					}
					else {
						m_buildingElements.m_otherElemnts.push_back(bElem);
						m_buildingElements.m_otherElemnts.back()->update(elem, m_openings);
					}
				}
				else
					continue;
			}
		}

		for(std::shared_ptr<BuildingElement>& openingElement : m_buildingElements.m_openingElemnts) {
			openingElement->fillOpeningProperties(m_buildingElements.m_constructionElements, m_openings);
		}


		m_database.collectData(m_buildingElements);

		if(m_siteShape != nullptr) {
			std::shared_ptr<IfcSpatialStructureElement> se = std::dynamic_pointer_cast<IfcSpatialStructureElement>(m_siteShape->m_ifc_object_definition.lock());
			m_site.set(se, m_siteShape, m_buildingsShape);
			for(auto& building : m_site.m_buildings) {
				building->fetchStoreys(m_storeysShape);
				building->updateStoreys(m_elementEntitesShape, m_spaceEntitesShape, m_geometryConverter.getBuildingModel()->getUnitConverter(),
									   m_buildingElements, m_openings, m_useSpaceBoundaries);
			}
		}
		else {
			m_errorText = "No site";
			m_hasError = true;
			return false;
		}

		int failures = m_instances.collectComponentInstances(m_buildingElements, m_database, m_site);
		if(failures > 0) {
			m_errorText = "Not all surface can be matched to a component. failures: " + std::to_string(failures);
			m_hasError = true;
//			return false;
		}

		m_convertCompletedSuccessfully = true;
		return true;

	}
	catch (std::exception& e) {
		m_errorText = e.what();
		m_hasError = true;
		return false;
	}

	return false;
}

bool IFCReader::setVicusProject(VICUS::Project* project) {
	IBK_ASSERT(project != nullptr);

	std::map<int,int> idConversionMap;

	// add building structure
	int nextId = project->nextUnusedID();

	project->m_ifcFilePath = m_filename;

	for(auto& building : m_site.m_buildings) {
		project->m_buildings.emplace_back(building->getVicusObject(idConversionMap, nextId));
	}

	// add databases
	m_database.addToVicusProject(project, idConversionMap);

	// add component instances
	m_instances.addToVicusProject(project, idConversionMap);

	return true;
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

	// other files

	doc.SaveFile( filename.c_str() );
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
	ObjectTypes	m_typeRelatedElement;

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

	int sbCount = spaceBoundaries.size();
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
	result << tr("Statistic:");
	result << tr("%1 buildings.").arg(m_site.m_buildings.size());
	for(const auto& building : m_site.m_buildings) {
		result << tr("Building %1 with %2 storeys.").arg(QString::fromStdString(building->m_name))
				.arg(building->storeys().size());
		for(const auto& storey : building->storeys()) {
			result << tr("\tStorey %1 with %2 spaces.").arg(QString::fromStdString(storey->m_name))
					.arg(storey->spaces().size());
			for(const auto& space : storey->spaces()) {
				result << tr("\t\tSpace %1 with %2 space boundaries and %3 surfaces.").arg(QString::fromStdString(space->m_name+" - "+space->m_longName))
						.arg(space->spaceBoundaries().size()).arg(space->surfaces().size());
				for(const auto& surf : space->surfaces()) {
					if(surf.isMissing()) {
						result << tr("\t\t\tSurface %1 with missing connection.").arg(QString::fromStdString(surf.name()));
					}
					else if(surf.isVirtual()) {
						result << tr("\t\t\tSurface %1 is virtual.").arg(QString::fromStdString(surf.name()));
					}
					else {
						result << tr("\t\t\tSurface %1 with %2 subsurfaces.").arg(QString::fromStdString(surf.name()))
								.arg(surf.subSurfaces().size());
					}
				}
			}
		}
	}
	result << "\nDatabases\n";
	result << tr("\t%1 materials").arg(m_database.m_materials.size());
	for(const auto& mat : m_database.m_materials) {
		result << tr("\t\t%1 - id %2").arg(QString::fromStdString(mat.second.m_name)).arg(mat.first);
	}
	result << tr("\t%1 constructions").arg(m_database.m_constructions.size());
	for(const auto& con : m_database.m_constructions) {
		result << tr("\t\tConstruction id %1 with %2 layers").arg(con.first).arg(con.second.m_layers.size());
	}
	result << tr("\t%1 windows").arg(m_database.m_windows.size());
	for(const auto& win : m_database.m_windows) {
		result << tr("\t\tWindow %1 id %2").arg(QString::fromStdString(win.second.m_name)).arg(win.first);
	}
	result << tr("\t%1 windows").arg(m_database.m_windowGlazings.size());
	for(const auto& wgl : m_database.m_windowGlazings) {
		result << tr("\t\tWindow %1 id %2").arg(QString::fromStdString(wgl.second.m_name)).arg(wgl.first);
	}

	result << QString() << tr("Space boundary list") << QString();

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
		text += "\tconnected with: " + sbEval.m_nameRelatedElement;
		text += "\tcontained in: " + sbEval.m_nameRelatedSpace;
		result << text;
	}

	return result;
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

bool IFCReader::typeByGuid(const std::string& guid, std::pair<ObjectTypes,std::shared_ptr<ProductShapeData>>& res) {
	for(const auto& elemType : m_elementEntitesShape) {
		for(const auto& elem : elemType.second) {
			if(guid == ws2s(elem->m_entity_guid)) {
				res = std::pair<ObjectTypes,std::shared_ptr<ProductShapeData>>(elemType.first,elem);
				return true;
			}
		}
	}
	return false;
}


} // end namespace
