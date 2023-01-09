#ifndef IFCC_OpeningH
#define IFCC_OpeningH


#include <ifcpp/IFC4/include/IfcFeatureElementSubtraction.h>


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
	bool set(std::shared_ptr<IfcFeatureElementSubtraction> ifcElement);

	/*! Get and transform geometry and fill surface vector.
		\param Shape data of a opening.
	*/
	void update(std::shared_ptr<ProductShapeData> productShape);

	/*! Return all surfaces of this opening.*/
	const std::vector<Surface>& surfaces() const;

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

	/*! Map store the surface indices which are connected to a space given by ID.
		Map key is id of space.
		First value is index if space surface.
		Second value is index of opening surface.
	*/
	std::map<int,std::vector<std::pair<size_t,size_t>>>	m_spaceSurfaceConnection;

	/*! Return true if the opening is already connected to a space boundary.*/
	bool hasSpaceBoundary() const;

	/*! Get the vector of opening construction ids connected to this opening.*/
	const std::vector<int>& openingElementIds() const;

private:

	/*! Transfor geometric shape data from local into global coordinate system.
		Will be called from update.
	*/
	void transform(std::shared_ptr<ProductShapeData> productShape);

	/*! Take geometry from shape data and convert it into a list of surfaces.*/
	void fetchGeometry(std::shared_ptr<ProductShapeData> productShape);

	std::string				m_guid;						///< GUID of original IFC object.
	std::vector<Surface>	m_surfaces;
	/*! Vector of ids of opening elements (window or door) which are used from this opening (should only be one).*/
	std::vector<int>		m_openingElementIds;
	/*! Vector of ids of construction elements (wall, roof or slab) which contains this opening (should only be one).*/
	std::vector<int>		m_containedInElementIds;
	/*! Connected space boundary.*/
	std::shared_ptr<SpaceBoundary>	m_spaceBoundary;
};

} // namespace IFCC

#endif // IFCC_OpeningH
