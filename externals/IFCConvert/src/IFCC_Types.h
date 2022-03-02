#ifndef IFCC_TypesH
#define IFCC_TypesH

#include <vector>

#include <IBKMK_Vector3D.h>
#include <IBKMK_Vector2D.h>

#include <carve/mesh.hpp>

#include <ifcpp/geometry/Carve/GeometryInputData.h>


namespace IFCC {

class BuildingElement;

/*! Struct collects all types of building elements.*/
struct BuildingElementsCollector {
	/*! Primary building elements which are part of the construction and can contain openings.
		This can be: Wall, Roof, Slab
	*/
	std::vector<std::shared_ptr<BuildingElement>>		m_constructionElements;

	/*! Secondary building elements which are part of the construction and can contain openings.
		This can be: Beam, Column, Covering, Footing, CurtainWall
	*/
	std::vector<std::shared_ptr<BuildingElement>>		m_constructionSimilarElements;

	/*! All building elements which can be a opening.
		This can be: Window, Door
	*/
	std::vector<std::shared_ptr<BuildingElement>>		m_openingElements;

	/*! All other building elements.*/
	std::vector<std::shared_ptr<BuildingElement>>		m_otherElements;

	/*! Building elements without surfaces.*/
	std::vector<std::shared_ptr<BuildingElement>>		m_elementsWithoutSurfaces;

	void clear() {
		m_constructionElements.clear();
		m_constructionSimilarElements.clear();
		m_openingElements.clear();
		m_otherElements.clear();
		m_elementsWithoutSurfaces.clear();
	}

	std::vector<std::shared_ptr<BuildingElement>> allConstructionElements() const {
		std::vector<std::shared_ptr<BuildingElement>> constructionElements(m_constructionElements);
		constructionElements.insert(constructionElements.begin(), m_constructionSimilarElements.begin(),
									m_constructionSimilarElements.end());
		return constructionElements;
	}
};

/*! Types of building elements.*/
enum ObjectTypes {
	OT_Beam,
	OT_Chimney,
	OT_Column,
	OT_Covering,
	OT_CurtainWall,
	OT_Door,
	OT_Footing,
	OT_Member,
	OT_Pile,
	OT_Plate,
	OT_Railing,
	OT_Ramp,
	OT_RampFlight,
	OT_Roof,
	OT_ShadingDevice,
	OT_Slab,
	OT_Stair,
	OT_StairFlight,
	OT_Wall,
	OT_Window,
	OT_CivilElement,
	OT_DistributionElement,
	OT_ElementAssembly,
	OT_ElementComponent,
	OT_FeatureElement,
	OT_FurnishingElement,
	OT_GeographicalElement,
	OT_TransportElement,
	OT_VirtualElement,
	OT_All,
	OT_None
};


/*! Basic type of the object.*/
enum BasicType {
	BT_Real,			///< A real and existing object
	BT_Virtual,			///< A virtual object (e.g. space divider)
	BT_Missing,			///< A missing object for existing surfaces which doesn't have real connection
	NUM_BT
};


// some type definitions

using polyVector_t = std::vector<std::vector<std::vector<std::vector<IBKMK::Vector3D>>>>;
using meshVector_t = std::vector<std::shared_ptr<carve::mesh::MeshSet<3> > >;
using polygon3D_t = std::vector<IBKMK::Vector3D>;
using polygon2D_t = std::vector<IBKMK::Vector2D>;
using conversionVectors2D_t = std::pair<std::vector<IBKMK::Vector2D>,std::vector<IBKMK::Vector2D>>;

using objectShapeTypeVector_t = std::map<ObjectTypes,std::vector<shared_ptr<ProductShapeData>>>;
using objectShapeGUIDMap_t = std::map<std::string,shared_ptr<ProductShapeData>>;

} // end namespace

#endif // IFCC_TypesH
