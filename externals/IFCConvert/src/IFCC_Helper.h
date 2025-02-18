#ifndef IFCC_HelperH
#define IFCC_HelperH

#include <vector>

#include <IBKMK_Polygon3D.h>

#include <IBK_Path.h>

#include <ifcpp/IFC4X3/include/IfcObjectDefinition.h>
#include <ifcpp/IFC4X3/include/IfcLabel.h>
#include <ifcpp/IFC4X3/include/IfcText.h>
#include <ifcpp/IFC4X3/include/IfcIdentifier.h>
#include <ifcpp/IFC4X3/include/IfcConnectionGeometry.h>

#include <ifcpp/model/UnitConverter.h>

#include "IFCC_Types.h"

class TiXmlElement;

namespace IFCC {

class Surface;

/*! Class for creating project unique GUIDs starting with number 1.*/
class GUID_maker {
public:
	/*! Return only instance (Singleton).*/
	static GUID_maker& instance() {
		static GUID_maker self;
		return self;
	}
	/*! Return unique GUID.*/
	int guid() {
		return m_guid++;
	}

private:
	static int m_guid;	///< Current guid
};

// check functions

/*! Return a set with all element types directly usable as construction.
	This construction can have openings.
*/
inline QSet<BuildingElementTypes> constructionTypes() {
	QSet<BuildingElementTypes> types{BET_Wall,BET_Roof,BET_Slab,BET_Covering};
	return types;
}

inline bool isConstructionType(BuildingElementTypes type) {
	QSet<BuildingElementTypes> types = constructionTypes();
	return types.contains(type);
}

/*! Return a set with all element types usable as construction or helper constructions.
	This construction normally don't contain openings.
*/
inline QSet<BuildingElementTypes> constructionSimilarTypes() {
	QSet<BuildingElementTypes> types;
	types << BET_Beam << BET_Column << BET_CurtainWall << BET_Footing << BET_BuildingElementPart;
	return types;
}

inline bool isConstructionSimilarType(BuildingElementTypes type) {
	QSet<BuildingElementTypes> types = constructionSimilarTypes();
	return types.contains(type);
}

/*! Return building elements usable for openings.*/
inline QSet<BuildingElementTypes> openingTypes() {
	QSet<BuildingElementTypes> types{BET_Window,BET_Door};
	return types;
}

inline bool isOpeningType(BuildingElementTypes type) {
	QSet<BuildingElementTypes> types = openingTypes();
	return types.contains(type);
}

// conversion functions

/*! Convert wide string (Unicode) into UTF8 string.*/
std::string ws2s(const std::wstring& wstr);

/*! Return UTF8 version of label text from IfcLabel.*/
std::string label2s(const std::shared_ptr<IFC4X3::IfcLabel>& label);

/*! Return UTF8 version of text from IfcText.*/
std::string text2s(const std::shared_ptr<IFC4X3::IfcText>& text);

/*! Return UTF8 version of text from IfcText.*/
std::string name2s(const std::shared_ptr<IFC4X3::IfcIdentifier>& text);

/*! Return UTF8 version of the guid of the given IFC object.*/
std::string guidFromObject(IFC4X3::IfcRoot* object);

/*! Convert the given meshSet into vectors of polygons.
	A mesh set can contain several meshes which can contain several faces. Each face will be converted to a polygon.
	A polygon is represented by a vector of 3D vectors.
	\param meshSet Carve mesh set.
	\param polyvect vector of vector of polygons.
*/
void convert(const carve::mesh::MeshSet<3>& meshSet, std::vector<std::vector<std::vector<IBKMK::Vector3D>>>&  polyvect);

// debug helper

/*! Dump the content of the given carve meshSet into a file.
	Multiple calls of this function with the same filename will extend the file.
	\param meshSet Carve mesh set.
	\param name Name of the the meshset.
	\param filename Name of dump file
*/
void meshDump(const carve::mesh::MeshSet<3>& meshSet, const std::string& name, const IBK::Path& filename);

/*! Dump the content of the given carve face into a file.
	Multiple calls of this function with the same filename will extend the file.
	\param face Carve face.
	\param name Name of the the meshset.
	\param filename Name of dump file
*/
void faceDump(carve::mesh::Face<3>* face, const std::string& name, const IBK::Path& filename);

// mesh helper

/*! Try to simplify the given meshSets.
	Merging coplanar faces will always be perfomed. This can remove triangulation. Here a eps angle of 0.1 DEG will be used.
	\param removeLowVolume If true additionally a removing of very small volumes (<1e-8) will be performed.
*/
void simplifyMesh(meshVector_t& meshVector, bool removeLowVolume);

/*! Check if both values are nearly equal.
	It will be done by checking near equality of each coordinate.
*/
bool nearEqual(double v1, double v2, double eps);

/*! Check if the given value is near zero.*/
bool nearZero(double val, double eps);

/*! Check if both vectors are nearly equal.
	It will be done by checking near equality of each coordinate.
*/
bool nearEqual(const IBKMK::Vector3D& v1, const IBKMK::Vector3D& v2, double eps);

/*! Return the area of the given polygon.*/
double areaPolygon(const std::vector<IBKMK::Vector3D>& poly);

/*! Return the signed area of the given polygon.
	The sign represents the rotation type.
*/
double areaSignedPolygon(const std::vector<IBKMK::Vector2D>& poly);

/*! Return the signed area of the given polygon.
	The sign represents the rotation type.
*/
double areaSignedPolygon(const std::vector<IBKMK::Vector3D>& poly);

/*! Return a string for the given object type.*/
std::string objectTypeToString(BuildingElementTypes type);

/*! Return a string for the given object type.*/
std::string objectTypeToString(ObjectType type);

/*! Evaluate object type of given object definition.*/
BuildingElementTypes getObjectType(const std::shared_ptr<IFC4X3::IfcObjectDefinition>& od);

/*! Convert geometry given by IfcConnectionGeometry object into a polyVector.
	\param connectionGeometry Geometry object
	\param unit_converter Unit convert for geometry conversions
	\param spaceTransformation Space transformation matrix for current object
	\param objectId Id of the object. Only used for error messages.
	\param errors Vector of errors while conversion
	\return Vector of created polylines or empty in case of errors.
*/
polyVector_t polylinesFromConnectionGeometry(std::shared_ptr<IFC4X3::IfcConnectionGeometry> connectionGeometry,
										  shared_ptr<UnitConverter>& unit_converter,
										  const carve::math::Matrix& spaceTransformation,
										  int objectId,
										  std::vector<ConvertError>& errors);

bool isIntersected(carve::mesh::MeshSet<3>* a, carve::mesh::MeshSet<3>* b);

std::string dumpSurfaces(const std::vector<IFCC::Surface>& surfaces);

} // end namespace

#endif // IFCC_HelperH
