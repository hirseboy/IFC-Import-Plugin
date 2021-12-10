#include "IFCC_Space.h"

#include <ifcpp/IFC4/include/IfcRelSpaceBoundary.h>
#include <ifcpp/IFC4/include/IfcLengthMeasure.h>

#include <numeric>

#include <IBK_math.h>

#include "IFCC_Helper.h"

namespace IFCC {


static std::vector<std::pair<int,int>> surfaceMatchIndex(const std::vector<Surface>& wallSurfaces, const std::vector<Surface>& spaceSurfaces, double minDist) {

	std::vector<std::pair<int,int>> result;
	for(size_t wi=0; wi<wallSurfaces.size(); ++wi) {
		for(size_t si=0; si<spaceSurfaces.size(); ++si) {
			double dist = spaceSurfaces[si].distanceToParallelPlane(wallSurfaces[wi].planeNormal());
			if(dist < minDist) {
				result.push_back(std::make_pair(si,wi));
			}
		}
	}
	return result;
}


Space::Space(int id) :
	EntityBase(id)
{

}

bool Space::set(std::shared_ptr<IfcSpace> ifcSpace) {
	if(!EntityBase::set(dynamic_pointer_cast<IfcRoot>(ifcSpace)))
		return false;

	m_longName = label2s(ifcSpace->m_LongName);
	std::vector<shared_ptr<IfcObjectDefinition>> childs;
	getChildren(ifcSpace, childs);
	m_numChilds = childs.size();
	if(ifcSpace->m_PredefinedType != nullptr)
		m_spaceType = ifcSpace->m_PredefinedType->m_enum;
	if(ifcSpace->m_ElevationWithFlooring.get() != nullptr)
		m_lengthMeasure = ifcSpace->m_ElevationWithFlooring->m_value;

	// look for space boundaries from IFC
	for( const auto& bound : ifcSpace->m_BoundedBy_inverse) {
		auto boundP = bound.lock();
		SpaceBoundary sb(GUID_maker::instance().guid());
		bool res = sb.set(boundP);
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
	if(repCount > 0) {
		int itemDataCount = currentRep->m_vec_item_data.size();
		if(itemDataCount > 0) {
			m_meshSetClosedFinal = currentRep->m_vec_item_data.front()->m_meshsets;
			m_meshSetOpenFinal = currentRep->m_vec_item_data.front()->m_meshsets_open;
		}
	}

	meshVector_t& currentMeshSets =  m_meshSetClosedFinal.empty() ? m_meshSetOpenFinal : m_meshSetClosedFinal;
	simplifyMesh(currentMeshSets, false);

	if(!m_meshSetClosedFinal.empty()) {
		int msCount = m_meshSetClosedFinal.size();
		for(int i=0; i<msCount; ++i) {
			m_polyvectClosedFinal.push_back(std::vector<std::vector<std::vector<IBKMK::Vector3D>>>());
			const carve::mesh::MeshSet<3>& currMeshSet = *m_meshSetClosedFinal[i];
			convert(currMeshSet, m_polyvectClosedFinal.back());
			// get surfaces
			for(size_t mi=0; mi<currMeshSet.meshes.size(); ++mi) {
				for(size_t fi =0; fi<currMeshSet.meshes[mi]->faces.size(); ++fi) {
					if(currMeshSet.meshes[mi]->faces[fi] != nullptr)
						m_surfacesOrg.emplace_back(Surface(currMeshSet.meshes[mi]->faces[fi]));
				}
			}
		}
	}
	if(!m_meshSetOpenFinal.empty()) {
		int msCount = m_meshSetOpenFinal.size();
		for(int i=0; i<msCount; ++i) {
			m_polyvectOpenFinal.push_back(std::vector<std::vector<std::vector<IBKMK::Vector3D>>>());
			const carve::mesh::MeshSet<3>& currMeshSet = *m_meshSetOpenFinal[i];
			convert(currMeshSet, m_polyvectOpenFinal.back());
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

void Space::createSpaceBoundaries(std::vector<Surface>& surfaces, std::vector<SpaceBoundary>& spaceBoundaries,
								  const std::vector<BuildingElement>& elements) {
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

		Surface::MatchResult indices = Surface::findFirstSurfaceMatchIndex(construction.surfaces(), surfaces, dist);
		if(indices.isValid()) {
			int loopCount = 0;
			do {
				++loopCount;
				Surface::IntersectionResult intersectionResult = surfaces[indices.m_spaceSurfaceIndex].intersect2(construction.surfaces()[indices.m_wallSurfaceIndex]);
				for(const Surface& surf : intersectionResult.m_intersections) {
					SpaceBoundary sb(GUID_maker::instance().guid());
					std::string name = m_name + ":" + construction.m_name + " - " +
									   std::to_string(indices.m_spaceSurfaceIndex) + " : " + std::to_string(indices.m_wallSurfaceIndex);
					if(sb.set(name, construction)) {
						sb.m_elementEntityId = construction.m_id;
						sb.fetchGeometry(surf);
						spaceBoundaries.push_back(sb);
					}
				}
				std::vector<Surface> subsurfaces;
				divideSurface(intersectionResult, surfaces, indices.m_spaceSurfaceIndex, subsurfaces);

				indices = Surface::findFirstSurfaceMatchIndex(construction.surfaces(), surfaces, dist);

				if(loopCount > 100) {
					break;
				}
			} while(indices.isValid());
		}
	}
}

static Space::SpaceBoundariesMatching findFirstSurfaceSBMatchIndex(const BuildingElement& construction,
														 const std::vector<SpaceBoundary>& spaceBoundaries, double minDist) {
	const auto uidBegin = construction.m_openingProperties.m_usedInConstructionIds.begin();
	const auto uidEnd = construction.m_openingProperties.m_usedInConstructionIds.end();
	std::vector<std::pair<Space::SpaceBoundariesMatching,double>> distVect;
	Space::SpaceBoundariesMatching result;
	for(size_t wi=0; wi<construction.surfaces().size(); ++wi) {
		for(size_t si=0; si<spaceBoundaries.size(); ++si) {
			if(std::find(uidBegin, uidEnd, spaceBoundaries[si].m_elementEntityId) != uidEnd) {
				double constrThickness = construction.thickness();
				if(constrThickness > minDist) {
					minDist = constrThickness;
				}

				for(size_t sbsi=0; sbsi<spaceBoundaries[si].surfaces().size(); ++sbsi) {
					const Surface& currentSpaceSurf = spaceBoundaries[si].surfaces()[sbsi];
					double dist = currentSpaceSurf.distanceToParallelPlane(construction.surfaces()[wi].planeNormal());
					if(dist < 1e30)
						distVect.push_back(std::make_pair(Space::SpaceBoundariesMatching(wi,si,sbsi),dist));
					if(dist < minDist) {
						if(currentSpaceSurf.isIntersected(construction.surfaces()[wi])) {
							if(!result.isValid())
								result = Space::SpaceBoundariesMatching(wi,si,sbsi);
						}
					}
				}
			}
		}
	}
	return result;
}

static std::vector<Space::SpaceBoundariesMatching> findAllSurfaceSBMatchIndex(const BuildingElement& construction,
														 const std::vector<SpaceBoundary>& spaceBoundaries, double minDist) {
	std::vector<Space::SpaceBoundariesMatching> result;
	std::vector<std::pair<Space::SpaceBoundariesMatching,double>> distVect;
	const auto uidBegin = construction.m_openingProperties.m_usedInConstructionIds.begin();
	const auto uidEnd = construction.m_openingProperties.m_usedInConstructionIds.end();
	for(size_t wi=0; wi<construction.surfaces().size(); ++wi) {
		for(size_t si=0; si<spaceBoundaries.size(); ++si) {
			if(std::find(uidBegin, uidEnd, spaceBoundaries[si].m_elementEntityId) != uidEnd) {
				double constrThickness = construction.thickness();
				if(constrThickness > minDist) {
					minDist = constrThickness;
				}

				for(size_t sbsi=0; sbsi<spaceBoundaries[si].surfaces().size(); ++sbsi) {
					const Surface& currentSpaceSurf = spaceBoundaries[si].surfaces()[sbsi];
					double dist = currentSpaceSurf.distanceToParallelPlane(construction.surfaces()[wi]);
					if(dist < 1e30)
						distVect.push_back(std::make_pair(Space::SpaceBoundariesMatching(wi,si,sbsi),dist));
					if(dist < minDist) {
						if(currentSpaceSurf.isIntersected(construction.surfaces()[wi]))
							result.emplace_back(Space::SpaceBoundariesMatching(wi,si,sbsi));
					}
				}
			}
		}
	}
	return result;
}

static Space::SpaceBoundariesMatching findBestSurfaceSBMatchIndex(const BuildingElement& construction,
														 const std::vector<SpaceBoundary>& spaceBoundaries, double minDist) {
	const auto uidBegin = construction.m_openingProperties.m_usedInConstructionIds.begin();
	const auto uidEnd = construction.m_openingProperties.m_usedInConstructionIds.end();
	Space::SpaceBoundariesMatching result;
	double lastDistance = 1e30;
	for(size_t wi=0; wi<construction.surfaces().size(); ++wi) {
		for(size_t si=0; si<spaceBoundaries.size(); ++si) {
			if(std::find(uidBegin, uidEnd, spaceBoundaries[si].m_elementEntityId) != uidEnd) {
				double constrThickness = construction.thickness();
				if(constrThickness > minDist) {
					minDist = constrThickness;
				}

				for(size_t sbsi=0; sbsi<spaceBoundaries[si].surfaces().size(); ++sbsi) {
					const Surface& currentSpaceSurf = spaceBoundaries[si].surfaces()[sbsi];
					double dist = currentSpaceSurf.distanceToParallelPlane(construction.surfaces()[wi].planeNormal());
					if(dist < minDist) {
						if(currentSpaceSurf.isIntersected(construction.surfaces()[wi])) {
							if(!result.isValid() || dist < lastDistance) {
								result = Space::SpaceBoundariesMatching(wi,si,sbsi);
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

static Space::SpaceBoundariesMatching findBestSurfaceOpeningMatchIndex(const BuildingElement& openingConstruction,
														 const std::vector<Opening>& openings, double minDist) {
	const auto uidBegin = openingConstruction.m_usedFromOpenings.begin();
	const auto uidEnd = openingConstruction.m_usedFromOpenings.end();
	Space::SpaceBoundariesMatching result;
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
					double dist = currentSpaceSurf.distanceToParallelPlane(openingConstruction.surfaces()[wi].planeNormal());
					if(dist < minDist) {
						if(currentSpaceSurf.isIntersected(openingConstruction.surfaces()[wi])) {
							if(!result.isValid() || dist < lastDistance) {
								result = Space::SpaceBoundariesMatching(wi,oi,sbsi);
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
			Space::SpaceBoundariesMatching match = findBestSurfaceOpeningMatchIndex(construction, openings, maxConstructionThickness);
			if(match.isValid()) {
				const Surface& openingSurface = openings[match.m_spaceBoundaryIndex].surfaces()[match.m_spaceBoundarySurfaceIndex];
				const Surface& constrSurface = construction.surfaces()[match.m_constructionSurfaceIndex];
				Surface intersectionResult = openingSurface.intersect(constrSurface);
				if(intersectionResult.m_valid) {
					double dist2 = intersectionResult.distanceToParallelPlane(constrSurface);
					SpaceBoundary sb(GUID_maker::instance().guid());
					std::string name = m_name + ":" + construction.m_name + " - " +
									   std::to_string(match.m_spaceBoundaryIndex) + " : " +
									   std::to_string(match.m_spaceBoundarySurfaceIndex) + " : " +
									   std::to_string(match.m_constructionSurfaceIndex);
					if(sb.set(name, construction)) {
						sb.m_elementEntityId = construction.m_id;
						sb.fetchGeometry(intersectionResult);
						spaceBoundaries.push_back(sb);
					}
				}
			}
		}
	}
}

void Space::createSpaceBoundariesForOpeningsFromSpaceBoundaries(std::vector<SpaceBoundary> &spaceBoundaries,
																const std::vector<BuildingElement> &openingElements,
																const std::vector<Opening> &openings) {
	if(spaceBoundaries.empty())
		return;

	std::vector<SpaceBoundary> constructionSpaceBoundaries(spaceBoundaries);

	for(const auto& construction : openingElements) {
		if(!construction.isSubSurfaceComponent())
			continue;

		int wId = -1;
		if(construction.m_name == "EG-Fenster-7" && m_name == "6") {
			wId = construction.m_id;
		}
		double maxConstructionThickness = 0;
		if(construction.isSubSurfaceComponent() && !construction.m_openingProperties.m_constructionThicknesses.empty()) {
			maxConstructionThickness = *std::max_element(construction.m_openingProperties.m_constructionThicknesses.begin(),
												   construction.m_openingProperties.m_constructionThicknesses.end());
			Space::SpaceBoundariesMatching match = findBestSurfaceSBMatchIndex(construction, constructionSpaceBoundaries, maxConstructionThickness);
//			std::vector<Space::SpaceBoundariesMatching> ivect = findAllSurfaceSBMatchIndex(construction, constructionSpaceBoundaries, maxConstructionThickness);
			if(wId > -1) {
				double minDist = 1e30;
			}
			if(match.isValid()) {
				const Surface& sbSurface = constructionSpaceBoundaries[match.m_spaceBoundaryIndex].surfaces()[match.m_spaceBoundarySurfaceIndex];
				const Surface& constrSurface = construction.surfaces()[match.m_constructionSurfaceIndex];
				Surface intersectionResult = sbSurface.intersect(constrSurface);
				if(intersectionResult.m_valid) {
					double dist2 = intersectionResult.distanceToParallelPlane(constrSurface);
					SpaceBoundary sb(GUID_maker::instance().guid());
					std::string name = m_name + ":" + construction.m_name + " - " +
									   std::to_string(match.m_spaceBoundaryIndex) + " : " +
									   std::to_string(match.m_spaceBoundarySurfaceIndex) + " : " +
									   std::to_string(match.m_constructionSurfaceIndex);
					if(sb.set(name, construction)) {
						sb.m_elementEntityId = construction.m_id;
						sb.fetchGeometry(intersectionResult);
						spaceBoundaries.push_back(sb);
					}
				}
			}
		}
	}
}

void Space::updateSpaceBoundaries(const objectShapeTypeVector_t& shapes,
								  shared_ptr<UnitConverter>& unit_converter,
								  const std::vector<BuildingElement>& constructionElements,
								  const std::vector<BuildingElement>& openingElements,
								  const std::vector<Opening>& openings) {

	bool haveSpaceBoundaries = false;

	if(haveSpaceBoundaries && !m_spaceBoundaries.empty()) {
		for(auto& sb : m_spaceBoundaries) {
			sb.fetchGeometry(unit_converter, m_transformMatrix);
			int type = -1;
			for(const auto& elemType : shapes) {
				for(const auto& elem : elemType.second) {
					if(sb.m_guidRelatedElement == ws2s(elem->m_entity_guid)) {
						type = elemType.first;
						break;
					}
				}
				if(type > -1)
					break;
			}
			int id = -1;
			for(const auto& construction : constructionElements) {
				if(construction.m_guid == sb.m_guidRelatedElement) {
					id = construction.m_id;
				}
			}
			if(id == -1) {
				for(const auto& opening : openingElements) {
					if(opening.m_guid == sb.m_guidRelatedElement) {
						id = opening.m_id;
					}
				}
			}
//			if(id == -1) {
//				for(const auto& other : others) {
//					if(other.m_guid == sb.m_guidRelatedElement) {
//						id = other.m_id;
//					}
//				}
//			}
			if(type > -1) {
				sb.setRelatingElementType(static_cast<ObjectTypes>(type));
				sb.m_elementEntityId = id;
			}
			else {
				std::string name = sb.m_nameRelatedElement;
				m_notes = "Element type not found: " + name;;
			}
		}
	}
	// try to evaluate space boundaries from building element entities
	else {
		std::vector<Surface> surfaceCopy(m_surfacesOrg);
		std::vector<SpaceBoundary> newSpaceBoundaries;

		createSpaceBoundaries(surfaceCopy, newSpaceBoundaries, constructionElements);
		double maxArea = 0;
		double totalArea = 0;
		for(const Surface& surf : surfaceCopy) {
			double ta = surf.area();
			maxArea = std::max(maxArea, ta);
			totalArea += ta;
		}
		double totalOpeningArea = 0;
		for(const BuildingElement& elem : openingElements) {
			totalOpeningArea += elem.openingArea();
		}
		createSpaceBoundariesForOpeningsFromOpenings(newSpaceBoundaries, openingElements, openings);

		m_spaceBoundaries = newSpaceBoundaries;
	}

	updateSurfaces(constructionElements);
}

void Space::updateSurfaces(const std::vector<BuildingElement>& elems) {

	m_surfaces.clear();

	if(!m_spaceBoundaries.empty()) {
		for( const auto& sb : m_spaceBoundaries) {
			if(sb.m_type == SpaceBoundary::CT_ConstructionElement) {
				const std::vector<Surface>& tempSurfVect = sb.surfaces();
				m_surfaces.insert(m_surfaces.end(), tempSurfVect.begin(), tempSurfVect.end());
			}
			else if(sb.m_type == SpaceBoundary::CT_OpeningElement) {
				const std::vector<Surface>& tempSurfVect = sb.surfaces();
				m_subSurfaces.insert(m_subSurfaces.end(), tempSurfVect.begin(), tempSurfVect.end());
			}
		}
		std::vector<OpeningMatching> subSurfaceMatch;
		for(size_t i=0; i<m_surfaces.size(); ++i) {
			const Surface& curSurf = m_surfaces[i];
			int elementIndex = -1;
			for(int eli=0; eli<elems.size(); ++eli) {
				if(curSurf.m_elementEntityId == elems[eli].m_id) {
					elementIndex = eli;
					break;
				}
			}
			double thickness = 0;
			if(elementIndex>-1) {
				const BuildingElement& elem = elems[elementIndex];
				thickness = elem.thickness();
			}

			for(size_t j=0; j<m_subSurfaces.size(); ++j) {
				const Surface& sub = m_subSurfaces[j];

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
				m_surfaces[match.m_surfaceIndex].merge(m_subSurfaces[match.m_subSurfaceIndex]);
		}
		// now we can create the subsurfaces in 2D surface coordinates
		for( const auto& match : subSurfaceMatch) {
			m_surfaces[match.m_surfaceIndex].addSubSurface(m_subSurfaces[match.m_subSurfaceIndex]);
		}

	}
	else {
//		m_matchingWalls.clear();
//		m_matchingWallSurfaceIndices.clear();

//		for(auto& elem : elems) {
//			if(elem.m_type != OT_Wall)
//				continue;

//			try {
//				std::vector<std::pair<int,int>> found = surfaceMatchIndex(elem.surfaces(), m_surfacesOrg, 1e-4);
//				if(found.empty())
//					continue;

//				for(size_t i=0; i<found.size(); ++i) {
//					int spaceIndex = found[i].first;
//					int wallIndex = found[i].second;
//					if(spaceIndex > -1 && wallIndex > -1) {

//						bool res = m_surfacesOrg[spaceIndex].intersect(elem.surfaces()[wallIndex]);
//						if( res) {
//							auto fit = std::find(m_matchingWallSurfaceIndices[spaceIndex].begin(),m_matchingWallSurfaceIndices[spaceIndex].end(),wallIndex);
//							if(fit == m_matchingWallSurfaceIndices[spaceIndex].end()) {
//								m_matchingWalls[spaceIndex].push_back(elem);
//								m_matchingWallSurfaceIndices[spaceIndex].push_back(wallIndex);
//							}
//						}
//					}
//				}
//			}
//			catch (IBK::Exception& e) {
//			}
//		}
	}
}

const meshVector_t& Space::meshVector() const {
	if(!m_meshSetClosedFinal.empty() && !m_meshSetOpenFinal.empty()) {
		throw IBK::Exception("Mesh includes open and closed meshset.", "Space::meshVector");
	}

	if(!m_meshSetClosedFinal.empty())
		return m_meshSetClosedFinal;

	return m_meshSetOpenFinal;
}

const polyVector_t& Space::polyVector() const {
	if(!m_polyvectClosedFinal.empty() && !m_polyvectOpenFinal.empty()) {
		throw IBK::Exception("Mesh includes open and closed meshset.", "Space::meshVector");
	}

	if(!m_polyvectClosedFinal.empty())
		return m_polyvectClosedFinal;

	return m_polyvectOpenFinal;
}

const std::vector<Surface>& Space::surfaces() const {
	return m_surfaces;
}

carve::mesh::Face<3>* Space::face(FaceIndex findex) const {
	if(!m_meshSetClosedFinal.empty() && !m_meshSetOpenFinal.empty()) {
		return nullptr;
	}

	if(!m_meshSetClosedFinal.empty())
		return faceFromMeshset(m_meshSetClosedFinal, findex);

	return faceFromMeshset(m_meshSetOpenFinal, findex);
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
