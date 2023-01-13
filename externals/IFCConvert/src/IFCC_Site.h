#ifndef IFCC_SiteH
#define IFCC_SiteH


#include <ifcpp/IFC4/include/IfcSpatialStructureElement.h>
#include <ifcpp/geometry/Carve/GeometryInputData.h>
#include <ifcpp/IFC4/include/IfcBuilding.h>

#include <tinyxml.h>

#include "IFCC_Types.h"
#include "IFCC_Surface.h"
#include "IFCC_Building.h"
#include "IFCC_EntityBase.h"

namespace IFCC {

/*! Class represents a building site. It can contain geometric objects of the surrounding.
	It contains also all buildings with their content (storeys, spaces etc.).
*/
class Site : public EntityBase
{
public:
	/*! Standard constructor.
		\param id Unique id for using in project.
	*/
	explicit Site(int id);

	/*! Initialise site from a IFC site object.
		It set a name and fills the original building vector. Then geometry will be transformed and converted into surfaces.
		Also the building vector is filled from the original building vector. All building objects are not fully initalized.
		Call Building::update() on all objects.
		It calls transform, fetchGeometry and fetchBuildings.
		\param ifcElement IFC object for a site. It uses the base class of IfcBuildingStorey.
		\param productShape Shape data of site
		\param buildings Shape data of buildings
	*/
	bool set(std::shared_ptr<IfcSpatialStructureElement> ifcElement, std::shared_ptr<ProductShapeData> productShape,
			 const std::map<std::string,shared_ptr<ProductShapeData>>& buildings);

	/*! Return the internal surface vector.*/
	const std::vector<Surface>& surfaces() const;

	/*! Return a vector of all space boundaries in project.*/
	std::vector<std::shared_ptr<SpaceBoundary>> allSpaceBoundaries() const;

	/*! Return a vector of all spaces in project.*/
	std::vector<std::shared_ptr<Space>> allSpaces() const;

//	/*! Return a vector of all space connection in project.*/
//	std::vector<Space::SurfaceConnectionVectors> allSurfaceConnectionVectors() const;

	/*! Write the site in vicus xml format including buildings.*/
	TiXmlElement * writeXML(TiXmlElement * parent, bool flipPolygons) const;

	std::vector<std::shared_ptr<Building>>	m_buildings;	///< vector of pointers to building objects

private:

	/*! Initialise site from a IFC site object.
		It set a name and fills the original building vector.
		\param ifcElement IFC object for a site. It uses the base class of IfcBuildingStorey.
	*/
	bool set(std::shared_ptr<IfcSpatialStructureElement> ifcElement);

	/*! Transforms the site geometry by using transformation matrix from productShape.
		It transforms all coordinates from local system into global system.
	*/
	void transform(std::shared_ptr<ProductShapeData> productShape);

	/*! Get the geometry from the product shape.
		It fills the surface vector m_surfaces.
	*/
	void fetchGeometry(std::shared_ptr<ProductShapeData> productShape);

	/*! Fill the buildings vector from original buildings vector and given building shapes.
		\param buildings Shapes for all buildings in project.
	*/
	void fetchBuildings(const std::map<std::string,shared_ptr<ProductShapeData>>& buildings);

	std::vector<Surface>						m_surfaces;				///< Vector of all surfaces

	std::vector<std::shared_ptr<IfcBuilding>>	m_buildingsOriginal;	///< Vector of original IFC building objects
};

} // namespace IFCC

#endif // IFCC_SiteH
