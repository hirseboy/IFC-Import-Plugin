#include "IFCC_Space.h"

#include <ifcpp/IFC4X3/include/IfcRelSpaceBoundary.h>
#include <ifcpp/IFC4X3/include/IfcLengthMeasure.h>

#include <numeric>
#include <algorithm>

#include <IBK_math.h>
#include <IBK_FormatString.h>

#include <IBKMK_3DCalculations.h>

#include <Carve/src/include/carve/carve.hpp>

#include "IFCC_MeshUtils.h"
#include "IFCC_Helper.h"

namespace IFCC {

Space::Space(int id) :
	EntityBase(id)
{
}

bool Space::set(std::shared_ptr<IFC4X3::IfcSpace> ifcSpace, std::vector<ConvertError>& errors) {
	if(!EntityBase::set(dynamic_pointer_cast<IFC4X3::IfcRoot>(ifcSpace)))
		return false;

	m_longName = label2s(ifcSpace->m_LongName);
	if(ifcSpace->m_PredefinedType != nullptr)
		m_spaceType = ifcSpace->m_PredefinedType->m_enum;

	// look for space boundaries from IFC
	for( const auto& bound : ifcSpace->m_BoundedBy_inverse) {
		auto boundP = bound.lock();
		std::shared_ptr<SpaceBoundary> sb = std::shared_ptr<SpaceBoundary>(new SpaceBoundary(GUID_maker::instance().guid()));
		bool res = sb->setFromIFC(boundP, errors);
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


static bool divideSurface(const Surface::IntersectionResult& intRes, std::vector<Surface>& spaceSurfaces, int ssIndex, std::vector<Surface>& subsurfaces) {
	// we don't have any intersections
	if(intRes.m_intersections.empty())
		return false;

	// construction element covers complete space surface
	if(intRes.m_diffBaseMinusClip.empty()) {
//		spaceSurfaces.erase(spaceSurfaces.begin() + ssIndex);
		return true;
	}

	// we have a rest of the space surface without matching construction element
	// we have only one resulting space surface - change original surface to it
	std::vector<Surface> diffSurfaces;
	for(size_t i=0; i<intRes.m_diffBaseMinusClip.size(); ++i) {
		Surface surf = intRes.m_diffBaseMinusClip[i];
		std::vector<Surface> tmp = surf.getSimplified();

		// should never happen
		if(tmp.empty())
			return false;

		// we have some holes - add these to subsurface list
		if(!intRes.m_holesBaseMinusClip[i].empty()) {
			for(const Surface& subsurf : intRes.m_holesBaseMinusClip[i])
				subsurfaces.push_back(subsurf);
		}
		for(const auto& s : tmp) {
			diffSurfaces.push_back(s);
		}
	}

	spaceSurfaces[ssIndex] = diffSurfaces.front();
	for(size_t i=1; i<diffSurfaces.size(); ++i) {
		spaceSurfaces.push_back(diffSurfaces[i]);
	}
	return false;
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
		const Surface& wallSurf = wallSurfaces[wi];
		for(size_t si=0; si<spaceSurfaces.size(); ++si) {
			const Surface& spaceSurf = spaceSurfaces[si];
			if(spaceSurf.isParallelTo(wallSurf)) {
				double dist = spaceSurf.distanceToParallelPlane(wallSurf);
				if(dist < minDist * (1+EPS)) {
					if(wallSurf.isIntersected(spaceSurf))
						return MatchResult(wi,si);
				}
			}
		}
	}
	return MatchResult();
}


std::vector<std::shared_ptr<SpaceBoundary>> Space::createSpaceBoundaries(const BuildingElementsCollector& buildingElements, std::vector<ConvertError>& errors,
																		 const ConvertOptions& convertOptions) {
	std::vector<Surface> surfaces(m_surfacesOrg);
	std::vector<std::shared_ptr<SpaceBoundary>> spaceBoundaries;
	std::vector<std::shared_ptr<BuildingElement>> constructionElements = buildingElements.allConstructionElements();


	if(m_name == "A2.25") {
		std::string surfSpace = dumpSurfaces(surfaces);
		std::string dump;
		dump = "Space A2.25 \n";
		dump += surfSpace;
		std::ofstream out("/home/fechner/temp/space_surfaces_org.txt");
		out << dump;
	}

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

		if(m_name == "A2.25" && (construction->m_ifcId == 60525 || construction->m_ifcId == 50176 || construction->m_ifcId == 52161)) {
			std::string surfSpace = dumpSurfaces(surfaces);
			std::string surfConstr = dumpSurfaces(construction->surfaces());
			std::string dump;
			dump = "Space A2.25 \n";
			dump += surfSpace;
			dump += "\nConstruction 60525\n";
			dump += surfConstr;
			std::ofstream out("/home/fechner/temp/space_surfaces.txt");
			out << dump;
		}

		MatchResult indices = findFirstSurfaceMatchIndex(construction->surfaces(), surfaces, dist*2);
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

				for(size_t i=0; i<intersectionResult.m_intersections.size(); ++i) {
					if(!intersectionResult.m_holesIntersections[i].empty()) {
//						errors.push_back(ConvertError{OT_Space, m_id, IBK::FormatString("intersection from space surface and building element surface has %1 holes")
//													  .arg(intersectionResult.m_holesIntersections[i].size()).str()});
					}
				}

				if(intersectionResult.holesWithChilds() > 0) {
					errors.push_back(ConvertError{OT_Space, m_id, "one or more holes in intersections or diff surface has childs"});
				}

				const Surface& firstISurf = intersectionResult.m_intersections.front();
				if(intersectionResult.m_intersections.size() == 1 && IBK::nearly_equal<2>(firstISurf.area(),currSpaceSurf.area())) {
					int id = GUID_maker::instance().guid();
					std::shared_ptr<SpaceBoundary> sb = std::shared_ptr<SpaceBoundary>(new SpaceBoundary(id));
					std::string name = m_name + ":" + construction->m_name + " - " +
							std::to_string(indices.m_spaceSurfaceIndex) +
							" : " + std::to_string(indices.m_wallSurfaceIndex);
					sb->setFromBuildingElement(name, construction, *this);
					sb->m_elementEntityId = construction->m_id;
					sb->fetchGeometryFromBuildingElement(firstISurf);
					spaceBoundaries.push_back(sb);
					surfaces.erase(surfaces.begin() + indices.m_spaceSurfaceIndex);
				}
				else {
					for(const Surface& surf : intersectionResult.m_intersections) {
						int id = GUID_maker::instance().guid();
						std::shared_ptr<SpaceBoundary> sb = std::shared_ptr<SpaceBoundary>(new SpaceBoundary(id));
						std::string name = m_name + ":" + construction->m_name + " - " +
								std::to_string(indices.m_spaceSurfaceIndex) + " : " + std::to_string(indices.m_wallSurfaceIndex);
						sb->setFromBuildingElement(name, construction, *this);
						sb->m_elementEntityId = construction->m_id;
						sb->fetchGeometryFromBuildingElement(surf);
						spaceBoundaries.push_back(sb);
					}
					std::vector<Surface> subsurfaces;
					// add difference surface - intersections to the surface list and remove the original one
					divideSurface(intersectionResult, surfaces, indices.m_spaceSurfaceIndex, subsurfaces);
					if(!subsurfaces.empty()) {
						// what should we do with the holes?
						errors.push_back(ConvertError{OT_Space, m_id, "rest surface from intersection from space surface and building element surface has holes"});
					}
				}
				indices = findFirstSurfaceMatchIndex(construction->surfaces(), surfaces, dist);

				if(loopCount > 1000) {
					errors.push_back(ConvertError{OT_Space, m_id, "more than 1000 intersections found"});
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

struct ConstructionSurfaceInfo {
	int						m_id = -1;
	int						m_surfaceIndex = -1;
	BuildingElementTypes	m_type = IFCC::BET_None;
	double					m_distance = 0;
	double					m_intersectionArea = 0;

	bool isValid() const { return m_id > -1; }
};

struct SpaceSurfaceMatches {
	int										m_spaceSurfaceIndex;
	std::vector<ConstructionSurfaceInfo>	m_constructions;
};

static ConstructionSurfaceInfo findFirstSurfaceMatchIndex_2(const std::vector<Surface>& constructionSurfaces, const Surface& spaceSurface, double minDist) {
	const double EPS = 1e-3;

	// collects all
	std::vector<ConstructionSurfaceInfo> matches;
	for(size_t wi=0; wi<constructionSurfaces.size(); ++wi) {
		const Surface& constructionSurface = constructionSurfaces[wi];
		if(spaceSurface.isParallelTo(constructionSurface)) {
			double dist = spaceSurface.distanceToParallelPlane(constructionSurface);
			if(dist < minDist * (1+EPS)) {
				Surface intersection = spaceSurface.intersect(constructionSurface);
				double area = intersection.area();
				if(intersection.isValid() && area > 1e-4) {
					ConstructionSurfaceInfo	conInfo;
					conInfo.m_id = 0;				// set to 0 indicates we have found something. Real id will be set later
					conInfo.m_distance = dist;
					conInfo.m_intersectionArea = area;
					conInfo.m_surfaceIndex = wi;	// Index of surface in construction surface list
					matches.push_back(conInfo);
				}
			}
		}
	}

	if(matches.empty())
		return ConstructionSurfaceInfo();

	double maxArea = matches[0].m_intersectionArea;
	int returnIndex = 0;
	for( size_t i=1; i<matches.size(); ++i) {
		if(matches[i].m_intersectionArea > maxArea) {
			maxArea = matches[i].m_intersectionArea;
			returnIndex = i;
		}
	}
	return matches[returnIndex];
}

static std::shared_ptr<SpaceBoundary> createSpaceBoundary(const std::shared_ptr<BuildingElement>& constr, const SpaceSurfaceMatches& match, int constrSurfIndex, const Space& space, const Surface& intersection ) {
	int id = GUID_maker::instance().guid();
	std::shared_ptr<SpaceBoundary> sb = std::shared_ptr<SpaceBoundary>(new SpaceBoundary(id));
	std::string name = space.m_name + ":" + constr->m_name + " - " +
			std::to_string(match.m_spaceSurfaceIndex) +
			" : " + std::to_string(constrSurfIndex);
	sb->setFromBuildingElement(name, constr, space);
	sb->m_elementEntityId = constr->m_id;
	sb->fetchGeometryFromBuildingElement(intersection);
	return sb;
}

std::vector<std::shared_ptr<SpaceBoundary>> Space::createSpaceBoundaries_2(const BuildingElementsCollector& buildingElements, std::vector<ConvertError>& errors,
																		   const ConvertOptions& convertOptions) {
	std::vector<Surface> surfaces(m_surfacesOrg);
	std::vector<std::shared_ptr<SpaceBoundary>> spaceBoundaries;
	std::vector<std::shared_ptr<BuildingElement>> constructionElements = buildingElements.allConstructionElements();

	std::vector<SpaceSurfaceMatches> matches;

	bool foundOne = false;
	int loopCount = 0;

	ConvertOptions::ConstructionMatching matchType = convertOptions.m_matchingType;

	if(matchType != ConvertOptions::CM_NoMatching) {

		const int MAX_LOOP_COUNT = matchType == ConvertOptions::CM_MatchOnlyFirstConstruction ? 1 : (matchType == ConvertOptions::CM_MatchFirstNConstructions ? convertOptions.m_matchedConstructionNumbers : 500);

		do {
			matches.clear();
			foundOne = false;
			bool noConstructions = true;
			for(size_t i=0; i<surfaces.size(); ++i) {
				Surface& currentSurf = surfaces[i];
				matches.emplace_back(SpaceSurfaceMatches());
				matches.back().m_spaceSurfaceIndex = i;

				for(const auto& construction : constructionElements) {

					BuildingElementTypes type = construction->type();

					if(!convertOptions.hasElementsForSpaceBoundaries(type))
						continue;

					double dist = construction->thickness();
					double maxConstructionDist = 0;
					if(construction->isSubSurfaceComponent() && !construction->m_openingProperties.m_constructionThicknesses.empty()) {
						maxConstructionDist = *std::max_element(construction->m_openingProperties.m_constructionThicknesses.begin(),
																construction->m_openingProperties.m_constructionThicknesses.end());
					}
					const double MIN_THICKNESS = 0.1;
					if(dist < MIN_THICKNESS) {
						if(maxConstructionDist > MIN_THICKNESS)
							dist = maxConstructionDist;
						else
							dist = MIN_THICKNESS;
					}
					dist *= convertOptions.m_distanceFactor;

					// add all construction surface which are parallel and more or less near the space surface
					IFCC::ConstructionSurfaceInfo	conInfo = findFirstSurfaceMatchIndex_2(construction->surfaces(), currentSurf, dist);
					if(conInfo.isValid()) {
						conInfo.m_id = construction->m_id;		// update real construction id
						conInfo.m_type = construction->type();
						matches.back().m_constructions.push_back(conInfo);
						noConstructions = false;
					}
				}
			}

			// matches contains now a vector of all space surfaces which contains a vector of all possible matching construction surfaces

			if(noConstructions)
				break;

			// run over all matches
			std::vector<Surface> surfacesBackup = surfaces;

			std::set<int> indicesToErase;

			for(const SpaceSurfaceMatches& match : matches) {
				if(match.m_constructions.empty())
					continue;

				// store the current stage of space surfaces. It is necessery in order to keep the indices inside the loop
				// try to find best fitting surface
				std::vector<int> biggestSurfIndex(match.m_constructions.size());			///< Index vector of the surface with the largest interception
				std::vector<int> smallestDistanceIndex(match.m_constructions.size());		///< Index vector of the surface with the smallest distance

				// temporary storage for construction vector of the current space surface
				const std::vector<ConstructionSurfaceInfo>& matchConstructions = match.m_constructions;

				// start vector contains the indices of all construction surfaces
				std::iota(biggestSurfIndex.begin(), biggestSurfIndex.end(), 0);
				std::iota(smallestDistanceIndex.begin(), smallestDistanceIndex.end(), 0);

				// indices of the construction surfaces will be sorted according surface area and distances
				// biggestSurfIndex - biigest surface is the first one and all other follows
				std::sort(biggestSurfIndex.begin(), biggestSurfIndex.end(), [ &matchConstructions](int a, int b) -> bool { return matchConstructions[a].m_intersectionArea > matchConstructions[b].m_intersectionArea; } );
				// surface with the smallest distance to current space surface is the first one and all others follows
				std::sort(smallestDistanceIndex.begin(), smallestDistanceIndex.end(), [ &matchConstructions](int a, int b) -> bool { return matchConstructions[a].m_distance < matchConstructions[b].m_distance; } );

				double currentArea = matchConstructions[smallestDistanceIndex[0]].m_distance;
				std::vector<int> firstIndexList(1, 0);

				int fi = 0;
				for(size_t i=1; i<smallestDistanceIndex.size(); ++i) {
					if(!IBK::near_equal(matchConstructions[smallestDistanceIndex[i]].m_distance, currentArea)) {
						++fi;
						currentArea = matchConstructions[smallestDistanceIndex[i]].m_distance;
					}
					firstIndexList.push_back(fi);
				}

				size_t constructionCount = match.m_constructions.size();
				// posVect contains priority numbers for all construction surfaces
				// the lowest priority number has the highest priority
				// priority number includes position in area vector, position in distance vector and construction type ranking
				std::vector<int> posVect(constructionCount, 0);
				for(size_t i=0; i<constructionCount; ++i) {
					posVect[biggestSurfIndex[i]] += i;			// add position in area list

					posVect[smallestDistanceIndex[i]] += firstIndexList[i];		// add position in distance list

					// add number according construction type ranking
					if(matchConstructions[i].m_type == BET_Wall || matchConstructions[i].m_type == BET_Roof)
						posVect[i] += 0;
					else if(matchConstructions[i].m_type == BET_Slab)
						posVect[i] += 1;
					else if(matchConstructions[i].m_type == BET_Beam || matchConstructions[i].m_type == BET_Covering)
						posVect[i] += constructionCount / 2 + 4;
					else
						posVect[i] += constructionCount / 2 + 10;
				}

				// look for the index with the lowest element (highest priority)
				int bestMatch_1 = std::min_element(posVect.begin(), posVect.end()) - posVect.begin();

				// get construction from the id of the best match
				std::shared_ptr<BuildingElement> constr = buildingElements.fromID(matchConstructions[bestMatch_1].m_id);
				// get surface from this construction
				int bestConstrSurfaceIndex = matchConstructions[bestMatch_1].m_surfaceIndex;
				const Surface& currConstructionSurface = constr->surfaces()[bestConstrSurfaceIndex];
				// get current space surface
				const Surface& currSpaceSurface = surfacesBackup[match.m_spaceSurfaceIndex];

				// creat intersection tree
				Surface::IntersectionResult intersectionResult = currSpaceSurface.intersect2(currConstructionSurface);
				if(intersectionResult.m_intersections.empty())
					continue;

				foundOne = true;	///< at least one intesection for a space boundary found

				for(size_t i=0; i<intersectionResult.m_intersections.size(); ++i) {
					if(!intersectionResult.m_holesIntersections[i].empty()) {
						//						errors.push_back(ConvertError{OT_Space, m_id, IBK::FormatString("intersection from space surface and building element surface has %1 holes")
						//													  .arg(intersectionResult.m_holesIntersections[i].size()).str()});
					}
				}

				if(intersectionResult.holesWithChilds() > 0) {
					errors.push_back(ConvertError{OT_Space, m_id, "one or more holes in intersections or diff surface has childs"});
				}

				if(convertOptions.m_matchingType == ConvertOptions::CM_MatchOnlyFirstConstruction) {
					spaceBoundaries.push_back(createSpaceBoundary(constr, match, bestConstrSurfaceIndex, *this, currSpaceSurface));
					indicesToErase.insert(match.m_spaceSurfaceIndex);
					continue;
				}


				if(convertOptions.m_matchingType == ConvertOptions::CM_MatchFirstNConstructions) {

					if(loopCount < MAX_LOOP_COUNT - 1) {
						for(const auto& surf : intersectionResult.m_intersections)
							spaceBoundaries.push_back(createSpaceBoundary(constr, match, bestConstrSurfaceIndex, *this, surf));

						std::vector<Surface> subsurfaces;
						bool toErase = divideSurface(intersectionResult, surfaces, match.m_spaceSurfaceIndex, subsurfaces);
						if(toErase)
							indicesToErase.insert(match.m_spaceSurfaceIndex);
						if(!subsurfaces.empty()) {
							// what should we do with the holes?
							errors.push_back(ConvertError{OT_Space, m_id, "rest surface from intersection from space surface and building element surface has holes"});
						}
					}
					else {
						spaceBoundaries.push_back(createSpaceBoundary(constr, match, bestConstrSurfaceIndex, *this, currSpaceSurface));
						indicesToErase.insert(match.m_spaceSurfaceIndex);
					}
					continue;
				}
				else {
					const Surface& firstISurf = intersectionResult.m_intersections.front();
					double spaceArea = currSpaceSurface.area();
					double constructionArea = firstISurf.area();

					if((intersectionResult.m_intersections.size() == 1 && IBK::nearly_equal<2>(constructionArea,spaceArea))) {
						spaceBoundaries.push_back(createSpaceBoundary(constr, match, bestConstrSurfaceIndex, *this, firstISurf));
						indicesToErase.insert(match.m_spaceSurfaceIndex);
					}
					else {
						for(const Surface& surf : intersectionResult.m_intersections) {
							spaceBoundaries.push_back(createSpaceBoundary(constr, match, bestConstrSurfaceIndex, *this, surf));
						}
						std::vector<Surface> subsurfaces;
						// add difference surface - intersections to the surface list and remove the original one
						bool toErase = divideSurface(intersectionResult, surfaces, match.m_spaceSurfaceIndex, subsurfaces);
						if(toErase)
							indicesToErase.insert(match.m_spaceSurfaceIndex);
						if(!subsurfaces.empty()) {
							// what should we do with the holes?
							errors.push_back(ConvertError{OT_Space, m_id, "rest surface from intersection from space surface and building element surface has holes"});
						}
					}
				}
			}
			if(!indicesToErase.empty()) {
				std::vector<Surface> tempS;
				for(size_t i=0; i<surfaces.size(); ++i) {
					if(indicesToErase.find(i) == indicesToErase.end())
						tempS.push_back(surfaces[i]);
				}
				surfaces = tempS;
				indicesToErase.clear();
			}
			++loopCount;
		} while(foundOne && loopCount < MAX_LOOP_COUNT);

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

void Space::createSpaceBoundariesForOpeningsFromSpaceBoundaries(std::vector<std::shared_ptr<SpaceBoundary>>& spaceBoundaries,
																const BuildingElementsCollector& buildingElements,
																std::vector<Opening>& openings, std::vector<ConvertError>& errors,
																const ConvertOptions& convertOptions) {
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
									  [opid](const auto& op) -> bool { return op.m_id == opid; });
			if(fitOp != openings.end())
				containedOpeningsIndices.push_back(std::distance(openings.begin(), fitOp));
		}
		// only go further if we have some contained openings
		if(containedOpeningsIndices.empty())
			continue;

		// look for all openings which are related to the construction element of the space boundary
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
				if(dist <= 1e6) {
					if(currentOpeningSurf.isIntersected(spaceBoundary->surface())) {
						Surface intersectionResult = spaceBoundary->surface().intersect(currentOpeningSurf);
						if(intersectionResult.isValid()) {
							std::shared_ptr<SpaceBoundary> sb = std::shared_ptr<SpaceBoundary>(new SpaceBoundary(GUID_maker::instance().guid()));
							if(openingElem) {
								std::string name = m_longName + ":" + spaceBoundary->m_name+ ":" + openingElem->m_name + " - O" +
												   std::to_string(opIndex) + " : OS" +
												   std::to_string(cosi);
								sb->setFromBuildingElement(name, openingElem, *this);
								sb->m_elementEntityId = openingElem->m_id;
								sb->m_openingId = currOp.m_id;
								sb->fetchGeometryFromBuildingElement(intersectionResult);
								openingSpaceBoundaries.push_back(sb);
								currOp.setSpaceBoundary(sb);
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

	// now look for all non related openings
	for(Opening& currOp : openings) {
		if(currOp.hasSpaceBoundary())
			continue;

		for(const auto& spaceBoundary : spaceBoundaries) {
			// openings can only be part of a construction space boundary
			if(!spaceBoundary->isConstructionElement())
				continue;

			std::shared_ptr<BuildingElement> openingElem;
			// has no construction - its a breakout
			if(currOp.openingElementIds().size() == 1) {
				int id = currOp.openingElementIds().front();
				openingElem = buildingElements.fromID(id);
			}

			for(size_t cosi=0; cosi<currOp.surfaces().size(); ++cosi) {
				const Surface& currentOpeningSurf = currOp.surfaces()[cosi];
				double dist = currentOpeningSurf.distanceToParallelPlane(spaceBoundary->surface());
				if(dist > convertOptions.m_openingDistance)
					continue;

				Surface intersectionResult = spaceBoundary->surface().intersect(currentOpeningSurf);
				if(intersectionResult.isValid()) {
					std::shared_ptr<SpaceBoundary> sb = std::shared_ptr<SpaceBoundary>(new SpaceBoundary(GUID_maker::instance().guid()));
					if(openingElem) {
						std::string name = m_longName + ":" + spaceBoundary->m_name + ":" + openingElem->m_name + " : OS" +
										   std::to_string(cosi);
						sb->setFromBuildingElement(name, openingElem, *this);
						sb->m_elementEntityId = openingElem->m_id;
						sb->m_openingId = currOp.m_id;
						sb->fetchGeometryFromBuildingElement(intersectionResult);
						openingSpaceBoundaries.push_back(sb);
						currOp.setSpaceBoundary(sb);
					}
					else {
						std::string name = m_longName + spaceBoundary->m_name+ ": breakout - OS" +
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

	if(!openingSpaceBoundaries.empty()) {
		spaceBoundaries.insert(spaceBoundaries.end(), openingSpaceBoundaries.begin(), openingSpaceBoundaries.end());
	}
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
										 std::vector<ConvertError>& errors,
										 const ConvertOptions& convertOptions) {
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

	int wrongSurfaces = 0;
	for(auto sb : m_spaceBoundaries) {
		if(!sb->checkAndHealSurface(true))
			++wrongSurfaces;
	}
	if(wrongSurfaces > 0) {
		errors.push_back(ConvertError{OT_Space, m_id, "Space contains " + std::to_string(wrongSurfaces) + " space boundaries with non valid surface."});
	}

	// create two temporary vectors for construction space boundaries and opening space boundaries
	std::vector<std::shared_ptr<SpaceBoundary>> constructionSBs;
	std::vector<std::shared_ptr<SpaceBoundary>> openingSBs;
	for(auto sb : m_spaceBoundaries) {
		if(sb->isConstructionElement() || sb->isVirtual())
			constructionSBs.push_back(sb);
		if(sb->isOpeningElement())
			openingSBs.push_back(sb);
	}

	if(openingSBs.empty())
		return true;

	std::map<int,std::vector<int>> parallelOpeningSBs;
	for(size_t ci=0; ci<constructionSBs.size(); ++ci) {
		const Surface& constrSurf = constructionSBs[ci]->surface();
		for(size_t oi=0; oi<openingSBs.size(); ++oi) {
			const Surface& openingSurf = openingSBs[oi]->surface();
			if(constrSurf.isParallelTo(openingSurf))
				parallelOpeningSBs[ci].push_back(oi);
		}
	}

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
			double searchDist = convertOptions.m_openingDistance;
			if(element)
				searchDist = std::max(element->thickness(),searchDist);

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
		// check if and why the opening isn't found
		int constructionCount = constructionSBs.size();
		int failures = notParallel + wrongDistance + notIntersected;
		std::string failureReasonString;
		if(notParallel > 0)
			failureReasonString += "NP: " + std::to_string(notParallel) +  " : ";
		if(wrongDistance > 0)
			failureReasonString += "WD: " + std::to_string(wrongDistance) +  " : ";
		if(notIntersected > 0)
			failureReasonString += "NI: " + std::to_string(notIntersected);
		if(failures == constructionCount) {
			errors.push_back(ConvertError{OT_Space, m_id, "Opening space boundary id '" + std::to_string(openingSB->m_id) + "' has no connection because " + failureReasonString});
		}

	}

	std::vector<std::shared_ptr<SpaceBoundary>> missingOpeningSBs;
	for(auto openingSB : openingSBs) {
		int id = openingSB->m_id;
		if(std::find_if(addedOpeningIds.begin(),addedOpeningIds.end(), [id](int addedId) -> bool { return id == addedId; }) == addedOpeningIds.end()) {
			missingOpeningSBs.push_back(openingSB);
		}
	}

	if(missingOpeningSBs.size() > 0) {
		for(auto sb : missingOpeningSBs) {
			std::map<int,int> distIndexMap;
			errors.push_back(ConvertError{OT_Space, m_id, "Opening space boundary id '" + std::to_string(sb->m_ifcId) + "' is not connected to a construction space boundary"});
//			for(size_t ci=0; ci<constructionSBs.size(); ++ci) {
//				const Surface& constrSurf = constructionSBs[ci]->surface();
//				if(constrSurf.isParallelTo(sb->surface()) && constrSurf.isIntersected(sb->surface())) {
//					double dist = constrSurf.distanceToParallelPlane(sb->surface());
//					distIndexMap[int(dist*10000)] = ci;
//				}
//			}
//			if(!distIndexMap.empty()) {
//				double dist = distIndexMap.begin()->first / 10000.0;
//				int smallestIndex = distIndexMap.begin()->second;
//			}
		}
	}

	return true;
}


bool Space::evaluateSpaceBoundariesFromConstruction(const BuildingElementsCollector& buildingElements, std::vector<Opening>& openings,
									std::vector<ConvertError>& errors, const ConvertOptions& convertOptions) {

	std::vector<std::shared_ptr<SpaceBoundary>> newSpaceBoundaries = createSpaceBoundaries_2(buildingElements, errors, convertOptions);
	createSpaceBoundariesForOpeningsFromSpaceBoundaries(newSpaceBoundaries, buildingElements, openings, errors, convertOptions);

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
								  std::vector<ConvertError>& errors,
								  const ConvertOptions& convertOptions) {

	bool success;
	// update existing space boundaries from IFC
	if(useSpaceBoundaries && !m_spaceBoundaries.empty()) {
		// get space boundary types and set element id connections
		// convert geometry and create surfaces
		success = evaluateSpaceBoundaryFromIFC(shapes, buildingElements, unit_converter, errors, convertOptions);
	}
	// try to evaluate space boundaries from building element entities
	else {
		success = evaluateSpaceBoundariesFromConstruction(buildingElements, openings, errors, convertOptions);
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

TiXmlElement * Space::writeXML(TiXmlElement * parent) const {
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
			sb->writeXML(child);
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
