#ifndef IFCC_HelperH
#define IFCC_HelperH

#include <vector>

#include <IBKMK_Polygon3D.h>

#include <IBK_Path.h>

#include <ifcpp/IFC4/include/IfcObjectDefinition.h>
#include <ifcpp/IFC4/include/IfcLabel.h>
#include <ifcpp/IFC4/include/IfcText.h>
#include <ifcpp/IFC4/include/IfcIdentifier.h>

#include "IFCC_Types.h"

class TiXmlElement;

namespace IFCC {

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

inline bool isConstructionType(ObjectTypes type) {
	return type == OT_Wall || type == OT_Roof || type == OT_Slab;
}

inline bool isConstructionSimilarType(ObjectTypes type) {
	return type == OT_Beam || type == OT_Covering || type == OT_Column || type == OT_CurtainWall || type == OT_Footing;
}

inline bool isOpeningType(ObjectTypes type) {
	return type == OT_Window || type == OT_Door;
}

// conversion functions

/*! Convert wide string (Unicode) into UTF8 string.*/
std::string ws2s(const std::wstring& wstr);

/*! Return UTF8 version of label text from IfcLabel.*/
std::string label2s(const std::shared_ptr<IfcLabel>& label);

/*! Return UTF8 version of text from IfcText.*/
std::string text2s(const std::shared_ptr<IfcText>& text);

/*! Return UTF8 version of text from IfcText.*/
std::string name2s(const std::shared_ptr<IfcIdentifier>& text);

/*! Return UTF8 version of the guid of the given IFC object.*/
std::string guidFromObject(IfcRoot* object);

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

/*! Check if both vectors are nearly equal.
	It will be done by checking near equality of each coordinate (eps = 1e-4).
*/
bool nearEqual(const IBKMK::Vector3D& v1, const IBKMK::Vector3D& v2);

/*! Return the area of the given polygon.*/
double areaPolygon(const std::vector<IBKMK::Vector3D>& poly);

std::string objectTypeToString(ObjectTypes type);

} // end namespace

#endif // IFCC_HelperH
