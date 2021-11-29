#ifndef IFCC_SiteH
#define IFCC_SiteH

#include <IBKMK_Vector3D.h>

#include <carve/mesh.hpp>
#include <carve/matrix.hpp>

#include <ifcpp/IFC4/include/IfcSpatialStructureElement.h>
#include <ifcpp/geometry/Carve/GeometryInputData.h>
#include <ifcpp/IFC4/include/IfcBuilding.h>

#include <tinyxml.h>

#include "IFCC_Types.h"
#include "IFCC_Surface.h"
#include "IFCC_Building.h"
#include "IFCC_EntityBase.h"

namespace IFCC {

class Site : public EntityBase
{
public:
	explicit Site(int id);

	bool set(std::shared_ptr<IfcSpatialStructureElement> ifcElement);

	bool set(std::shared_ptr<IfcSpatialStructureElement> ifcElement, std::shared_ptr<ProductShapeData> productShape,
			 const std::map<std::string,shared_ptr<ProductShapeData>>& buildings);

	const meshVector_t& meshVector() const;

	const polyVector_t& polyVector() const;

	const std::vector<Surface>& surfaces() const;

	carve::mesh::Face<3>* face(FaceIndex findex) const;

	TiXmlElement * writeXML(TiXmlElement * parent) const;

	ObjectTypes				m_type;
	std::vector<Building>	m_buildings;

private:

	void transform(std::shared_ptr<ProductShapeData> productShape);

	void fetchGeometry(std::shared_ptr<ProductShapeData> productShape);

	void fetchBuildings(const std::map<std::string,shared_ptr<ProductShapeData>>& buildings);

	meshVector_t			m_meshSetClosedFinal;
	meshVector_t			m_meshSetOpenFinal;
	polyVector_t			m_polyvectClosedFinal;
	polyVector_t			m_polyvectOpenFinal;
	carve::math::Matrix		m_transformMatrix;
	std::vector<Surface>	m_surfaces;

	std::vector<std::shared_ptr<IfcBuilding>>	m_buildingsOriginal;
};

} // namespace IFCC

#endif // IFCC_SiteH
