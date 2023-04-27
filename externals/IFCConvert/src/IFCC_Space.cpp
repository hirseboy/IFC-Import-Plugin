#include "IFCC_Space.h"

#include <ifcpp/IFC4X3/include/IfcRelSpaceBoundary.h>
#include <ifcpp/IFC4X3/include/IfcLengthMeasure.h>

#include <numeric>

#include <IBK_math.h>

#include <IBKMK_3DCalculations.h>

#include <Carve/src/include/carve/carve.hpp>

#include "IFCC_MeshUtils.h"
#include "IFCC_Helper.h"

namespace IFCC {

Space::Space(int id) :
	EntityBase(id)
{
}

bool Space::set(std::shared_ptr<IFC4X3::IfcSpace> ifcSpace) {
	if(!EntityBase::set(dynamic_pointer_cast<IFC4X3::IfcRoot>(ifcSpace)))
		return false;

	m_longName = label2s(ifcSpace->m_LongName);
	if(ifcSpace->m_PredefinedType != nullptr)
		m_spaceType = ifcSpace->m_PredefinedType->m_enum;

	// look for space boundaries from IFC
	for( const auto& bound : ifcSpace->m_BoundedBy_inverse) {
		auto boundP = bound.lock();
		std::shared_ptr<SpaceBoundary> sb = std::shared_ptr<SpaceBoundary>(new SpaceBoundary(GUID_maker::instance().guid()));
		bool res = sb->setFromIFC(boundP);
		if(res) {
			m_spaceBoundaries.push_back(sb);
			m_spaceBoundaryGUIDs.push_back(sb->m_guid);
		}
	}
	return true;
}

void Space::update(std::shared_ptr<ProductShapeData> productShape, std::vector<ConvertError>& errors) {
	transform(productShape);
	fetchGeometry(productShape, errors);
}


void Space::transform(std::shared_ptr<ProductShapeData> productShape) {
	if(productShape == nullptr)
		return;

	m_transformMatrix = productShape->getTransform();
	if(m_transformMatrix != carve::math::Matrix::IDENT()) {
		productShape->applyTransformToProduct(m_transformMatrix, true, true);
	}
}

void Space::fetchGeometry(std::shared_ptr<ProductShapeData> productShape, std::vector<ConvertError>& errors) {
	if(productShape == nullptr)
		return;

	surfacesFromRepresentation(productShape, m_surfacesOrg, errors, OT_Space, m_id);

	m_meshSets = meshSetsFromBodyRepresentation(productShape);

	if(m_surfacesOrg.empty())
		return;
}


static void divideSurface(const Surface::IntersectionResult& intRes, std::vector<Surface>& spaceSurfaces, int ssIndex, std::vector<Surface>& subsurfaces) {
	// we don't have any intersections
	if(intRes.m_intersections.empty())
		return;

	// construction element covers complete space surface
	if(intRes.m_diffBaseMinusClip.empty()) {
		spaceSurfaces.erase(spaceSurfaces.begin() + ssIndex);
		return;
	}

	// we have a rest of the space surface without matching construction element
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
	// we have more than one resulting space surface - set first one to original and add the others
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


std::vector<std::shared_ptr<SpaceBoundary>> Space::createSpaceBoundaries(const BuildingElementsCollector& buildingElements, std::vector<ConvertError>& errors) {
	std::vector<Surface> surfaces(m_surfacesOrg);
	std::vector<std::shared_ptr<SpaceBoundary>> spaceBoundaries;
	std::vector<std::shared_ptr<BuildingElement>> constructionElements = buildingElements.allConstructionElements();

	for(const auto& construction : constructionElements) {
		double dist = construction->thickness();
		double maxConstructionDist = 0;
		if(construction->isSubSurfaceComponent() && !construction->m_openingProperties.m_constructionThicknesses.empty()) {
			maxConstructionDist = *std::max_element(construction->m_openingProperties.m_constructionThicknesses.begin(),
												   construction->m_openingProperties.m_constructionThicknesses.end());
		}
		if(dist < 1.0e-4) {
			if(maxConstructionDist > 1.0e-4)
				dist = maxConstructionDist;
			else
				dist = 1.0e-4;
		}

		MatchResult indices = findFirstSurfaceMatchIndex(construction->surfaces(), surfaces, dist);
		if(indices.isValid()) {
			int loopCount = 0;
			do {
				const Surface& currSpaceSurf = surfaces[indices.m_spaceSurfaceIndex];
				const Surface& currConstSurf = construction->surfaces()[indices.m_wallSurfaceIndex];
				++loopCount;
				Surface::IntersectionResult intersectionResult = currSpaceSurf.intersect2(currConstSurf);
				// no intersections found
				if(intersectionResult.m_intersections.empty())
					break;

				const Surface& firstISurf = intersectionResult.m_intersections.front();
				if(intersectionResult.m_intersections.size() == 1 && IBK::nearly_equal<2>(firstISurf.area(),currSpaceSurf.area())) {
					int id = GUID_maker::instance().guid();
					std::shared_ptr<SpaceBoundary> sb = std::shared_ptr<SpaceBoundary>(new SpaceBoundary(id));
					std::string name = m_name + ":" + construction->m_name + " - " +
							std::to_string(indices.m_spaceSurfaceIndex) + " : " + std::to_string(indices.m_wallSurfaceIndex);
					if(sb->setFromBuildingElement(name, construction, *this)) {
						sb->m_elementEntityId = construction->m_id;
						sb->fetchGeometryFromBuildingElement(currSpaceSurf);
						spaceBoundaries.push_back(sb);
						surfaces.erase(surfaces.begin() + indices.m_spaceSurfaceIndex);
					}
				}
				else {
					for(const Surface& surf : intersectionResult.m_intersections) {
						int id = GUID_maker::instance().guid();
						std::shared_ptr<SpaceBoundary> sb = std::shared_ptr<SpaceBoundary>(new SpaceBoundary(id));
						std::string name = m_name + ":" + construction->m_name + " - " +
								std::to_string(indices.m_spaceSurfaceIndex) + " : " + std::to_string(indices.m_wallSurfaceIndex);
						if(sb->setFromBuildingElement(name, construction, *this)) {
							sb->m_elementEntityId = construction->m_id;
							sb->fetchGeometryFromBuildingElement(surf);
							spaceBoundaries.push_back(sb);
						}
					}
					std::vector<Surface> subsurfaces;
					divideSurface(intersectionResult, surfaces, indices.m_spaceSurfaceIndex, subsurfaces);
					if(!subsurfaces.empty()) {
						// what should we do with the holes?
						errors.push_back(ConvertError{OT_Space, m_id, "intersection from space surface and building element surface has holes"});
					}
				}
				indices = findFirstSurfaceMatchIndex(construction->surfaces(), surfaces, dist);

				if(loopCount > 100) {
					break;
				}
			} while(indices.isValid());
		}
	}

	for(const Surface& surf : surfaces) {
		if(surf.area() < 0.01)
			continue;

		std::shared_ptr<SpaceBoundary> sb = std::shared_ptr<SpaceBoundary>(new SpaceBoundary(GUID_maker::instance().guid()));
		std::string name = "Missing";
		sb->setForMissingElement(name, *this, false);
		sb->fetchGeometryFromBuildingElement(surf);
		spaceBoundaries.push_back(sb);
	}

	return spaceBoundaries;
}

/*! Search for space boundaries by checking for */
static Space::OpeningConstructionMatching findBestSurfaceOpeningMatchIndex(const std::shared_ptr<BuildingElement>& openingConstruction,
														 const std::vector<Opening>& openings, double minDist) {
	// opening construction not used from any opening
	if(openingConstruction->m_usedFromOpenings.empty())
		return Space::OpeningConstructionMatching();

	// opening construction used from only one opening
	if(openingConstruction->m_usedFromOpenings.size() == 1) {
		int usedOpeningId = openingConstruction->m_usedFromOpenings.front();
		int usedOpeningIndex = -1;
		for(size_t oi=0; oi<openings.size(); ++oi) {
			if(openings[oi].m_id == usedOpeningId) {
				usedOpeningIndex = oi;
				break;
			}
		}
		if(usedOpeningIndex == -1)
			return Space::OpeningConstructionMatching();

		const Opening& usedOpening = openings[usedOpeningIndex];
		double lastDistance = 1e30;
		double constrThickness = openingConstruction->thickness();
		if(constrThickness > minDist) {
			minDist = constrThickness;
		}
		Space::OpeningConstructionMatching result;
		for(size_t wi=0; wi<openingConstruction->surfaces().size(); ++wi) {
			const Surface& currentConstructionSurf = openingConstruction->surfaces()[wi];
			for(size_t sbsi=0; sbsi<usedOpening.surfaces().size(); ++sbsi) {
				const Surface& currentOpeningSurf = usedOpening.surfaces()[sbsi];
				double dist = currentOpeningSurf.distanceToParallelPlane(currentConstructionSurf);
				if(dist < minDist) {
					if(currentOpeningSurf.isIntersected(currentConstructionSurf)) {
						if(!result.isValid() || dist < lastDistance) {
							result = Space::OpeningConstructionMatching(wi,usedOpeningIndex,sbsi);
							lastDistance = dist;
						}
					}
				}

			}
		}
		return result;
	}

	// more than one opening use this construction
	const auto uidBegin = openingConstruction->m_usedFromOpenings.begin();
	const auto uidEnd = openingConstruction->m_usedFromOpenings.end();
	Space::OpeningConstructionMatching result;
	double lastDistance = 1e30;
	for(size_t wi=0; wi<openingConstruction->surfaces().size(); ++wi) {
		for(size_t oi=0; oi<openings.size(); ++oi) {
			if(std::find(uidBegin, uidEnd, openings[oi].m_id) != uidEnd) {
				double constrThickness = openingConstruction->thickness();
				if(constrThickness > minDist) {
					minDist = constrThickness;
				}

				for(size_t sbsi=0; sbsi<openings[oi].surfaces().size(); ++sbsi) {
					const Surface& currentSpaceSurf = openings[oi].surfaces()[sbsi];
					double dist = currentSpaceSurf.distanceToParallelPlane(openingConstruction->surfaces()[wi]);
					if(dist < minDist) {
						if(currentSpaceSurf.isIntersected(openingConstruction->surfaces()[wi])) {
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


void Space::createSpaceBoundariesForOpeningsFromSpaceBoundaries(std::vector<std::shared_ptr<SpaceBoundary>>& spaceBoundaries,
																const BuildingElementsCollector& buildingElements,
																std::vector<Opening>& openings, std::vector<ConvertError>& errors) {
	if(openings.empty())
		return;

	std::vector<std::shared_ptr<SpaceBoundary>> openingSpaceBoundaries;

	for(const auto& spaceBoundary : spaceBoundaries) {
		// openings can only be part of a construction space boundary
		if(!spaceBoundary->isConstructionElement())
			continue;

		std::string elemGUID = spaceBoundary->guidRelatedElement();
		const std::shared_ptr<BuildingElement> elem = buildingElements.fromGUID(elemGUID);
		// only go further if space boundary is connected to a existing construction element
		if(elem.get() == nullptr)
			continue;

		// only go further if the construction contains openings
		if(elem->m_containedOpenings.empty())
			continue;

		double thickness = elem->thickness();
		if(thickness == 0)
			thickness = 0.5;

		// collect all contained openings
		std::vector<size_t> containedOpeningsIndices;
		for(int opid : elem->m_containedOpenings) {
			auto fitOp = std::find_if(openings.begin(), openings.end(),
									  [opid](const auto& op) { return op.m_id == opid; });
			if(fitOp != openings.end())
				containedOpeningsIndices.push_back(std::distance(openings.begin(), fitOp));
		}
		// only go further if we have some contained openings
		if(containedOpeningsIndices.empty())
			continue;

		for(size_t coi=0; coi<containedOpeningsIndices.size(); ++coi) {
			int opIndex = containedOpeningsIndices[coi];
			Opening& currOp = openings[opIndex];
			if(currOp.hasSpaceBoundary())
				continue;

			std::shared_ptr<BuildingElement> openingElem;
			// has no construction - its a breakout
			if(currOp.openingElementIds().size() == 1) {
				int id = currOp.openingElementIds().front();
				openingElem = buildingElements.fromID(id);
			}
			// error - an opening should not have more than one opening construction
			else {
				errors.push_back(ConvertError{OT_Space, m_id, "An opening is connected to more than one building element"});
			}

			for(size_t cosi=0; cosi<currOp.surfaces().size(); ++cosi) {
				const Surface& currentOpeningSurf = currOp.surfaces()[cosi];
				double dist = currentOpeningSurf.distanceToParallelPlane(spaceBoundary->surface());
				if(dist <= thickness) {
					if(currentOpeningSurf.isIntersected(spaceBoundary->surface())) {
						Surface intersectionResult = spaceBoundary->surface().intersect(currentOpeningSurf);
						if(intersectionResult.isValid()) {
							std::shared_ptr<SpaceBoundary> sb = std::shared_ptr<SpaceBoundary>(new SpaceBoundary(GUID_maker::instance().guid()));
							if(openingElem) {
								std::string name = m_longName + ":" + spaceBoundary->m_name+ ":" + openingElem->m_name + " - O" +
												   std::to_string(opIndex) + " : OS" +
												   std::to_string(cosi);
								if(sb->setFromBuildingElement(name, openingElem, *this)) {
									sb->m_elementEntityId = openingElem->m_id;
									sb->m_openingId = currOp.m_id;
									sb->fetchGeometryFromBuildingElement(intersectionResult);
									openingSpaceBoundaries.push_back(sb);
									currOp.setSpaceBoundary(sb);
								}
							}
							else {
								std::string name = m_longName + spaceBoundary->m_name+ ":" + ": breakout - O" +
												   std::to_string(opIndex) + " : OS" +
												   std::to_string(cosi);
								sb->setForVirtualElement(name, *this, true);
								sb->m_openingId = currOp.m_id;
								sb->fetchGeometryFromBuildingElement(intersectionResult);
								openingSpaceBoundaries.push_back(sb);
								currOp.setSpaceBoundary(sb);
							}
							spaceBoundary->addContainedOpeningSpaceBoundaries(sb);
							// we found a connection therfore we can end searching
							break;
						}
					}
				}
			}
		}
	}

	if(!openingSpaceBoundaries.empty()) {
		spaceBoundaries.insert(spaceBoundaries.end(), openingSpaceBoundaries.begin(), openingSpaceBoundaries.end());
	}
}

void Space::createSpaceBoundariesForOpeningsFromOpenings(std::vector<std::shared_ptr<SpaceBoundary>>& spaceBoundaries,
											 const BuildingElementsCollector& buildingElements,
											 const std::vector<Opening>& openings, std::vector<ConvertError>& errors) {
	if(openings.empty())
		return;

	if(m_longName == "Buero") {
		int t1 = 0;
	}
	for(const auto& construction : buildingElements.m_openingElements) {
		if(!construction->isSubSurfaceComponent())
			continue;

		double maxConstructionThickness = 0;
		if(construction->isSubSurfaceComponent() && !construction->m_openingProperties.m_constructionThicknesses.empty()) {
			maxConstructionThickness = *std::max_element(construction->m_openingProperties.m_constructionThicknesses.begin(),
												   construction->m_openingProperties.m_constructionThicknesses.end());
			Space::OpeningConstructionMatching match = findBestSurfaceOpeningMatchIndex(construction, openings, maxConstructionThickness);
			if(match.isValid()) {
				const Surface& openingSurface = openings[match.m_openingIndex].surfaces()[match.m_openingSurfaceIndex];
				const Surface& constrSurface = construction->surfaces()[match.m_constructionSurfaceIndex];
				Surface intersectionResult = openingSurface.intersect(constrSurface);
				if(intersectionResult.isValid()) {
					std::shared_ptr<SpaceBoundary> sb = std::shared_ptr<SpaceBoundary>(new SpaceBoundary(GUID_maker::instance().guid()));
					std::string name = m_name + ":" + construction->m_name + " - " +
									   std::to_string(match.m_openingIndex) + " : " +
									   std::to_string(match.m_openingSurfaceIndex) + " : " +
									   std::to_string(match.m_constructionSurfaceIndex);
					if(sb->setFromBuildingElement(name, construction, *this)) {
						sb->m_elementEntityId = construction->m_id;
						sb->fetchGeometryFromBuildingElement(intersectionResult);
						spaceBoundaries.push_back(sb);
					}
				}
			}
		}
		// construction don't have a thickness
		else {
			errors.push_back(ConvertError{OT_Space, m_id, "Check for openings: opening without thickness found"});
		}
	} // loop over all opening constructions

}

static int typeFromElementShape(const shared_ptr<SpaceBoundary>& sb, const objectShapeTypeVector_t& shapes) {
	for(const auto& elemType : shapes) {
		for(const auto& elem : elemType.second) {
			if(sb->guidRelatedElement() == elem->m_entity_guid) {
				return elemType.first;
			}
		}
	}
	return -1;
}

int constructionId(const shared_ptr<SpaceBoundary>& sb, const BuildingElementsCollector& buildingElements) {
	int id = -1;
	std::vector<std::shared_ptr<BuildingElement>> constructionElements = buildingElements.allConstructionElements();
	for(const auto& construction : constructionElements) {
		if(construction->m_guid == sb->guidRelatedElement()) {
			id = construction->m_id;
		}
	}
	if(id == -1) {
		for(const auto& opening : buildingElements.m_openingElements) {
			if(opening->m_guid == sb->guidRelatedElement()) {
				id = opening->m_id;
			}
		}
	}
	return id;
}

void Space::evaluateSpaceBoundaryTypes(const objectShapeTypeVector_t& shapes,
								 const BuildingElementsCollector& buildingElements) {
	for(size_t sbI=0; sbI<m_spaceBoundaries.size(); ++sbI) {
		auto& sb = m_spaceBoundaries[sbI];

		int type = typeFromElementShape(sb, shapes);

		int id = constructionId(sb, buildingElements);

		if(type > -1) {
			sb->setRelatingElementType(static_cast<BuildingElementTypes>(type));
			sb->m_elementEntityId = id;
		}
		else {
			if(sb->isVirtual()) {
				sb->setRelatingElementType(static_cast<BuildingElementTypes>(BET_None));
			}
			else {
				sb->setRelatingElementType(static_cast<BuildingElementTypes>(BET_None));
			}
			sb->m_elementEntityId = -1;
		}

	} // end loop over space boundaries
}

bool Space::evaluateSpaceBoundaryGeometry(shared_ptr<UnitConverter>& unit_converter,
									   std::vector<ConvertError>& errors) {
	if(m_spaceBoundaries.empty())
		return false;


	bool foundOne = false;
	std::vector<int> wrongSBIds;
	for(size_t sbI=0; sbI<m_spaceBoundaries.size(); ++sbI) {
		auto& sb = m_spaceBoundaries[sbI];

		bool res = sb->fetchGeometryFromIFC(unit_converter, m_transformMatrix, errors);
		if(res) {
			foundOne = true;
		}
		else {
			wrongSBIds.push_back(sb->m_id);
		}

	} // end loop over space boundaries

	if(!foundOne) {
		errors.push_back({OT_Space, m_id, "Cannot find at least one connected space boundary"});
	}

	// remove all space boundaries from the list which have non valid surfaces
	if(!wrongSBIds.empty()) {
		for(auto it = m_spaceBoundaries.begin(); it!=m_spaceBoundaries.end();) {
			if(std::find_if(wrongSBIds.begin(), wrongSBIds.end(), [it](int id) -> bool { return id == (*it)->m_id;}) != wrongSBIds.end()) {
				it = m_spaceBoundaries.erase(it);
			}
			else {
				++it;
			}
		}
	}
	return foundOne;
}

bool Space::evaluateSpaceBoundaryFromIFC(const objectShapeTypeVector_t& shapes,
										 const BuildingElementsCollector& buildingElements,
										 shared_ptr<UnitConverter>& unit_converter,
										 std::vector<ConvertError>& errors) {
	// get space boundary types and set element id connections
	evaluateSpaceBoundaryTypes(shapes, buildingElements);

	// convert geometry and create surfaces
	bool res = evaluateSpaceBoundaryGeometry(unit_converter, errors);
	if(!res)
		return false;

	std::vector<std::shared_ptr<SpaceBoundary>> splittedSBs;
	for(auto sb : m_spaceBoundaries) {
		std::vector<std::shared_ptr<SpaceBoundary>> ssbs = sb->splitBySurfaces();
		if(!ssbs.empty()) {
			splittedSBs.insert(splittedSBs.end(), ssbs.begin(), ssbs.end());
		}
	}
	if(!splittedSBs.empty()) {
		m_spaceBoundaries.insert(m_spaceBoundaries.begin(), splittedSBs.begin(), splittedSBs.end());
	}

	// create two temporary vectors for construction space boundaries and opening space boundaries
	std::vector<std::shared_ptr<SpaceBoundary>> constructionSBs;
	std::vector<std::shared_ptr<SpaceBoundary>> openingSBs;
	for(auto sb : m_spaceBoundaries) {
		if(sb->isConstructionElement())
			constructionSBs.push_back(sb);
		if(sb->isOpeningElement())
			openingSBs.push_back(sb);
	}

	if(openingSBs.empty())
		return true;

	// try to find out which opening sb is related to which construction sb
	std::vector<int> addedOpeningIds;
	for(auto openingSB : openingSBs) {
		// check if the opening sb is already added
		int id = openingSB->m_id;
		if(std::find_if(addedOpeningIds.begin(),addedOpeningIds.end(), [id](int addedId) -> bool { return id == addedId; }) != addedOpeningIds.end())
			continue;

		const Surface& opSurf = openingSB->surface();
		int notParallel = 0;
		int wrongDistance = 0;
		int notIntersected = 0;
		for(auto constrSB : constructionSBs ) {
			const Surface& constrSurf = constrSB->surface();

			// fetch thickness of construction element if exist
			int elementId = constrSB->m_elementEntityId;
			std::shared_ptr<BuildingElement> element = buildingElements.fromID(elementId);
			double searchDist = 0.001;
			if(element)
				searchDist = element->thickness();

			// find subsurfaces in surfaces which have already cutted openings
			// in this case surface and subsurface must have same points
			std::vector<std::pair<size_t,size_t>> samepoints = constrSurf.samePoints(opSurf);
			if(samepoints.size() > 2) {
				// heal the construction surface by merging with subsurface
				constrSB->mergeSurface(opSurf);
				constrSB->addContainedOpeningSpaceBoundaries(openingSB);
				addedOpeningIds.push_back(openingSB->m_id);
			}
			// check for parallel and intersected surfaces
			else {
				if(constrSurf.isParallelTo(opSurf)) {
					double dist = constrSurf.distanceToParallelPlane(opSurf);
					bool isIntersected = constrSurf.isIntersected(opSurf);
					if(dist <= searchDist*1.1 && isIntersected) {
						constrSB->addContainedOpeningSpaceBoundaries(openingSB);
						addedOpeningIds.push_back(openingSB->m_id);
					}
					else {
						if(dist > searchDist*1.1) {
							++wrongDistance;
						}
						else if(!isIntersected) {
							++notIntersected;
						}
					}
				}
				else {
					++notParallel;
				}
			}
		}
		// check if and whay the opening isn't found
		int constructionCount = constructionSBs.size();
		int failures = notParallel + wrongDistance + notIntersected;
		if(failures == constructionCount) {
			errors.push_back(ConvertError{OT_Space, m_id, "Opening space boundary id '" + std::to_string(openingSB->m_id) + "' has no connection"});
		}

	}
	std::vector<int> missingOpeningIds;
	for(auto openingSB : openingSBs) {
		int id = openingSB->m_id;
		if(std::find_if(addedOpeningIds.begin(),addedOpeningIds.end(), [id](int addedId) -> bool { return id == addedId; }) == addedOpeningIds.end())
			missingOpeningIds.push_back(id);
	}

	if(missingOpeningIds.size() > 0)
		errors.push_back(ConvertError{OT_Space, m_id, "Opening space boundaries without connection found"});
	return true;
}


bool Space::evaluateSpaceBoundariesFromConstruction(const BuildingElementsCollector& buildingElements, std::vector<Opening>& openings,
									std::vector<ConvertError>& errors) {

	std::vector<std::shared_ptr<SpaceBoundary>> newSpaceBoundaries = createSpaceBoundaries(buildingElements, errors);
	createSpaceBoundariesForOpeningsFromSpaceBoundaries(newSpaceBoundaries, buildingElements, openings, errors);

	m_spaceBoundaries = newSpaceBoundaries;

	if(m_spaceBoundaries.empty()) {
		errors.push_back({OT_Space, m_id, "Cannot evaluate any space boundary for this space"});
		return false;
	}
	return true;
}

bool Space::updateSpaceBoundaries(const objectShapeTypeVector_t& shapes,
								  shared_ptr<UnitConverter>& unit_converter,
								  const BuildingElementsCollector& buildingElements,
								  std::vector<Opening>& openings,
								  bool useSpaceBoundaries,
								  std::vector<ConvertError>& errors) {

	bool success;
	// update existing space boundaries from IFC
	if(useSpaceBoundaries && !m_spaceBoundaries.empty()) {
		// get space boundary types and set element id connections
		// convert geometry and create surfaces
		success = evaluateSpaceBoundaryFromIFC(shapes, buildingElements, unit_converter, errors);
	}
	// try to evaluate space boundaries from building element entities
	else {
		success = evaluateSpaceBoundariesFromConstruction(buildingElements, openings, errors);
	}
	return success;
}

const std::vector<std::shared_ptr<SpaceBoundary>>& Space::spaceBoundaries() const {
	return m_spaceBoundaries;
}

void Space::removeDublicatedSpaceBoundaries() {
	if(m_spaceBoundaries.size() < 2)
		return;

	std::set<size_t> indicesToRemove;
	for(size_t i=0; i<m_spaceBoundaries.size()-1; ++i) {
		auto sb1 = m_spaceBoundaries[i];
		for(size_t j=i+1; j<m_spaceBoundaries.size(); ++j) {
			auto sb2 = m_spaceBoundaries[j];
			if(sb1->surface().isEqualTo(sb2->surface()))
				indicesToRemove.insert(j);
		}
	}
	if(!indicesToRemove.empty()) {
		std::vector<std::shared_ptr<SpaceBoundary>> tempSbs;
		for(size_t i=0; i<m_spaceBoundaries.size(); ++i) {
			if(indicesToRemove.count(i) == 0)
				tempSbs.push_back(m_spaceBoundaries[i]);
		}
		m_spaceBoundaries = tempSbs;
	}
}

std::vector<int> Space::checkUniqueSubSurfaces() const {
	std::vector<int> res;
	std::set<int> usedSubSurfaceIds;
	for(auto sb : m_spaceBoundaries) {
		for(auto subsb : sb->containedOpeningSpaceBoundaries()) {
			if(usedSubSurfaceIds.count(subsb->surface().id()) > 0) {
				res.push_back(subsb->m_id);
			}
			else {
				usedSubSurfaceIds.insert(subsb->surface().id());
			}
		}
	}
	return res;
}

void Space::checkForEqualSpaceBoundaries(std::vector<std::pair<int,int>>& equalSBs) const {
	if(m_spaceBoundaries.size() < 2)
		return;

	for(size_t i=0; i<m_spaceBoundaries.size()-1; ++i) {
		auto sb1 = m_spaceBoundaries[i];
		for(size_t j=i+1; j<m_spaceBoundaries.size(); ++j) {
			auto sb2 = m_spaceBoundaries[j];
			if(sb1->surface().isEqualTo(sb2->surface()))
				equalSBs.push_back({sb1->m_id, sb2->m_id});
		}
	}
}

bool Space::isIntersected(const Space& other) const {
	for(const auto& p1 : m_meshSets) {
		for(const auto& p2 : other.m_meshSets) {
			if(IFCC::isIntersected(p1.get(), p2.get()))
				return true;
		}
	}
	for(const auto& p1 : m_spaceBoundaries) {
		for(const auto& p2 : other.m_spaceBoundaries) {
			const Surface & surf1 = p1->surface();
			const Surface & surf2 = p2->surface().polygon();
			if(surf1.isValid() && surf2.isValid()) {
				if(IBKMK::polyIntersect(surf1.polygon(), surf2.polygon()))
					return true;
			}
		}
	}
	return false;
}

TiXmlElement * Space::writeXML(TiXmlElement * parent, bool positiveRotation) const {
	if (m_id == -1)
		return nullptr;

	TiXmlElement * e = new TiXmlElement("Room");
	parent->LinkEndChild(e);

	e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_longName.empty())
		e->SetAttribute("displayName", m_longName);
	else if (!m_name.empty())
		e->SetAttribute("displayName", m_name);
//	e->SetAttribute("visible", IBK::val2string<bool>(true));

	if(!m_spaceBoundaries.empty()) {
		TiXmlElement * child = new TiXmlElement("Surfaces");
		e->LinkEndChild(child);
		for(auto sb : m_spaceBoundaries) {
			if(!sb->isOpeningElement()) {
				Surface s = sb->surfaceWithSubsurfaces();

				// check for rotation type
				double area = s.signedArea();
				bool isPositive = area >= 0;
				if((isPositive && !positiveRotation) || (!isPositive && positiveRotation))
					s.flip(positiveRotation);

				s.writeXML(child);
			}
		}
	}
	return e;
}

bool Space::hasSpaceBoundary(const std::string &guid) const {
	for(const std::string& currguid : m_spaceBoundaryGUIDs) {
		if(currguid == guid)
			return true;
	}
	return false;
}

bool Space::shareSameSpaceBoundary(const Space &space) const {
	for(const std::string& currguid : m_spaceBoundaryGUIDs) {
		if(space.hasSpaceBoundary(currguid))
			return true;
	}
	return false;
}


} // namespace IFCC
