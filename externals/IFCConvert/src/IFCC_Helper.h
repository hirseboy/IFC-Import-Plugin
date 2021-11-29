#ifndef IFCC_HelperH
#define IFCC_HelperH

#include <vector>
#include <unordered_set>

#include <IBKMK_Polygon3D.h>

#include <IBK_Path.h>

#include <ifcpp/IFC4/include/IfcObjectDefinition.h>
#include <ifcpp/IFC4/include/IfcSite.h>
#include <ifcpp/IFC4/include/IfcLabel.h>
#include <ifcpp/IFC4/include/IfcText.h>

#include "IFCC_Types.h"

class TiXmlElement;

namespace IFCC {

struct MyIfcTreeItem
{
	MyIfcTreeItem() = default;
	std::wstring m_name;
	std::wstring m_description;
	std::wstring m_entity_guid;
	std::string m_ifc_class_name;
	std::vector<shared_ptr<MyIfcTreeItem> > m_children;
};

struct FaceIndex {
	int m_meshSetIndex;
	int m_meshIndex;
	int m_faceIndex;
	FaceIndex() :
		m_meshSetIndex(-1),
		m_meshIndex(-1),
		m_faceIndex(-1)
	{}

	FaceIndex(int msi, int mi, int fi) :
		m_meshSetIndex(msi),
		m_meshIndex(mi),
		m_faceIndex(fi)
	{}

	operator int() const { return m_meshSetIndex * 10000 + m_meshIndex * 1000 + m_faceIndex; }
};

class GUID_maker {
public:
	static GUID_maker& instance() {
		static GUID_maker self;
		return self;
	}
	int guid() {
		return m_guid++;
	}


private:
	static int m_guid;
};

// IFC helper

std::string ws2s(const std::wstring& wstr);

inline std::string label2s(const std::shared_ptr<IfcLabel>& label) {
	if(label != nullptr)
		return ws2s(label->m_value);
	return std::string();
}

inline std::string text2s(const std::shared_ptr<IfcText>& label) {
	if(label != nullptr)
		return ws2s(label->m_value);
	return std::string();
}

void getChildren(const shared_ptr<IfcObjectDefinition>& object_def, std::vector<shared_ptr<IfcObjectDefinition> >& vec_children);

shared_ptr<MyIfcTreeItem> resolveTreeItems(shared_ptr<BuildingObject> obj, std::unordered_set<int>& set_visited);

bool siteHasRelativePlacement(const shared_ptr<IfcSite>& ifc_site);

void resetIfcSiteLargeCoords(shared_ptr<IfcSite>& ifc_site);

// conversion functions

void convert(const carve::mesh::MeshSet<3>& meshSet, std::vector<std::vector<std::vector<IBKMK::Vector3D>>>&  polyvect);

carve::mesh::Face<3>* faceFromMeshset(const meshVector_t& meshvect, FaceIndex findex);

// debug helper

void meshDump(const carve::mesh::MeshSet<3>& meshSet, const std::string& name, const IBK::Path& filename);

void faceDump(carve::mesh::Face<3>* face, const std::string& name, const IBK::Path& filename);

TiXmlElement * writeVector3D(TiXmlElement * parent, const std::string & name, const std::vector<IBKMK::Vector3D> & vec);

// mesh helper

void simplifyMesh(meshVector_t& meshVector, bool removeLowVolume);

void closeMeshSet(carve::mesh::MeshSet<3>* meshset);

bool checkMeshSetValidAndClosed( carve::mesh::MeshSet<3>* mesh_set);

IBKMK::Vector3D fromCarveVector(const carve::geom::vector<3>& vect);

carve::geom::vector<3> toCarveVector(const IBKMK::Vector3D& vect);

carve::geom::plane<3> pointsToPlane(const IBKMK::Vector3D& x, const IBKMK::Vector3D& y, const IBKMK::Vector3D& z);

bool nearEqual(const carve::geom::vector<3>& v1, const carve::geom::vector<3>& v2);

bool nearEqual(const IBKMK::Vector2D& v1, const IBKMK::Vector2D& v2);

bool nearEqual(const IBKMK::Vector3D& v1, const IBKMK::Vector3D& v2);

double areaPolygon(const std::vector<IBKMK::Vector3D>& poly);

bool intersects(const std::vector<IBKMK::Vector3D>& poly1, const std::vector<IBKMK::Vector3D>& poly2);

bool intersects2(const std::vector<IBKMK::Vector3D>& poly1, const std::vector<IBKMK::Vector3D>& poly2);

bool intersects2From2D(const std::vector<IBKMK::Vector3D>& poly1, const std::vector<IBKMK::Vector3D>& poly2);

bool intersects(carve::mesh::Face<3>* face1, carve::mesh::Face<3>* face2);

bool intersects(carve::mesh::Face<3>* face1, carve::mesh::MeshSet<3>* meshset2);

ObjectTypes typeFromObjectType(const std::string& typestring);

std::string guidFromObject(IfcRoot* object);

} // end namespace

#endif // IFCC_HelperH
