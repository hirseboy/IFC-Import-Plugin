#ifndef IFCC_TypesH
#define IFCC_TypesH

#include <vector>

#include <IBKMK_Vector3D.h>
#include <IBKMK_Vector2D.h>

#include <carve/mesh.hpp>

#include <ifcpp/geometry/Carve/GeometryInputData.h>

namespace IFCC {

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
