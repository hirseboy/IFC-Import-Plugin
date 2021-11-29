#ifndef IFCC_SpaceH
#define IFCC_SpaceH

#include <ifcpp/IFC4/include/IfcSpaceTypeEnum.h>
#include <ifcpp/IFC4/include/IfcSpace.h>

#include <IBKMK_Vector3D.h>

#include <tinyxml.h>

#include "IFCC_BuildingElement.h"
#include "IFCC_Helper.h"
#include "IFCC_Surface.h"
#include "IFCC_EntityBase.h"
#include "IFCC_SpaceBoundary.h"
#include "IFCC_Opening.h"

namespace IFCC {

class Space : public EntityBase
{
public:

	enum OpeningMatchingType {
		OMT_SamePoints,
		OMT_WallThicknessIntersection,
		OMT_NoMatching
	};

	struct OpeningMatching {
		OpeningMatching(OpeningMatchingType type, size_t si, size_t subi) :
			m_type(type),
			m_surfaceIndex(si),
			m_subSurfaceIndex(subi)
		{}

		OpeningMatchingType m_type;
		size_t				m_surfaceIndex;
		size_t				m_subSurfaceIndex;
	};

	explicit Space(int id);

	bool set(std::shared_ptr<IfcSpace> ifcSpace);

	void update(std::shared_ptr<ProductShapeData> productShape);

	void updateSpaceBoundaries(const std::map<ObjectTypes,std::vector<shared_ptr<ProductShapeData>>>& shapes,
							   shared_ptr<UnitConverter>& unit_converter,
							   const std::vector<BuildingElement>& constructionElemnts,
							   const std::vector<BuildingElement>& openingElemnts,
							   const std::vector<Opening>& openings);

	void updateSurfaces(const std::vector<BuildingElement>& elems);

	const meshVector_t& meshVector() const;

	const polyVector_t& polyVector() const;

	const std::vector<Surface>& surfaces() const;

	carve::mesh::Face<3>* face(FaceIndex findex) const;

	TiXmlElement * writeXML(TiXmlElement * parent) const;

	std::string									m_longName;
	int											m_numChilds = 0;
	std::vector<SpaceBoundary>					m_spaceBoundaries;
	IfcSpaceTypeEnum::IfcSpaceTypeEnumEnum		m_spaceType;
	double										m_lengthMeasure;
	std::vector<Surface>						m_surfaces;
	std::string									m_notes;

private:

	void transform(std::shared_ptr<ProductShapeData> productShape);

	void fetchGeometry(std::shared_ptr<ProductShapeData> productShape);

	void createSpaceBoundaries(std::vector<Surface>& surfaces, std::vector<SpaceBoundary>& spaceBoundaries,
							   const std::vector<BuildingElement>& elements,
							   const std::vector<Opening>& openings);

	meshVector_t								m_meshSetClosedFinal;
	meshVector_t								m_meshSetOpenFinal;
	polyVector_t								m_polyvectClosedFinal;
	polyVector_t								m_polyvectOpenFinal;
	carve::math::Matrix							m_transformMatrix;
	std::map<int,std::vector<BuildingElement>>	m_matchingWalls;
	std::map<int,std::vector<int>>				m_matchingWallSurfaceIndices;
	std::vector<Surface>						m_surfacesOrg;
	std::vector<Surface>						m_subSurfaces;
};

} // namespace IFCC

#endif // IFCC_SpaceH
