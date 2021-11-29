#ifndef IFCC_OpeningH
#define IFCC_OpeningH

#include <IBKMK_Vector3D.h>

#include <carve/mesh.hpp>
#include <carve/matrix.hpp>

#include <ifcpp/IFC4/include/IfcFeatureElementSubtraction.h>
#include <ifcpp/geometry/Carve/GeometryInputData.h>


#include "IFCC_Types.h"
#include "IFCC_Surface.h"
#include "IFCC_EntityBase.h"

namespace IFCC {

class Opening : public EntityBase
{
public:
	explicit Opening(int id);

	bool set(std::shared_ptr<IfcFeatureElementSubtraction> ifcElement);

	void update(std::shared_ptr<ProductShapeData> productShape);

//	const meshVector_t& meshVector() const;

	const polyVector_t& polyVector() const;

	const std::vector<Surface>& surfaces() const;

	carve::mesh::Face<3>* face(FaceIndex findex) const;

	std::string				m_guid;
	std::vector<int>		m_openingElementIds;
	std::vector<int>		m_containedInElementIds;

private:

	void transform(std::shared_ptr<ProductShapeData> productShape);

	void fetchGeometry(std::shared_ptr<ProductShapeData> productShape);

	meshVector_t			m_meshSetClosedFinal;
	meshVector_t			m_meshSetOpenFinal;
	polyVector_t			m_polyvectClosedFinal;
	polyVector_t			m_polyvectOpenFinal;
	carve::math::Matrix		m_transformMatrix;
	std::vector<Surface>	m_surfaces;
};

} // namespace IFCC

#endif // IFCC_OpeningH
