#include "IFCC_Helper.h"

#include <locale>
#include <codecvt>
#include <fstream>

#include <ifcpp/IFC4/include/IfcGloballyUniqueId.h>
#include <ifcpp/IFC4/include/IfcWall.h>
#include <ifcpp/IFC4/include/IfcBeam.h>
#include <ifcpp/IFC4/include/IfcChimney.h>
#include <ifcpp/IFC4/include/IfcColumn.h>
#include <ifcpp/IFC4/include/IfcCovering.h>
#include <ifcpp/IFC4/include/IfcCurtainWall.h>
#include <ifcpp/IFC4/include/IfcDoor.h>
#include <ifcpp/IFC4/include/IfcFooting.h>
#include <ifcpp/IFC4/include/IfcMember.h>
#include <ifcpp/IFC4/include/IfcPile.h>
#include <ifcpp/IFC4/include/IfcPlate.h>
#include <ifcpp/IFC4/include/IfcRailing.h>
#include <ifcpp/IFC4/include/IfcRamp.h>
#include <ifcpp/IFC4/include/IfcRampFlight.h>
#include <ifcpp/IFC4/include/IfcRoof.h>
#include <ifcpp/IFC4/include/IfcShadingDevice.h>
#include <ifcpp/IFC4/include/IfcSlab.h>
#include <ifcpp/IFC4/include/IfcStair.h>
#include <ifcpp/IFC4/include/IfcStairFlight.h>
#include <ifcpp/IFC4/include/IfcWall.h>
#include <ifcpp/IFC4/include/IfcCivilElement.h>
#include <ifcpp/IFC4/include/IfcDistributionElement.h>
#include <ifcpp/IFC4/include/IfcElementAssembly.h>
#include <ifcpp/IFC4/include/IfcElementComponent.h>
#include <ifcpp/IFC4/include/IfcFurnishingElement.h>
#include <ifcpp/IFC4/include/IfcGeographicElement.h>
#include <ifcpp/IFC4/include/IfcTransportElement.h>
#include <ifcpp/IFC4/include/IfcVirtualElement.h>
#include <ifcpp/IFC4/include/IfcExternalSpatialElement.h>
#include <ifcpp/IFC4/include/IfcSpatialZone.h>
#include <ifcpp/IFC4/include/IfcWindow.h>
#include <ifcpp/IFC4/include/IfcFeatureElement.h>

#include <carve/mesh_simplify.hpp>

#include <IBK_math.h>
#include <IBK_assert.h>

namespace IFCC {

int GUID_maker::m_guid = 1;


std::string ws2s(const std::wstring& wstr) {
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}


std::string label2s(const std::shared_ptr<IfcLabel>& label) {
	if(label != nullptr)
		return ws2s(label->m_value);
	return std::string();
}

std::string text2s(const std::shared_ptr<IfcText>& label) {
	if(label != nullptr)
		return ws2s(label->m_value);
	return std::string();
}

std::string name2s(const std::shared_ptr<IfcIdentifier>& text) {
	if(text != nullptr)
		return ws2s(text->m_value);
	return std::string();
}


void convert(const carve::mesh::MeshSet<3>& meshSet, std::vector<std::vector<std::vector<IBKMK::Vector3D>>>&  polyvect) {
	const carve::mesh::MeshSet<3>& currMesh = meshSet;
	int mCount = currMesh.meshes.size();
	for(int mi=0; mi<mCount; ++mi) {
		polyvect.push_back(std::vector<std::vector<IBKMK::Vector3D>>());
		carve::mesh::MeshSet<3>::mesh_t* mesh = currMesh.meshes[mi];
		int faCount = mesh->faces.size();
		for(int fi = 0; fi<faCount; ++fi) {
			carve::mesh::Face<3>* face = mesh->faces[fi];
			std::vector<carve::mesh::Vertex<3>* > vertices;
			face->getVertices(vertices);
			int numVert = vertices.size();
			polyvect.back().push_back(std::vector<IBKMK::Vector3D>(numVert));
			for(int vi=0; vi<numVert; ++vi) {
				double x = vertices[vi]->v.x;
				double y = vertices[vi]->v.y;
				double z = vertices[vi]->v.z;
				polyvect.back().back()[vi].set(x,y,z);
			}
		}
	}
}

void meshDump(const carve::mesh::MeshSet<3>& meshSet, const std::string& name, const IBK::Path& filename) {
	std::ofstream file(filename.c_str(), std::ios::app);
	if(!file.is_open())
		return;

	file << "\n" << name << "\n";
	const carve::mesh::MeshSet<3>& currMesh = meshSet;
	int mCount = currMesh.meshes.size();
	for(int mi=0; mi<mCount; ++mi) {
		file << "mesh: " << mi << "\n";
		carve::mesh::MeshSet<3>::mesh_t* mesh = currMesh.meshes[mi];
		int faCount = mesh->faces.size();
		for(int fi = 0; fi<faCount; ++fi) {
			carve::mesh::Face<3>* face = mesh->faces[fi];
			std::vector<carve::mesh::Vertex<3>* > vertices;
			face->getVertices(vertices);
			int numVert = vertices.size();
			if(numVert == 3)
				file << "dreieck(";
			else if(numVert == 4)
				file << "viereck(";
			else
				file << "polygon(";
			for(int vi=0; vi<numVert; ++vi) {
				double x = vertices[vi]->v.x;
				double y = vertices[vi]->v.y;
				double z = vertices[vi]->v.z;
				if(vi>0)
					file << " ";
				file << x << "|" << y << "|" << z;
			}
			file << ")\n";
		}
	}
	file << "end: " << name << "\n\n";
}


void faceDump(carve::mesh::Face<3>* face, const std::string& name, const IBK::Path& filename) {
	std::vector<std::vector<std::vector<IBKMK::Vector3D>>> data;
	std::ofstream file(filename.c_str(), std::ios::app);
	if(file.is_open()) {
		file << "\n" << name <<"\n";
		std::vector<carve::mesh::Vertex<3>* > vertices;
		face->getVertices(vertices);
		int numVert = vertices.size();
		for( size_t i=0; i<numVert; ++i) {
			if(i>0)
				file << " ";
			file << vertices[i]->v.x << "|" << vertices[i]->v.y << "|" << vertices[i]->v.z;
		}
	}
}


void simplifyMesh(meshVector_t& meshVector, bool removeLowVolume) {

	carve::mesh::MeshSimplifier simplifier;

	const double MINIMUM_NORMAL_ANGLE_DEG = 0.1;
	const double MINIMUM_NORMAL_ANGLE = MINIMUM_NORMAL_ANGLE_DEG * M_PI / 180.0;

	int meshSetCount = meshVector.size();
	for(int i=0; i<meshSetCount; ++i) {
		simplifier.mergeCoplanarFaces(meshVector[i].get(), MINIMUM_NORMAL_ANGLE);
	// The following functions can open or destroy the mesh
//		simplifier.simplify(meshVector[i].get(), 1e-6, 1e-6, MINIMUM_NORMAL_ANGLE, 1e-6);
//		simplifier.improveMesh_conservative(meshVector[i].get());
		if(removeLowVolume)
			simplifier.removeLowVolumeManifolds( meshVector[i].get(), 1e-8 );
	}

}

bool nearEqual(const IBKMK::Vector3D& v1, const IBKMK::Vector3D& v2) {
	if(!IBK::nearly_equal<4>(v1.m_x,v2.m_x))
		return false;
	if(!IBK::nearly_equal<4>(v1.m_y,v2.m_y))
		return false;
	if(!IBK::nearly_equal<4>(v1.m_z,v2.m_z))
		return false;

	return true;
}

double areaPolygon(const std::vector<IBKMK::Vector3D>& poly) {
	IBKMK::Vector3D tmp;
	if(poly.empty())
		return 0;

	for(size_t i=0; i<poly.size()-1; ++i) {
		tmp = tmp + poly[i].crossProduct(poly[(i+1)%poly.size()]);
	}
	return tmp.magnitude()*0.5;
}

std::string guidFromObject(IfcRoot* object) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
	return converterX.to_bytes(object->m_GlobalId->m_value);
}

std::string objectTypeToString(ObjectTypes type) {
	switch(type) {
		case OT_Beam:					return "beam";
		case OT_Chimney:				return "chimney";
		case OT_Column:					return "column";
		case OT_Covering:				return "covering";
		case OT_CurtainWall:			return "curtain wall";
		case OT_Door:					return "door";
		case OT_Footing:				return "footing";
		case OT_Member:					return "member";
		case OT_Pile:					return "pile";
		case OT_Plate:					return "plate";
		case OT_Railing:				return "railing";
		case OT_Ramp:					return "ramp";
		case OT_RampFlight:				return "ramp flight";
		case OT_Roof:					return "roof";
		case OT_ShadingDevice:			return "shading device";
		case OT_Slab:					return "slab";
		case OT_Stair:					return "stair";
		case OT_StairFlight:			return "stair flight";
		case OT_Wall:					return "wall";
		case OT_Window:					return "window";
		case OT_CivilElement:			return "civil element";
		case OT_DistributionElement:	return "distribution element";
		case OT_ElementAssembly:		return "element assembly";
		case OT_ElementComponent:		return "element component";
		case OT_FeatureElement:			return "feature element";
		case OT_FurnishingElement:		return "furnishing element";
		case OT_GeographicalElement:	return "geographical element";
		case OT_TransportElement:		return "transport element";
		case OT_VirtualElement:			return "virtual element";
		case OT_All:					return "not defined";
		case OT_None:					return "not defined";
	}
	return "not defined";
}

ObjectTypes getObjectType(const std::shared_ptr<IfcObjectDefinition>& od) {
	if(dynamic_pointer_cast<IfcElement>(od) == nullptr)
		return OT_None;

	if(dynamic_pointer_cast<IfcWall>(od) != nullptr)
		return OT_Wall;
	if(dynamic_pointer_cast<IfcBeam>(od) != nullptr)
		return OT_Beam;
	if(dynamic_pointer_cast<IfcChimney>(od) != nullptr)
		return OT_Chimney;
	if(dynamic_pointer_cast<IfcCovering>(od) != nullptr)
		return OT_Covering;
	if(dynamic_pointer_cast<IfcCurtainWall>(od) != nullptr)
		return OT_CurtainWall;
	if(dynamic_pointer_cast<IfcDoor>(od) != nullptr)
		return OT_Door;
	if(dynamic_pointer_cast<IfcFooting>(od) != nullptr)
		return OT_Footing;
	if(dynamic_pointer_cast<IfcMember>(od) != nullptr)
		return OT_Member;
	if(dynamic_pointer_cast<IfcPile>(od) != nullptr)
		return OT_Pile;
	if(dynamic_pointer_cast<IfcPlate>(od) != nullptr)
		return OT_Plate;
	if(dynamic_pointer_cast<IfcRailing>(od) != nullptr)
		return OT_Railing;
	if(dynamic_pointer_cast<IfcRamp>(od) != nullptr)
		return OT_Ramp;
	if(dynamic_pointer_cast<IfcRampFlight>(od) != nullptr)
		return OT_RampFlight;
	if(dynamic_pointer_cast<IfcRoof>(od) != nullptr)
		return OT_Roof;
	if(dynamic_pointer_cast<IfcShadingDevice>(od) != nullptr)
		return OT_ShadingDevice;
	if(dynamic_pointer_cast<IfcSlab>(od) != nullptr)
		return OT_Slab;
	if(dynamic_pointer_cast<IfcStair>(od) != nullptr)
		return OT_Stair;
	if(dynamic_pointer_cast<IfcStairFlight>(od) != nullptr)
		return OT_StairFlight;
	if(dynamic_pointer_cast<IfcWindow>(od) != nullptr)
		return OT_Window;
	if(dynamic_pointer_cast<IfcFeatureElement>(od) != nullptr)
		return OT_FeatureElement;
	if(dynamic_pointer_cast<IfcCivilElement>(od) != nullptr)
		return OT_CivilElement;
	if(dynamic_pointer_cast<IfcDistributionElement>(od) != nullptr)
		return OT_DistributionElement;
	if(dynamic_pointer_cast<IfcElementAssembly>(od) != nullptr)
		return OT_ElementAssembly;
	if(dynamic_pointer_cast<IfcElementComponent>(od) != nullptr)
		return OT_ElementComponent;
	if(dynamic_pointer_cast<IfcFurnishingElement>(od) != nullptr)
		return OT_FurnishingElement;
	if(dynamic_pointer_cast<IfcGeographicElement>(od) != nullptr)
		return OT_GeographicalElement;
	if(dynamic_pointer_cast<IfcTransportElement>(od) != nullptr)
		return OT_TransportElement;
	if(dynamic_pointer_cast<IfcVirtualElement>(od) != nullptr)
		return OT_VirtualElement;

	return OT_All;

}

} // end namespace
