#include "IFCC_RepresentationHelper.h"

namespace IFCC {


void surfacesFromMeshSets(std::vector<shared_ptr<carve::mesh::MeshSet<3> > >& meshsets, std::vector<Surface>& surfaces) {
	if(meshsets.empty())
		return;

	// try to simplify meshes by merging all coplanar faces
	simplifyMesh(meshsets, false);
	polyVector_t polyvectFinal;
	int msCount = meshsets.size();
	for(int i=0; i<msCount; ++i) {
		polyvectFinal.push_back(std::vector<std::vector<std::vector<IBKMK::Vector3D>>>());
		const carve::mesh::MeshSet<3>& currMeshSet = *meshsets[i];
		convert(currMeshSet, polyvectFinal.back());
		// get surfaces
		for(size_t mi=0; mi<currMeshSet.meshes.size(); ++mi) {
			for(size_t fi =0; fi<currMeshSet.meshes[mi]->faces.size(); ++fi) {
				if(currMeshSet.meshes[mi]->faces[fi] != nullptr && currMeshSet.meshes[mi]->faces[fi]->edge != nullptr)
					surfaces.emplace_back(Surface(currMeshSet.meshes[mi]->faces[fi]));
			}
		}
	}
}

RepresentationStructure getRepresentationStructure(std::shared_ptr<ProductShapeData> productShape) {
	RepresentationStructure res;

	int repCount = productShape->m_vec_representations.size();
	std::shared_ptr<RepresentationData> currentRep;
	for(int repi = 0; repi<repCount; ++repi) {
		currentRep = productShape->m_vec_representations[repi];
		if(currentRep->m_representation_identifier == "Body") {
			// 3D Body representation, e.g. as wireframe, surface, or solid model, of an element
			res.m_bodyRep = currentRep;
			++res.m_bodyRepCount;
		}
		if(currentRep->m_representation_identifier == "Reference") {
			// 	3D representation that is not part of the Body representation. This is used, e.g., for opening geometries, if there are to be excluded from an implicit Boolean operation.
			res.m_referenceRep = currentRep;
			++res.m_referenceRepCount;
		}
		if(currentRep->m_representation_identifier == "Surface") {
			// 3D Surface representation, e.g. of an analytical surface, of an elementplane)
			res.m_surfaceRep = currentRep;
			++res.m_surfaceRepCount;
		}
		if(currentRep->m_representation_identifier == "Profile") {
			// 	3D line representation of a profile being planar, e.g. used for door and window outlines
			res.m_profileRep = currentRep;
			++res.m_profileRepCount;
		}
		if(currentRep->m_representation_identifier == "CoG") {
			// Point to identify the center of gravity of an element. This value can be used for validation purposes.
			++res.m_otherRepCount;
		}
		if(currentRep->m_representation_identifier == "Box") {
			// 	Bounding box as simplified 3D box geometry of an element
			++res.m_otherRepCount;
		}
		if(currentRep->m_representation_identifier == "Annotation") {
			// 2D annotations not representing elements
			++res.m_otherRepCount;
		}
		if(currentRep->m_representation_identifier == "Axis") {
			// 	2D or 3D Axis, or single line, representation of an element
			++res.m_otherRepCount;
		}
		if(currentRep->m_representation_identifier == "FootPrint") {
			// 2D Foot print, or double line, representation of an element, projected to ground view
			++res.m_otherRepCount;
		}
		if(currentRep->m_representation_identifier == "Clearance") {
			// 3D clearance volume of the element. Such clearance region indicates space that should not intersect with the 'Body' representation of other elements, though may intersect with the 'Clearance' representation of other elements
			++res.m_otherRepCount;
		}
		if(currentRep->m_representation_identifier == "Lighting") {
			// Representation of emitting light as a light source within a shape representation
			++res.m_otherRepCount;
		}
	}
	return res;
}

meshVector_t finalMeshSet(std::shared_ptr<RepresentationData> representation, std::vector<ConvertError>& errors, std::vector<Surface>& surfaces,
						  ObjectType objectType, int objectId) {
	meshVector_t meshSetClosedFinal;
	meshVector_t meshSetOpenFinal;
	for(const auto& shapeData : representation->m_vec_item_data) {
		const std::vector<shared_ptr<carve::mesh::MeshSet<3> > >& mc = shapeData->m_meshsets;
		if(!mc.empty()) {
			for(auto mSet : mc)
				if(mSet.get() != nullptr) {
					meshSetClosedFinal.push_back(mSet);
				}
				else {
					errors.push_back({objectType, objectId, "Non valid mesh set found."});
				}
		}
		const std::vector<shared_ptr<carve::mesh::MeshSet<3> > >& mo = shapeData->m_meshsets_open;
		if(!mo.empty()) {
			for(auto mSet : mo)
				if(mSet.get() != nullptr) {
					meshSetOpenFinal.push_back(mSet);
				}
				else {
					errors.push_back({objectType, objectId, "Non valid mesh set found."});
				}
		}
	}

	surfacesFromMeshSets(meshSetClosedFinal, surfaces);
	surfacesFromMeshSets(meshSetOpenFinal, surfaces);
	if(!meshSetClosedFinal.empty())
		return meshSetClosedFinal;

	return meshSetOpenFinal;
}

meshVector_t surfacesFromRepresentation(std::shared_ptr<ProductShapeData> productShape, std::vector<Surface>& surfaces,
								std::vector<ConvertError>& errors, ObjectType objectType, int objectId) {

	surfaces.clear();
	meshVector_t meshSet;

	RepresentationStructure repStruct = getRepresentationStructure(productShape);

	if(repStruct.m_bodyRep) {
		if(repStruct.m_bodyRepCount > 1) {
			errors.push_back({objectType, objectId, "more than one geometric representaion of type 'body' found"});
		}

		meshSet = finalMeshSet(repStruct.m_bodyRep, errors, surfaces, objectType, objectId);
	}

	if(repStruct.m_referenceRep && surfaces.empty()) {
		if(repStruct.m_referenceRepCount > 1) {
			errors.push_back({objectType, objectId, "more than one geometric representaion of type 'reference' found"});
		}
		meshSet = finalMeshSet(repStruct.m_referenceRep, errors, surfaces, objectType, objectId);
	}

	if(repStruct.m_surfaceRep && surfaces.empty()) {
		///< \todo Implement
		if(!repStruct.m_bodyRep)
			errors.push_back({objectType, objectId, "Geometric representation of type 'surface' cannot be evaluated."});
	}

	if(repStruct.m_profileRep && surfaces.empty()) {
		///< \todo Implement
		if(!repStruct.m_bodyRep)
			errors.push_back({objectType, objectId, "Geometric representation of type 'profile' cannot be evaluated."});
	}

	// check surfaces
	if(objectType == OT_Space) {
		std::vector<Surface> addedSurfaces;
		for(size_t i=0; i<surfaces.size(); ++i) {
			Surface& surf = surfaces[i];
			if(!surf.hasSimplePolygon()) {
				std::vector<Surface> res = surf.innerIntersection();
				if(!res.empty()) {
					surf = res.front();
					if(res.size()>1) {
						addedSurfaces.insert(addedSurfaces.end(), res.begin()+1, res.end());
					}
				}
				errors.push_back({objectType, objectId, "Created surface is not valid: " + std::to_string(i)});
			}
		}
		if(!addedSurfaces.empty())
			surfaces.insert(surfaces.end(), addedSurfaces.begin(), addedSurfaces.end());
	}

	return meshSet;
}

static std::shared_ptr<RepresentationData> firstBodyRep(std::shared_ptr<ProductShapeData> productShape) {
	for(auto rep : productShape->m_vec_representations) {
		if(rep->m_representation_identifier == "Body") {
			return rep;
		}
	}

	return std::shared_ptr<RepresentationData>();
}

meshVector_t meshSetsFromBodyRepresentation(std::shared_ptr<ProductShapeData> productShape) {

	std::shared_ptr<RepresentationData> bodyRep = firstBodyRep(productShape);
	if(!bodyRep)
		return meshVector_t();

	meshVector_t meshSetClosedFinal;
	meshVector_t meshSetOpenFinal;
	for(const auto& shapeData : bodyRep->m_vec_item_data) {
		const std::vector<shared_ptr<carve::mesh::MeshSet<3> > >& mc = shapeData->m_meshsets;
		if(!mc.empty()) {
			meshSetClosedFinal.insert(meshSetClosedFinal.begin(), mc.begin(), mc.end());
		}
		const std::vector<shared_ptr<carve::mesh::MeshSet<3> > >& mo = shapeData->m_meshsets_open;
		if(!mo.empty()) {
			meshSetOpenFinal.insert(meshSetOpenFinal.begin(), mo.begin(), mo.end());
		}
	}

	if(!meshSetClosedFinal.empty()) {
		return meshSetClosedFinal;
	}

	if(!meshSetOpenFinal.empty()) {
		return meshSetOpenFinal;
	}
	return meshVector_t();
}

} // namespace IFCC
