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
		std::shared_ptr<SpaceBoundary> sb = std::shared_ptr<SpaceBoundary>(new SpaceBoundary(GUID_maker::instance().guid()));
		bool res = sb->setFromIFC(boundP);
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

	surfacesFromRepresentation(productShape, m_surfacesOrg);

	if(m_surfacesOrg.empty())
		return;

//	for(size_t i=0; i<m_surfacesOrg.size()-1; ++i) {
//		for(size_t j=i+1; j<m_surfacesOrg.size(); ++j) {
//			if(m_surfacesOrg[i].isParallelTo(m_surfacesOrg[j])) {
//				double dist = m_surfacesOrg[i].distanceToParallelPlane(m_surfacesOrg[j]);
//				m_parallelSpaceSurfaces.emplace_back(SurfaceConnection{(int)i,0,(int)j,dist});
//			}
//		}
//	}
}

static void divideSurface(const Surface::IntersectionResult& intRes, std::vector<Surface>& spaceSurfaces, int ssIndex, std::vector<Surface>& subsurfaces) {
	// we don't have any intersections
	if(intRes.m_intersections.empty())
		return;

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


std::vector<std::shared_ptr<SpaceBoundary>> Space::createSpaceBoundaries(const BuildingElementsCollector& buildingElements) {
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
				++loopCount;
				Surface::IntersectionResult intersectionResult = surfaces[indices.m_spaceSurfaceIndex].
																 intersect2(construction->surfaces()[indices.m_wallSurfaceIndex]);
				// no intersections found
				if(intersectionResult.m_intersections.empty())
					break;

				for(const Surface& surf : intersectionResult.m_intersections) {
					std::shared_ptr<SpaceBoundary> sb = std::shared_ptr<SpaceBoundary>(new SpaceBoundary(GUID_maker::instance().guid()));
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
		sb->setForMissingElement(name, *this);
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
																std::vector<Opening>& openings) {
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
								sb->setForVirtualElement(name, *this);
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
											 const std::vector<Opening>& openings) {
	if(openings.empty())
		return;

	if(m_longName == "Buero") {
		int t1 = 0;
	}
	for(const auto& construction : buildingElements.m_openingElements) {
		if(!construction->isSubSurfaceComponent())
			continue;

		if(construction->m_name == "EG-Fenster-6") {
			int t2 = 0;
		}
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
//						// remove opening i
//						auto fit = std::find(openingIndicesFound.begin(), openingIndicesFound.end(), match.m_openingIndex);
//						if(fit != openingIndicesFound.end())
//							openingIndicesFound.erase(fit);
					}
				}
			}
		}
		// construction don't have a thickness
		else {

		}
	} // loop over all opening constructions

	// \todo schleife über alle nicht gefundenen openings und versuche match auf andere Art zu finden
//	for(int opIndex : openingIndicesFound) {
//		const Opening& op = openings[opIndex];
//	}
}

static int typeFromElementShape(const shared_ptr<SpaceBoundary>& sb, const objectShapeTypeVector_t& shapes) {
	for(const auto& elemType : shapes) {
		for(const auto& elem : elemType.second) {
			if(sb->guidRelatedElement() == ws2s(elem->m_entity_guid)) {
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
	for(size_t sbI=0; sbI<m_spaceBoundaries.size(); ++sbI) {
		auto& sb = m_spaceBoundaries[sbI];

		bool res = sb->fetchGeometryFromIFC(unit_converter, m_transformMatrix, errors);
		if(res)
			foundOne = true;

	} // end loop over space boundaries

	if(!foundOne) {
		errors.push_back({OT_Space, m_id, "Cannot find at least one connected space boundary"});
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
	for(auto constrSB : constructionSBs ) {
		Surface& constrSurf = constrSB->surface();
		for(auto openingSB : openingSBs) {
			const Surface& opSurf = openingSB->surface();
			int elementId = constrSB->m_elementEntityId;
			std::shared_ptr<BuildingElement> element = buildingElements.fromID(elementId);
			double searchDist = 1;
			if(element)
				searchDist = element->thickness();

			// find subsurfaces in surfaces which have already cutted openings
			// in this case surface and subsurface must have same points
			std::vector<std::pair<size_t,size_t>> samepoints = constrSurf.samePoints(opSurf);
			if(samepoints.size() > 2) {
				// heal the construction surface by merging with subsurface
				constrSurf.merge(opSurf);
				constrSB->addContainedOpeningSpaceBoundaries(openingSB);
			}
			// check for parallel and intersected surfaces
			else {
				if(constrSurf.isParallelTo(opSurf)) {
					double dist = constrSurf.distanceToParallelPlane(opSurf);
					if(dist <= searchDist*1.1 && constrSurf.isIntersected(opSurf)) {
						constrSB->addContainedOpeningSpaceBoundaries(openingSB);
					}
				}
			}
		}
	}
	return true;
}


bool Space::evaluateSpaceBoundariesFromConstruction(const BuildingElementsCollector& buildingElements, std::vector<Opening>& openings,
									std::vector<ConvertError>& errors) {

	std::vector<std::shared_ptr<SpaceBoundary>> newSpaceBoundaries = createSpaceBoundaries(buildingElements);
	createSpaceBoundariesForOpeningsFromSpaceBoundaries(newSpaceBoundaries, buildingElements, openings);

	m_spaceBoundaries = newSpaceBoundaries;

	if(m_spaceBoundaries.empty()) {
		errors.push_back({OT_Space, m_id, "Cannot evaluate any space boundary for this space"});
		return false;
	}
	else {
		// create connection vector for current space boundary
//		for(size_t spaceBoundaryI=0; spaceBoundaryI<m_spaceBoundaries.size(); ++spaceBoundaryI) {
//			for(size_t spaceI=0; spaceI<m_surfacesOrg.size(); ++spaceI) {
//				for(size_t sbsI=0; sbsI<m_spaceBoundaries[spaceBoundaryI]->surfaces().size(); ++sbsI) {
//					const Surface& spaceSurf = m_surfacesOrg[spaceI];
//					const Surface& sbSurf = m_spaceBoundaries[spaceBoundaryI]->surfaces()[sbsI];
//					bool parallel = spaceSurf.isParallelTo(sbSurf);
//					if(parallel) {
//						double dist = spaceSurf.distanceToParallelPlane(sbSurf);
//						if(dist<10) {
//							m_connectionVectors.m_spaceBoundaryConnections.emplace_back(SurfaceConnection{(int)spaceI,(int)spaceBoundaryI,(int)sbsI,dist});
//						}
//					}
//				}
//			}
//		}
	}
	return true;
}

void Space::updateSpaceBoundaries(const objectShapeTypeVector_t& shapes,
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

//	if(success)
//		updateSurfaces(buildingElements);
}

//void Space::updateSpaceConnections(BuildingElementsCollector& buildingElements, std::vector<Opening>& openings) {

//	for(size_t spaceSurfI=0; spaceSurfI<m_surfacesOrg.size(); ++spaceSurfI) {

//		// create connection vector for construction
//		for(size_t conI=0; conI<buildingElements.m_constructionElements.size(); ++conI) {
//			std::shared_ptr<BuildingElement>& constr = buildingElements.m_constructionElements[conI];
//			double thickness = constr->thickness();
//			if(thickness <= 0)
//				thickness = 1;
//			for(size_t entSurfI=0; entSurfI<constr->surfaces().size(); ++entSurfI) {
//				const Surface& spaceSurf = m_surfacesOrg[spaceSurfI];
//				const Surface& conSurf = constr->surfaces()[entSurfI];
//				bool parallel = spaceSurf.isParallelTo(conSurf);
//				if(parallel) {
//					double dist = spaceSurf.distanceToParallelPlane(conSurf);
//					if(dist<=thickness*1.1 && spaceSurf.isIntersected(conSurf)) {
//						m_connectionVectors.m_constructionElementConnections.emplace_back(SurfaceConnection{(int)spaceSurfI,constr->m_id,(int)entSurfI,dist});
//						constr->m_spaceSurfaceConnection[m_id].push_back(std::pair<size_t,size_t>(spaceSurfI,entSurfI));
//					}
//				}
//			}
//		}

//		// create connection vector for construction similar elements
//		for(size_t conI=0; conI<buildingElements.m_constructionSimilarElements.size(); ++conI) {
//			std::shared_ptr<BuildingElement>& constr = buildingElements.m_constructionSimilarElements[conI];
//			double thickness = constr->thickness();
//			if(thickness <= 0)
//				thickness = 1;
//			for(size_t entSurfI=0; entSurfI<constr->surfaces().size(); ++entSurfI) {
//				const Surface& spaceSurf = m_surfacesOrg[spaceSurfI];
//				const Surface& conSurf = constr->surfaces()[entSurfI];
//				bool parallel = spaceSurf.isParallelTo(conSurf);
//				if(parallel) {
//					double dist = spaceSurf.distanceToParallelPlane(conSurf);
//					if(dist<=thickness*1.1 && spaceSurf.isIntersected(conSurf)) {
//						m_connectionVectors.m_constructionElementConnections.emplace_back(SurfaceConnection{(int)spaceSurfI,constr->m_id,(int)entSurfI,dist});
//						constr->m_spaceSurfaceConnection[m_id].push_back(std::pair<size_t,size_t>(spaceSurfI,entSurfI));
//					}
//				}
//			}
//		}

//		// create connection vector for opening elements
//		for(size_t conI=0; conI<buildingElements.m_openingElements.size(); ++conI) {
//			std::shared_ptr<BuildingElement>& constr = buildingElements.m_openingElements[conI];
//			double thickness = constr->thickness();
//			if(thickness <= 0)
//				thickness = 1;
//			for(size_t entSurfI=0; entSurfI<constr->surfaces().size(); ++entSurfI) {
//				const Surface& spaceSurf = m_surfacesOrg[spaceSurfI];
//				const Surface& opSurf = constr->surfaces()[entSurfI];
//				bool parallel = spaceSurf.isParallelTo(opSurf);
//				if(parallel) {
//					double dist = spaceSurf.distanceToParallelPlane(opSurf);
//					if(dist<=thickness*1.1 && spaceSurf.isIntersected(opSurf)) {
//						m_connectionVectors.m_openingElementConnections.emplace_back(SurfaceConnection{(int)spaceSurfI,constr->m_id,(int)entSurfI,dist});
//						constr->m_spaceSurfaceConnection[m_id].push_back(std::pair<size_t,size_t>(spaceSurfI,entSurfI));
//					}
//				}
//			}
//		}

//		// create connection vector for openings
//		for(size_t opI=0; opI<openings.size(); ++opI) {
//			Opening& opening = openings[opI];
//			double thickness = 0.5;
//			for(size_t entSurfI=0; entSurfI<opening.surfaces().size(); ++entSurfI) {
//				const Surface& spaceSurf = m_surfacesOrg[spaceSurfI];
//				const Surface& opSurf = opening.surfaces()[entSurfI];
//				bool parallel = spaceSurf.isParallelTo(opSurf);
//				if(parallel) {
//					double dist = spaceSurf.distanceToParallelPlane(opSurf);
//					if(dist<=thickness*1.1 && spaceSurf.isIntersected(opSurf)) {
//						m_connectionVectors.m_openingConnections.emplace_back(SurfaceConnection{(int)spaceSurfI,(int)opI,(int)entSurfI,dist});
//						opening.m_spaceSurfaceConnection[m_id].push_back(std::pair<size_t,size_t>(spaceSurfI,entSurfI));
//					}
//				}
//			}
//		}

//	}
//}

//void Space::updateSurfaces(const BuildingElementsCollector& buildingElements) {

//	m_surfaces.clear();
//	std::vector<Surface> subSurfaces;

//	if(!m_spaceBoundaries.empty()) {
//		for( const auto& sb : m_spaceBoundaries) {
//			if(sb->isConstructionElement()) {
//				m_surfaces.insert(m_surfaces.end(), sb->surface());
//			}
//			else if(sb->isOpeningElement()) {
//				subSurfaces.insert(subSurfaces.end(), sb->surface());
//			}
//			else {
//				if(sb->isVirtual()) {
//					m_surfaces.insert(m_surfaces.end(), sb->surface());
//				}
//				else {
//					m_surfaces.insert(m_surfaces.end(), sb->surface());
//				}
//			}
//		}

//		std::vector<std::shared_ptr<BuildingElement>> constructionElements = buildingElements.allConstructionElements();
//		std::vector<OpeningMatching> subSurfaceMatch;
//		for(size_t i=0; i<m_surfaces.size(); ++i) {
//			const Surface& curSurf = m_surfaces[i];
//			int elementIndex = -1;
//			for(int eli=0; eli<constructionElements.size(); ++eli) {
//				if(curSurf.elementId() == constructionElements[eli]->m_id) {
//					elementIndex = eli;
//					break;
//				}
//			}
//			double thickness = 0;
//			if(elementIndex>-1) {
//				const std::shared_ptr<BuildingElement>& elem = constructionElements[elementIndex];
//				thickness = elem->thickness();
//			}

//			for(size_t j=0; j<subSurfaces.size(); ++j) {
//				const Surface& sub = subSurfaces[j];

//				// find subsurfaces in surfaces which have already cutted openings
//				// in this case surface and subsurface must have same points
//				std::vector<std::pair<size_t,size_t>> samepoints = m_surfaces[i].samePoints(sub);
//				if(samepoints.size() > 2) {
//					subSurfaceMatch.emplace_back(OpeningMatching(OMT_SamePoints, i,j));
//				}
//				else {
//					bool parallel = m_surfaces[i].isParallelTo(sub);

//					// check if we can find a subsurface which is parallel to the surface
//					// then check if the distance of the subsurface to the surface is smaller than the thickness of the bulding element
//					// if this is the case the window or door can be inside of the wall
//					if(parallel && thickness > 0) {
//						double dist = m_surfaces[i].distanceToParallelPlane(sub);
//						// distance is smaller than thickness - check if we have an intersection
//						if(dist < thickness) {
//							if(m_surfaces[i].isIntersected(sub)) {
//								subSurfaceMatch.emplace_back(OpeningMatching(OMT_WallThicknessIntersection, i,j));
//							}
//						}
//					}
//				}
//			}
//		}

//		// first merge all subsurface in order to get complete original surface
//		for( const auto& match : subSurfaceMatch) {
//			if(match.m_type == OMT_SamePoints)
//				m_surfaces[match.m_surfaceIndex].merge(subSurfaces[match.m_subSurfaceIndex]);
//		}
//		// now we can create the subsurfaces in 2D surface coordinates
//		for( const auto& match : subSurfaceMatch) {
//			m_surfaces[match.m_surfaceIndex].addSubSurface(subSurfaces[match.m_subSurfaceIndex]);
//		}

//	}
//	else {
//		///< \todo Error handling?
//	}
//}

//const std::vector<Surface>& Space::surfaces() const {
//	return m_surfaces;
//}

const std::vector<std::shared_ptr<SpaceBoundary>>& Space::spaceBoundaries() const {
	return m_spaceBoundaries;
}


TiXmlElement * Space::writeXML(TiXmlElement * parent, bool flipPolygons) const {
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
			if(sb->isConstructionElement()) {
				Surface s = sb->surfaceWithSubsurfaces();
				if(flipPolygons)
					s.flip();
				s.writeXML(child);
			}
		}
	}
	return e;
}

bool Space::evaluateSpaceBoundaryTypes_old(const objectShapeTypeVector_t& shapes,
								 shared_ptr<UnitConverter>& unit_converter,
								 const BuildingElementsCollector& buildingElements,
									   std::vector<ConvertError>& errors) {
	if(m_spaceBoundaries.empty())
		return false;


	bool foundOne = false;

	std::vector<std::shared_ptr<BuildingElement>> constructionElements = buildingElements.allConstructionElements();

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

		bool res = sb->fetchGeometryFromIFC(unit_converter, m_transformMatrix, errors);
		if(res)
			foundOne = true;

//		// create connection vector for current space boundary
//		for(size_t spaceI=0; spaceI<m_surfacesOrg.size(); ++spaceI) {
//			for(size_t sbsI=0; sbsI<sb->surfaces().size(); ++sbsI) {
//				const Surface& spaceSurf = m_surfacesOrg[spaceI];
//				const Surface& sbSurf = sb->surfaces()[sbsI];
//				bool parallel = spaceSurf.isParallelTo(sbSurf);
//				if(parallel) {
//					double dist = spaceSurf.distanceToParallelPlane(sbSurf);
//					if(dist<10) {
//						m_connectionVectors.m_spaceBoundaryConnections.emplace_back(SurfaceConnection{(int)spaceI,(int)sbI,(int)sbsI,dist});
//					}
//				}
//			}
//		}

	} // end loop over space boundaries

	if(!foundOne) {
		errors.push_back({OT_Space, m_id, "Cannot find at least one connected space boundary"});
	}
	return foundOne;
}

//VICUS::Room Space::getVicusObject(std::map<int,int>& idMap, int& nextid) const {
//	VICUS::Room res;
//	int newId = nextid++;
//	if (!m_longName.empty())
//		res.m_displayName = QString::fromUtf8(m_longName.c_str());
//	else
//		res.m_displayName = QString::fromUtf8(m_name.c_str());
//	res.m_id = newId;
//	res.m_ifcGUID = m_guid;
//	idMap[m_id] = newId;

//	for(const auto& surface : m_surfaces) {
//		res.m_surfaces.emplace_back(surface.getVicusObject(idMap, nextid));
//	}

//	return res;
//}

//Space::SurfaceConnectionVectors Space::surfaceConnectionVectors() const {
//	return m_connectionVectors;
//}


} // namespace IFCC
