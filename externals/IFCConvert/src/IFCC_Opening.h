#ifndef IFCC_OpeningH
#define IFCC_OpeningH


#include <ifcpp/IFC4X3/include/IfcFeatureElementSubtraction.h>


//#include "IFCC_SpaceBoundary.h"
#include "IFCC_Surface.h"
#include "IFCC_EntityBase.h"

namespace IFCC {

class SpaceBoundary;

/*! Class represents a opening.
	This mainly a geometric object which have connections to a construction element and an opening element.
*/
class Opening : public EntityBase
{
public:
	/*! Standard constructor.
		\param id Unique id for using in project.
	*/
	explicit Opening(int id);

	/*! Initialise opening from a IFC element object.
		It set a name and guid.
		\param ifcElement Original IFC element which can represent a opening.
	*/
	bool set(std::shared_ptr<IFC4X3::IfcFeatureElementSubtraction> ifcElement);

	/*! Get and transform geometry and fill surface vector.
		\param Shape data of a opening.
	*/
	void update(std::shared_ptr<ProductShapeData> productShape, std::vector<ConvertError>& errors);

	/*! Return all surfaces of this opening.*/
	const std::vector<Surface>& surfaces() const;

	/*! Return all surfaces created by CSG intersection with construction of this opening.*/
	const std::vector<Surface>& surfacesCSG() const;

	/*! Return GUID of original IFC object.*/
	std::string guid() const;

	/*! Add the given id to the opening element ids vector.
		\param id Id of an opening element (window or door) which is connected to this opening
	*/
	void addOpeningElementId(int id);

	/*! Add the given id to the containing element ids vector.
		\param id Id of a construction element (wall, roof or slab) which is connected to this opening
	*/
	void addContainingElementId(int id);

	/*! Add all ids from the containing element ids vector to the end of the given one.*/
	void insertContainingElementId(std::vector<int>& other) const;

	/*! Set the connected space boundary.*/
	void setSpaceBoundary(std::shared_ptr<SpaceBoundary> sb);

	/*! Return true if the opening is already connected to a space boundary.*/
	bool hasSpaceBoundary() const;

	/*! Get the vector of opening construction ids connected to this opening.*/
	const std::vector<int>& openingElementIds() const;

	bool isConnectedToOpeningElement() const {
		return !m_openingElementIds.empty();
	}

	/*! Try to find surfaces are parallel or intersecting building element surfaces.
	 *  Set a surface type based on the results.
	*/
	void checkSurfaceType(const BuildingElement& element);

	/*! Create a 3D intersection with opening and given building element.
	 *  Store the resulting surfaces in m_surfacesCSG.
	*/
	void createCSGSurfaces(const BuildingElement& element);

	/*! Map store the surface indices which are connected to a space given by ID.
		Map key is id of space.
		First value is index if space surface.
		Second value is index of opening surface.
	*/
	std::map<int,std::vector<std::pair<size_t,size_t>>>	m_spaceSurfaceConnection;

private:

	/*! Transfor geometric shape data from local into global coordinate system.
		Will be called from update.
	*/
	void transform(std::shared_ptr<ProductShapeData> productShape);

	/*! Take geometry from shape data and convert it into a list of surfaces.*/
	void fetchGeometry(std::shared_ptr<ProductShapeData> productShape, std::vector<ConvertError>& errors);

	std::string						m_guid;						///< GUID of original IFC object.

	std::vector<Surface>			m_surfaces;

	std::vector<Surface>			m_surfacesCSG;

	/*! Vector of ids of opening elements (window or door) which are used from this opening (should only be one).*/
	std::vector<int>				m_openingElementIds;

	/*! Vector of ids of construction elements (wall, roof or slab) which contains this opening (should only be one).*/
	std::vector<int>				m_containedInElementIds;

	/*! Connected space boundary.*/
	std::shared_ptr<SpaceBoundary>	m_spaceBoundary;

	/*! Original 3D mesh from conversion IFC to carve.*/
	meshVector_t					m_originalMesh;
};

} // namespace IFCC

#endif // IFCC_OpeningH
