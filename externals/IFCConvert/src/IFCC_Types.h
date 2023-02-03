#ifndef IFCC_TypesH
#define IFCC_TypesH

#include <vector>

#include <IBKMK_Vector3D.h>
#include <IBKMK_Vector2D.h>

#include <carve/mesh.hpp>

#include <ifcpp/geometry/Carve/GeometryInputData.h>


namespace IFCC {

class BuildingElement;


/*! Types of building elements.*/
enum BuildingElementTypes {
	BET_Beam,
	BET_Chimney,
	BET_Column,
	BET_Covering,
	BET_CurtainWall,
	BET_Door,
	BET_Footing,
	BET_Member,
	BET_Pile,
	BET_Plate,
	BET_Railing,
	BET_Ramp,
	BET_RampFlight,
	BET_Roof,
	BET_ShadingDevice,
	BET_Slab,
	BET_Stair,
	BET_StairFlight,
	BET_Wall,
	BET_Window,
	BET_CivilElement,
	BET_DistributionElement,
	BET_ElementAssembly,
	BET_ElementComponent,
	BET_FeatureElement,
	BET_FurnishingElement,
	BET_GeographicalElement,
	BET_TransportElement,
	BET_VirtualElement,
	BET_All,
	BET_None
};


/*! Basic type of the object.*/
enum BasicType {
	BT_Real,			///< A real and existing object
	BT_Virtual,			///< A virtual object (e.g. space divider)
	BT_Missing,			///< A missing object for existing surfaces which doesn't have real connection
	NUM_BT
};

enum ObjectType {
	OT_BuildingElement,
	OT_Opening,
	OT_Space,
	OT_SpaceBoundary,
	OT_Site,
	OT_Building,
	OT_Storey,
	OT_Material,
	OT_Construction,
	OT_Window,
	OT_Glazing,
	OT_Component,
	OT_SubComponent,
	OT_Instance,
	OT_GeometryConvert,
	OT_Unknown
};

struct ConvertError {
	ConvertError() :
		m_objectType(OT_Unknown),
		m_objectID(-1)
	{}
	ConvertError(ObjectType type, int id, std::string txt) :
		m_objectType(type),
		m_objectID(id),
		m_errorText(txt)
	{}

	ObjectType	m_objectType;
	int			m_objectID;
	std::string	m_errorText;
};


// some type definitions

using polyVector_t = std::vector<std::vector<std::vector<std::vector<IBKMK::Vector3D>>>>;
using meshVector_t = std::vector<std::shared_ptr<carve::mesh::MeshSet<3> > >;
using polygon3D_t = std::vector<IBKMK::Vector3D>;
using polygon2D_t = std::vector<IBKMK::Vector2D>;
using conversionVectors2D_t = std::pair<std::vector<IBKMK::Vector2D>,std::vector<IBKMK::Vector2D>>;

using objectShapeTypeVector_t = std::map<BuildingElementTypes,std::vector<shared_ptr<ProductShapeData>>>;
using objectShapeGUIDMap_t = std::map<std::string,shared_ptr<ProductShapeData>>;

} // end namespace

#endif // IFCC_TypesH
