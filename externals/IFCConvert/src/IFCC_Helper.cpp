#include "IFCC_Helper.h"

#include <locale>
#include <codecvt>
#include <fstream>

#include <ifcpp/IFC4/include/IfcGloballyUniqueId.h>

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
	for(size_t i=0; i<poly.size()-1; ++i) {
		tmp = tmp + poly[i].crossProduct(poly[(i+1)%poly.size()]);
	}
	return tmp.magnitude()*0.5;
}

std::string guidFromObject(IfcRoot* object) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
	return converterX.to_bytes(object->m_GlobalId->m_value);
}


} // end namespace
