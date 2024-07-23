#ifndef IFCC_RepresentationHelperH
#define IFCC_RepresentationHelperH

#include "IFCC_GeometryInputData.h"

#include "IFCC_Types.h"
#include "IFCC_Surface.h"


namespace IFCC {


struct RepresentationStructure {
	std::shared_ptr<RepresentationData> m_bodyRep;
	std::shared_ptr<RepresentationData> m_referenceRep;
	std::shared_ptr<RepresentationData> m_surfaceRep;
	std::shared_ptr<RepresentationData> m_profileRep;
	int									m_bodyRepCount = 0;
	int									m_referenceRepCount = 0;
	int									m_surfaceRepCount = 0;
	int									m_profileRepCount = 0;
	int									m_otherRepCount = 0;
};

void surfacesFromMeshSets(std::vector<shared_ptr<carve::mesh::MeshSet<3> > >& meshsets, std::vector<Surface>& surfaces);

RepresentationStructure getRepresentationStructure(std::shared_ptr<ProductShapeData> productShape);

meshVector_t finalMeshSet(std::shared_ptr<RepresentationData> representation, std::vector<ConvertError>& errors, std::vector<Surface>& surfaces,
						  ObjectType objectType, int objectId);

meshVector_t surfacesFromRepresentation(std::shared_ptr<ProductShapeData> productShape, std::vector<Surface>& surfaces,
								std::vector<ConvertError>& errors, ObjectType objectType, int objectId);

meshVector_t meshSetsFromBodyRepresentation(std::shared_ptr<ProductShapeData> productShape);

} // namespace IFCC

#endif // IFCC_RepresentationHelperH
