#ifndef IFCC_BuildingElementH
#define IFCC_BuildingElementH

#include <IBKMK_Vector3D.h>

#include <carve/mesh.hpp>
#include <carve/matrix.hpp>

#include <ifcpp/IFC4/include/IfcElement.h>
#include <ifcpp/IFC4/include/IfcOpeningElement.h>
#include <ifcpp/IFC4/include/IfcWindow.h>
#include <ifcpp/IFC4/include/IfcWindowTypeEnum.h>
#include <ifcpp/IFC4/include/IfcWindowTypePartitioningEnum.h>
#include <ifcpp/geometry/Carve/GeometryInputData.h>


#include "IFCC_Types.h"
#include "IFCC_Surface.h"
#include "IFCC_Opening.h"
#include "IFCC_EntityBase.h"

namespace IFCC {

class BuildingElement : public EntityBase
{
public:
	struct OpeningProperties {
		bool						m_isWindow = false;
		bool						m_isDoor = false;
		int							m_id;
		std::vector<int>			m_usedInConstructionIds;
		double						m_windowHeight = 0;
		double						m_windowWidth = 0;
		std::string					m_windowUserDefinedPartitionType;
		std::vector<std::string>	m_windowConstructionTypes;

		std::string					m_name;
		IfcWindowTypeEnum::IfcWindowTypeEnumEnum m_windowType = IfcWindowTypeEnum::ENUM_NOTDEFINED;
		IfcWindowTypePartitioningEnum::IfcWindowTypePartitioningEnumEnum m_windowPartitionType =
				IfcWindowTypePartitioningEnum::ENUM_NOTDEFINED;
	};

	explicit BuildingElement(int id);

	bool set(std::shared_ptr<IfcElement> ifcElement, ObjectTypes type);

	void update(std::shared_ptr<ProductShapeData> productShape, std::vector<Opening>& openings);

	const meshVector_t& meshVector() const;

	const polyVector_t& polyVector() const;

	const std::vector<Surface>& surfaces() const;

	carve::mesh::Face<3>* face(FaceIndex findex) const;

	double	thickness() const;

	ObjectTypes									m_type;
	bool										m_surfaceComponent;
	bool										m_subSurfaceComponent;
	std::vector<std::pair<double,std::string>>	m_materialLayers;
	std::vector<Surface>						m_surfaces;
	std::vector<int>							m_usedFromOpenings;
	std::vector<int>							m_containedOpenings;
	int											m_constructionId;
	OpeningProperties							m_openingProperties;
	std::vector<std::pair<int,int>>				m_parallelSurfaces;

private:
	void findSurfacePairs();

	void transform(std::shared_ptr<ProductShapeData> productShape);

	void fetchGeometry(std::shared_ptr<ProductShapeData> productShape);

	void fetchOpenings(std::vector<Opening>& openings);

	meshVector_t			m_meshSetClosedFinal;
	meshVector_t			m_meshSetOpenFinal;
	polyVector_t			m_polyvectClosedFinal;
	polyVector_t			m_polyvectOpenFinal;
	carve::math::Matrix		m_transformMatrix;
	std::vector<std::shared_ptr<IfcOpeningElement>>				m_isUsedFromOpeningsOriginal;
	std::vector<std::shared_ptr<IfcFeatureElementSubtraction>>	m_containedOpeningsOriginal;
};

} // namespace IFCC

#endif // IFCC_BuildingElementH
