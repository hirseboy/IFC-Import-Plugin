#include "IFCC_Space.h"

#include <ifcpp/IFC4/include/IfcRelSpaceBoundary.h>
#include <ifcpp/IFC4/include/IfcLengthMeasure.h>

#include <numeric>

#include <IBK_math.h>

#include "IFCC_Helper.h"

namespace IFCC {

Space::Space(int id) :
	EntityBase(id)
{
}

bool Space::set(std::shared_ptr<IfcSpace> ifcSpace) {
	if(!EntityBase::set(dynamic_pointer_cast<IfcRoot>(ifcSpace)))
		return false;

	m_longName = label2s(ifcSpace->m_LongName);
	if(ifcSpace->m_PredefinedType != nullptr)
		m_spaceType = ifcSpace->m_PredefinedType->m_enum;

	// look for space boundaries from IFC
	for( const auto& bound : ifcSpace->m_BoundedBy_inverse) {
		auto boundP = bound.lock();
		SpaceBoundary sb(GUID_maker::instance().guid());
		bool res = sb.setFromIFC(boundP);
		if(res) {
			m_spaceBoundaries.push_back(sb);
		}
	}
	return true;
}

void Space::update(std::shared_ptr<ProductShapeData> productShape) {
	transform(productShape);
	fetchGeometry(productShape);
}


void Space::transform(std::shared_ptr<ProductShapeData> productShape) {
	if(productShape == nullptr)
		return;

	m_transformMatrix = productShape->getTransform();
	if(m_transformMatrix != carve::math::Matrix::IDENT()) {
		productShape->applyTransformToProduct(m_transformMatrix);
	}
}

void Space::fetchGeometry(std::shared_ptr<ProductShapeData> productShape) {
	if(productShape == nullptr)
		return;

	int repCount = productShape->m_vec_representations.size();
	std::shared_ptr<RepresentationData> currentRep;
	for(int repi = 0; repi<repCount; ++repi) {
		currentRep = productShape->m_vec_representations[repi];
		if(currentRep->m_representation_identifier == L"Body")
			break;
	}

	meshVector_t meshSetClosedFinal;
	meshVector_t meshSetOpenFinal;
	if(repCount > 0) {
		int itemDataCount = currentRep->m_vec_item_data.size();
		if(itemDataCount > 0) {
			meshSetClosedFinal = currentRep->m_vec_item_data.front()->m_meshsets;
			meshSetOpenFinal = currentRep->m_vec_item_data.front()->m_meshsets_open;
		}
	}

	meshVector_t& currentMeshSets =  meshSetClosedFinal.empty() ? meshSetOpenFinal : meshSetClosedFinal;
	simplifyMesh(currentMeshSets, false);

	polyVector_t polyvectClosedFinal;
	if(!meshSetClosedFinal.empty()) {
		int msCount = meshSetClosedFinal.size();
		for(int i=0; i<msCount; ++i) {
			polyvectClosedFinal.push_back(std::vector<std::vector<std::vector<IBKMK::Vector3D>>>());
			const carve::mesh::MeshSet<3>& currMeshSet = *meshSetClosedFinal[i];
			convert(currMeshSet, polyvectClosedFinal.back());
			// get surfaces
			for(size_t mi=0; mi<currMeshSet.meshes.size(); ++mi) {
				for(size_t fi =0; fi<currMeshSet.meshes[mi]->faces.size(); ++fi) {
					if(currMeshSet.meshes[mi]->faces[fi] != nullptr)
						m_surfacesOrg.emplace_back(Surface(currMeshSet.meshes[mi]->faces[fi]));
				}
			}
		}
	}
	polyVector_t polyvectOpenFinal;
	if(!meshSetOpenFinal.empty()) {
		int msCount = meshSetOpenFinal.size();
		for(int i=0; i<msCount; ++i) {
			polyvectOpenFinal.push_back(std::vector<std::vector<std::vector<IBKMK::Vector3D>>>());
			const carve::mesh::MeshSet<3>& currMeshSet = *meshSetOpenFinal[i];
			convert(currMeshSet, polyvectOpenFinal.back());
			// get surfaces
			for(size_t mi=0; mi<currMeshSet.meshes.size(); ++mi) {
				for(size_t fi =0; fi<currMeshSet.meshes[mi]->faces.size(); ++fi) {
					if(currMeshSet.meshes[mi]->faces[fi] != nullptr)
						m_surfacesOrg.emplace_back(Surface(currMeshSet.meshes[mi]->faces[fi]));
				}
			}
		}
	}
}

static void divideSurface(const Surface::IntersectionResult& intRes, std::vector<Surface>& spaceSurfaces, int ssIndex, std::vector<Surface>& subsurfaces) {
	// construction element covers complete space surface
	if(intRes.m_diffBaseMinusClip.empty()) {
		spaceSurfaces.erase(spaceSurfaces.begin() + ssIndex);
	}
	// we have a rest of the space surface without matching construction element
	else {
		// we have only one resulting space surface - change original surface to it
		if(intRes.m_diffBaseMinusClip.size() == 1) {
			// we have no holes
			spaceSurfaces[ssIndex] = intRes.m_diffBaseMinusClip.front();
			// we have some holes - add these to subsurface list
			if(!intRes.m_holesBaseMinusClip.front().empty()) {
				for(const Surface& subsurf : intRes.m_holesBaseMinusClip.front())
					subsurfaces.push_back(subsurf);
			}
		}
		// we have mor than one resulting space surface - set first one to original and add the others
		else {
			spaceSurfaces[ssIndex] = intRes.m_diffBaseMinusClip.front();
			for(const Surface& subsurf : intRes.m_holesBaseMinusClip.front())
				subsurfaces.push_back(subsurf);
			for(size_t i=1; i<intRes.m_diffBaseMinusClip.size(); ++i) {
				spaceSurfaces.push_back(intRes.m_diffBaseMinusClip[i]);
				for(const Surface& subsurf : intRes.m_holesBaseMinusClip[i])
					subsurfaces.push_back(subsurf);
			}
		}
	}
}

/*! Struct contains result of the function findFirstSurfaceMatchIndex.
	It is used for matching a construction surface to a space surface.*/
struct MatchResult {
	/*! Default constructor creates a non valid object.*/
	MatchResult() :
		m_wallSurfaceIndex(-1),
		m_spaceSurfaceIndex(-1)
	{}

	/*! Standard constructor.*/
	MatchResult(int wallSurfaceIndex, int spaceSurfaceIndex) :
		m_wallSurfaceIndex(wallSurfaceIndex),
		m_spaceSurfaceIndex(spaceSurfaceIndex)
	{}

	/*! Return true if both indices are valid (greater than -1).*/
	bool isValid() const {
		return m_wallSurfaceIndex > -1 && m_spaceSurfaceIndex > -1;
	}

	int m_wallSurfaceIndex;		///< Index of the construction surface
	int m_spaceSurfaceIndex;	///< Index of the space surface
};

static MatchResult findFirstSurfaceMatchIndex(const std::vector<Surface>& wallSurfaces, const std::vector<Surface>& spaceSurfaces, double minDist) {
	const double EPS = 1e-3;
	for(size_t wi=0; wi<wallSurfaces.size(); ++wi) {
		for(size_t si=0; si<spaceSurfaces.size(); ++si) {
			double dist = spaceSurfaces[si].distanceToParallelPlane(wallSurfaces[wi]);
			if(dist < minDist * (1+EPS)) {
				if(wallSurfaces[wi].isIntersected(spaceSurfaces[si]))
					return MatchResult(wi,si);
			}
		}
	}
	return MatchResult();
}


std::vector<SpaceBoundary> Space::createSpaceBoundaries(const std::vector<BuildingElement>& elements) {
	std::vector<Surface> surfaces(m_surfacesOrg);
	std::vector<SpaceBoundary> spaceBoundaries;
	for(const auto& construction : elements) {
		double dist = construction.thickness();
		double maxConstructionDist = 0;
		if(construction.isSubSurfaceComponent() && !construction.m_openingProperties.m_constructionThicknesses.empty()) {
			maxConstructionDist = *std::max_element(construction.m_openingProperties.m_constructionThicknesses.begin(),
												   construction.m_openingProperties.m_constructionThicknesses.end());
		}
		if(dist < 1.0e-4) {
			if(maxConstructionDist > 1.0e-4)
				dist = maxConstructionDist;
			else
				dist = 1.0e-4;
		}
		int wId = -1;
		if(construction.m_name == "Dach-2" && m_name == "7") {
			wId = construction.m_id;
		}

		MatchResult indices = findFirstSurfaceMatchIndex(construction.surfaces(), surfaces, dist);
		if(indices.isValid()) {
			int loopCount = 0;
			do {
				++loopCount;
				Surface::IntersectionResult intersectionResult = surfaces[indices.m_spaceSurfaceIndex].intersect2(construction.surfaces()[indices.m_wallSurfaceIndex]);
				for(const Surface& surf : intersectionResult.m_intersections) {
					SpaceBoundary sb(GUID_maker::instance().guid());
					std::string name = m_name + ":" + construction.m_name + " - " +
									   std::to_string(indices.m_spaceSurfaceIndex) + " : " + std::to_string(indices.m_wallSurfaceIndex);
					if(sb.setFromBuildingElement(name, construction)) {
						sb.m_elementEntityId = construction.m_id;
						sb.fetchGeometryFromBuildingElement(surf);
						spaceBoundaries.push_back(sb);
					}
				}
				std::vector<Surface> subsurfaces;
				divideSurface(intersectionResult, surfaces, indices.m_spaceSurfaceIndex, subsurfaces);

				indices = findFirstSurfaceMatchIndex(construction.surfaces(), surfaces, dist);

				if(loopCount > 100) {
					break;
				}
			} while(indices.isValid());
		}
	}

	return spaceBoundaries;
}

/*! Search for space boundaries by checking for */
static Space::OpeningConstructionMatching findBestSurfaceOpeningMatchIndex(const BuildingElement& openingConstruction,
														 const std::vector<Opening>& openings, double minDist) {
	const auto uidBegin = openingConstruction.m_usedFromOpenings.begin();
	const auto uidEnd = openingConstruction.m_usedFromOpenings.end();
	Space::OpeningConstructionMatching result;
	double lastDistance = 1e30;
	for(size_t wi=0; wi<openingConstruction.surfaces().size(); ++wi) {
		for(size_t oi=0; oi<openings.size(); ++oi) {
			if(std::find(uidBegin, uidEnd, openings[oi].m_id) != uidEnd) {
				double constrThickness = openingConstruction.thickness();
				if(constrThickness > minDist) {
					minDist = constrThickness;
				}

				for(size_t sbsi=0; sbsi<openings[oi].surfaces().size(); ++sbsi) {
					const Surface& currentSpaceSurf = openings[oi].surfaces()[sbsi];
					double dist = currentSpaceSurf.distanceToParallelPlane(openingConstruction.surfaces()[wi]);
					if(dist < minDist) {
						if(currentSpaceSurf.isIntersected(openingConstruction.surfaces()[wi])) {
							if(!result.isValid() || dist < lastDistance) {
								result = Space::OpeningConstructionMatching(wi,oi,sbsi);
								lastDistance = dist;
							}
						}
					}
				}
			}
		}
	}
	return result;
}

void Space::createSpaceBoundariesForOpeningsFromOpenings(std::vector<SpaceBoundary>& spaceBoundaries,
											 const std::vector<BuildingElement>& openingElements,
											 const std::vector<Opening>& openings) {
	if(openings.empty())
		return;

	for(const auto& construction : openingElements) {
		if(!construction.isSubSurfaceComponent())
			continue;

		double maxConstructionThickness = 0;
		if(construction.isSubSurfaceComponent() && !construction.m_openingProperties.m_constructionThicknesses.empty()) {
			maxConstructionThickness = *std::max_element(construction.m_openingProperties.m_constructionThicknesses.begin(),
												   construction.m_openingProperties.m_constructionThicknesses.end());
			Space::OpeningConstructionMatching match = findBestSurfaceOpeningMatchIndex(construction, openings, maxConstructionThickness);
			if(match.isValid()) {
				const Surface& openingSurface = openings[match.m_openingIndex].surfaces()[match.m_openingSurfaceIndex];
				const Surface& constrSurface = construction.surfaces()[match.m_constructionSurfaceIndex];
				Surface intersectionResult = openingSurface.intersect(constrSurface);
				if(intersectionResult.isValid()) {
					SpaceBoundary sb(GUID_maker::instance().guid());
					std::string name = m_name + ":" + construction.m_name + " - " +
									   std::to_string(match.m_openingIndex) + " : " +
									   std::to_string(match.m_openingSurfaceIndex) + " : " +
									   std::to_string(match.m_constructionSurfaceIndex);
					if(sb.setFromBuildingElement(name, construction)) {
						sb.m_elementEntityId = construction.m_id;
						sb.fetchGeometryFromBuildingElement(intersectionResult);
						spaceBoundaries.push_back(sb);
					}
				}
			}
		}
	}
}

bool Space::evaluateSpaceBoundaryTypes(const objectShapeTypeVector_t& shapes,
								 shared_ptr<UnitConverter>& unit_converter,
								 const std::vector<BuildingElement>& constructionElements,
								 const std::vector<BuildingElement>& openingElements) {
	if(m_spaceBoundaries.empty())
		return false;

	bool foundOne = false;

	for(auto& sb : m_spaceBoundaries) {
		int type = -1;
		for(const auto& elemType : shapes) {
			for(const auto& elem : elemType.second) {
				if(sb.guidRelatedElement() == ws2s(elem->m_entity_guid)) {
					type = elemType.first;
					break;
				}
			}
			if(type > -1)
				break;
		}
		int id = -1;
		for(const auto& construction : constructionElements) {
			if(construction.m_guid == sb.guidRelatedElement()) {
				id = construction.m_id;
			}
		}
		if(id == -1) {
			for(const auto& opening : openingElements) {
				if(opening.m_guid == sb.guidRelatedElement()) {
					id = opening.m_id;
				}
			}
		}
		if(type > -1) {
			sb.setRelatingElementType(static_cast<ObjectTypes>(type));
			sb.m_elementEntityId = id;
			sb.fetchGeometryFromIFC(unit_converter, m_transformMatrix);
			foundOne = true;
		}
		else {
			std::string name = sb.nameRelatedElement();
			m_notes = "Element type not found: " + name;
		}
	}

	if(!foundOne) {
		m_spaceBoundaryErrors = "Cannot find at least one connected space boundary";
	}
	return foundOne;
}

bool Space::evaluateSpaceBoundaries(const std::vector<BuildingElement>& constructionElements,
									const std::vector<BuildingElement>& openingElements,
									const std::vector<Opening>& openings) {

	std::vector<SpaceBoundary> newSpaceBoundaries = createSpaceBoundaries(constructionElements);
	createSpaceBoundariesForOpeningsFromOpenings(newSpaceBoundaries, openingElements, openings);

	m_spaceBoundaries = newSpaceBoundaries;

	if(m_spaceBoundaries.empty()) {
		m_spaceBoundaryErrors = "Cannot evaluate any space boundary for this space";
		return false;
	}
	return true;
}

bool Space::updateSpaceBoundaries(const objectShapeTypeVector_t& shapes,
								  shared_ptr<UnitConverter>& unit_converter,
								  const std::vector<BuildingElement>& constructionElements,
								  const std::vector<BuildingElement>& openingElements,
								  const std::vector<Opening>& openings) {
	bool success;
	// update existing space boundaries from IFC
//	m_spaceBoundaries.clear();
	if(!m_spaceBoundaries.empty()) {
		success = evaluateSpaceBoundaryTypes(shapes, unit_converter, constructionElements, openingElements);
	}
	// try to evaluate space boundaries from building element entities
	else {
		success = evaluateSpaceBoundaries(constructionElements, openingElements, openings);
	}

	if(success)
		updateSurfaces(constructionElements);

	return success;
}

void Space::updateSurfaces(const std::vector<BuildingElement>& elems) {

	m_surfaces.clear();
	std::vector<Surface> subSurfaces;

	if(!m_spaceBoundaries.empty()) {
		for( const auto& sb : m_spaceBoundaries) {
			if(sb.isConstructionElement()) {
				const std::vector<Surface>& tempSurfVect = sb.surfaces();
				m_surfaces.insert(m_surfaces.end(), tempSurfVect.begin(), tempSurfVect.end());
			}
			else if(sb.isOpeningElement()) {
				const std::vector<Surface>& tempSurfVect = sb.surfaces();
				subSurfaces.insert(subSurfaces.end(), tempSurfVect.begin(), tempSurfVect.end());
			}
			else {
				///< \todo handling other space boundary types
			}
		}
		std::vector<OpeningMatching> subSurfaceMatch;
		for(size_t i=0; i<m_surfaces.size(); ++i) {
			const Surface& curSurf = m_surfaces[i];
			int elementIndex = -1;
			for(int eli=0; eli<elems.size(); ++eli) {
				if(curSurf.elementId() == elems[eli].m_id) {
					elementIndex = eli;
					break;
				}
			}
			double thickness = 0;
			if(elementIndex>-1) {
				const BuildingElement& elem = elems[elementIndex];
				thickness = elem.thickness();
			}

			for(size_t j=0; j<subSurfaces.size(); ++j) {
				const Surface& sub = subSurfaces[j];

				// find subsurfaces in surfaces which have already cutted openings
				// in this case surface and subsurface must have same points
				std::vector<std::pair<size_t,size_t>> samepoints = m_surfaces[i].samePoints(sub);
				if(samepoints.size() > 2) {
					subSurfaceMatch.emplace_back(OpeningMatching(OMT_SamePoints, i,j));
				}
				else {
					bool parallel = m_surfaces[i].isParallelTo(sub);

					// check if we can find a subsurface which is parallel to the surface
					// then check if the distance of the subsurface to the surface is smaller than the thickness of the bulding element
					// if this is the case the window or door can be inside of the wall
					if(parallel && thickness > 0) {
						double dist = m_surfaces[i].distanceToParallelPlane(sub);
						// distance is smaller than thickness - check if we have an intersection
						if(dist < thickness) {
							if(m_surfaces[i].isIntersected(sub)) {
								subSurfaceMatch.emplace_back(OpeningMatching(OMT_WallThicknessIntersection, i,j));
							}
						}
					}
				}
			}
		}

		// first merge all subsurface in order to get complete original surface
		for( const auto& match : subSurfaceMatch) {
			if(match.m_type == OMT_SamePoints)
				m_surfaces[match.m_surfaceIndex].merge(subSurfaces[match.m_subSurfaceIndex]);
		}
		// now we can create the subsurfaces in 2D surface coordinates
		for( const auto& match : subSurfaceMatch) {
			m_surfaces[match.m_surfaceIndex].addSubSurface(subSurfaces[match.m_subSurfaceIndex]);
		}

	}
	else {
		///< \todo Error handling?
	}
}

const std::vector<Surface>& Space::surfaces() const {
	return m_surfaces;
}

const std::vector<SpaceBoundary>& Space::spaceBoundaries() const {
	return m_spaceBoundaries;
}


TiXmlElement * Space::writeXML(TiXmlElement * parent) const {
	if (m_id == -1)
		return nullptr;

	TiXmlElement * e = new TiXmlElement("Room");
	parent->LinkEndChild(e);

	e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_name.empty())
		e->SetAttribute("displayName", m_longName);
//	e->SetAttribute("visible", IBK::val2string<bool>(true));

	if(!m_surfaces.empty()) {
		TiXmlElement * child = new TiXmlElement("Surfaces");
		e->LinkEndChild(child);

		for( const Surface& surface : m_surfaces) {
			surface.writeXML(child);
		}
	}
	return e;
}


} // namespace IFCC
