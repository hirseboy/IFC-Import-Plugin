#include "IFCC_Helper.h"

#include <locale>
#include <codecvt>
#include <fstream>

#include <ifcpp/IFC4/include/IfcProduct.h>
#include <ifcpp/IFC4/include/IfcSpatialStructureElement.h>
#include <ifcpp/IFC4/include/IfcRelAggregates.h>
#include <ifcpp/IFC4/include/IfcRelContainedInSpatialStructure.h>
#include <ifcpp/IFC4/include/IfcLocalPlacement.h>
#include <ifcpp/IFC4/include/IfcAxis2Placement2D.h>
#include <ifcpp/IFC4/include/IfcAxis2Placement3D.h>
#include <ifcpp/IFC4/include/IfcCartesianPoint.h>
#include <ifcpp/IFC4/include/IfcLengthMeasure.h>
#include <ifcpp/IFC4/include/IfcGloballyUniqueId.h>
#include <ifcpp/IFC4/include/IfcLabel.h>
#include <ifcpp/IFC4/include/IfcText.h>

#include <carve/mesh_simplify.hpp>
#include <carve/csg.hpp>


#include <tinyxml.h>

#include <IBK_math.h>
#include <IBK_assert.h>

#include <IBKMK_2DCalculations.h>
#include <IBKMK_3DCalculations.h>

namespace IFCC {

int GUID_maker::m_guid = 1;


std::string ws2s(const std::wstring& wstr) {
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

void getChildren(const shared_ptr<IfcObjectDefinition>& object_def, std::vector<shared_ptr<IfcObjectDefinition> >& vec_children)
{
	if (!object_def) {
		return;
	}
	std::vector<shared_ptr<IfcObjectDefinition> >::iterator it_object_def;
	std::vector<shared_ptr<IfcProduct> >::iterator it_product;

	if (object_def->m_IsDecomposedBy_inverse.size() > 0)
	{
		std::vector<weak_ptr<IfcRelAggregates> >& vec_IsDecomposedBy = object_def->m_IsDecomposedBy_inverse;
		std::vector<weak_ptr<IfcRelAggregates> >::iterator it;
		for (it = vec_IsDecomposedBy.begin(); it!=vec_IsDecomposedBy.end(); ++it)
		{
			shared_ptr<IfcRelAggregates> rel_agg(*it);
			std::vector<shared_ptr<IfcObjectDefinition> >& vec = rel_agg->m_RelatedObjects;

			for (it_object_def = vec.begin(); it_object_def!=vec.end(); ++it_object_def)
			{
				shared_ptr<IfcObjectDefinition> child_obj_def = (*it_object_def);
				vec_children.push_back(child_obj_def);
				getChildren(child_obj_def, vec_children);
			}
		}
	}

	shared_ptr<IfcSpatialStructureElement> spatial_ele = dynamic_pointer_cast<IfcSpatialStructureElement>(object_def);
	if (spatial_ele)
	{
		std::vector<weak_ptr<IfcRelContainedInSpatialStructure> >& vec_contained = spatial_ele->m_ContainsElements_inverse;
		if (vec_contained.size() > 0)
		{
			std::vector<weak_ptr<IfcRelContainedInSpatialStructure> >::iterator it_rel_contained;
			for (it_rel_contained = vec_contained.begin(); it_rel_contained!=vec_contained.end(); ++it_rel_contained)
			{
				shared_ptr<IfcRelContainedInSpatialStructure> rel_contained(*it_rel_contained);
				std::vector<shared_ptr<IfcProduct> >& vec_related_elements = rel_contained->m_RelatedElements;
				std::vector<shared_ptr<IfcProduct> >::iterator it;

				for (it = vec_related_elements.begin(); it!=vec_related_elements.end(); ++it)
				{
					shared_ptr<IfcProduct> related_product = (*it);
					vec_children.push_back(related_product);
					getChildren(related_product, vec_children);
				}
			}
		}
	}
}

bool siteHasRelativePlacement(const shared_ptr<IfcSite>& ifc_site) {
	if (!ifc_site) {
		return false;
	}

	if (!ifc_site->m_ObjectPlacement) {
		return false;
	}

	shared_ptr<IfcLocalPlacement> local_placement = dynamic_pointer_cast<IfcLocalPlacement>(ifc_site->m_ObjectPlacement);
	if (!local_placement) {
		return false;
	}

	return local_placement->m_RelativePlacement != nullptr;
}

void resetIfcSiteLargeCoords(shared_ptr<IfcSite>& ifc_site) {
	if (!ifc_site) {
		return;
	}

	if (!ifc_site->m_ObjectPlacement) {
		return;
	}

	shared_ptr<IfcLocalPlacement> local_placement = dynamic_pointer_cast<IfcLocalPlacement>(ifc_site->m_ObjectPlacement);
	if (!local_placement) {
		return;
	}

	if (local_placement->m_RelativePlacement) {
		shared_ptr<IfcAxis2Placement3D> axis_placement = dynamic_pointer_cast<IfcAxis2Placement3D>(local_placement->m_RelativePlacement);
		if (axis_placement) {
			if (axis_placement->m_Location) {
				shared_ptr<IfcCartesianPoint> placement_location = dynamic_pointer_cast<IfcCartesianPoint>(axis_placement->m_Location);
				if (placement_location) {
//					if (placement_location->m_Coordinates.size() > 2) {
//						if (placement_location->m_Coordinates[0])
//						{
//							if (placement_location->m_Coordinates[0]->m_value > 1000) {
//								placement_location->m_Coordinates[0]->m_value = 0;
//							}
//						}

//						if (placement_location->m_Coordinates[1]) {
//							if (placement_location->m_Coordinates[1]->m_value > 1000) {
//								placement_location->m_Coordinates[1]->m_value = 0;
//							}
//						}

//						if (placement_location->m_Coordinates[2]) {
//							if (placement_location->m_Coordinates[2]->m_value > 1000) {
//								placement_location->m_Coordinates[2]->m_value = 0;
//							}
//						}
//					}
				}
			}
		}
	}
}


shared_ptr<MyIfcTreeItem> resolveTreeItems(shared_ptr<BuildingObject> obj, std::unordered_set<int>& set_visited) {
	shared_ptr<MyIfcTreeItem> item;

	shared_ptr<IfcObjectDefinition> obj_def = dynamic_pointer_cast<IfcObjectDefinition>(obj);
	if (obj_def) 	{
		if (set_visited.find(obj_def->m_entity_id) != set_visited.end()) {
			return nullptr;
		}
		set_visited.insert(obj_def->m_entity_id);

		item = std::shared_ptr<MyIfcTreeItem>(new MyIfcTreeItem());
		item->m_ifc_class_name = obj_def->className();

		// access some attributes of IfcObjectDefinition
		if (obj_def->m_GlobalId) {
			item->m_entity_guid = obj_def->m_GlobalId->m_value;
		}

		if (obj_def->m_Name) {
			item->m_name = obj_def->m_Name->m_value;
		}

		if (obj_def->m_Description) {
			item->m_description = obj_def->m_Description->m_value;
		}

		// check if there are child elements of current IfcObjectDefinition
		// child elements can be related to current IfcObjectDefinition either by IfcRelAggregates, or IfcRelContainedInSpatialStructure, see IFC doc
		if (obj_def->m_IsDecomposedBy_inverse.size() > 0) {
			// use inverse attribute to navigate to child elements:
			std::vector<weak_ptr<IfcRelAggregates> >& vec_IsDecomposedBy = obj_def->m_IsDecomposedBy_inverse;
			for ( auto it = vec_IsDecomposedBy.begin(); it != vec_IsDecomposedBy.end(); ++it) {
				shared_ptr<IfcRelAggregates> rel_agg(*it);
				std::vector<shared_ptr<IfcObjectDefinition> >& vec_related_objects = rel_agg->m_RelatedObjects;
				for (shared_ptr<IfcObjectDefinition> child_obj_def : vec_related_objects) {
					shared_ptr<MyIfcTreeItem> child_tree_item = resolveTreeItems(child_obj_def, set_visited);
					if (child_tree_item) 					{
						item->m_children.push_back(child_tree_item);
					}
				}
			}
		}

		shared_ptr<IfcSpatialStructureElement> spatial_ele = dynamic_pointer_cast<IfcSpatialStructureElement>(obj_def);
		if (spatial_ele) {
			// use inverse attribute to navigate to child elements:
			std::vector<weak_ptr<IfcRelContainedInSpatialStructure> >& vec_contained = spatial_ele->m_ContainsElements_inverse;
			if (vec_contained.size() > 0) 			{
				for (auto it_rel_contained = vec_contained.begin(); it_rel_contained != vec_contained.end(); ++it_rel_contained) {
					shared_ptr<IfcRelContainedInSpatialStructure> rel_contained(*it_rel_contained);
					std::vector<shared_ptr<IfcProduct> >& vec_related_elements = rel_contained->m_RelatedElements;

					for (shared_ptr<IfcProduct> related_product : vec_related_elements) {
						shared_ptr<MyIfcTreeItem> child_tree_item = resolveTreeItems(related_product, set_visited);
						if (child_tree_item) {
							item->m_children.push_back(child_tree_item);
						}
					}
				}
			}
		}
	}

	return item;
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


TiXmlElement * writeVector3D(TiXmlElement * parent, const std::string & name, const std::vector<IBKMK::Vector3D> & vec) {
	if (!vec.empty()) {
		TiXmlElement * child = new TiXmlElement(name);
		parent->LinkEndChild(child);

		std::stringstream vals;
		for (unsigned int i=0; i<vec.size(); ++i) {
			vals << vec[i].m_x << " " << vec[i].m_y << " " << vec[i].m_z;
			if (i<vec.size()-1)  vals << ", ";
		}
		TiXmlText * text = new TiXmlText( vals.str() );
		child->LinkEndChild( text );
		return child;
	}
	return nullptr;
}

carve::mesh::Face<3>* faceFromMeshset(const meshVector_t& meshvect, FaceIndex findex) {
	if(findex.m_meshSetIndex >= meshvect.size())
		return nullptr;

	if(findex.m_meshIndex >= meshvect[findex.m_meshSetIndex]->meshes.size())
		return nullptr;

	if(findex.m_faceIndex >= meshvect[findex.m_meshSetIndex]->meshes[findex.m_meshIndex]->faces.size())
		return nullptr;

	return  meshvect[findex.m_meshSetIndex]->meshes[findex.m_meshIndex]->faces[findex.m_faceIndex];
}

void simplifyMesh(meshVector_t& meshVector, bool removeLowVolume) {

	carve::mesh::MeshSimplifier simplifier;

	const double MINIMUM_NORMAL_ANGLE_DEG = 0.1;
	const double MINIMUM_NORMAL_ANGLE = MINIMUM_NORMAL_ANGLE_DEG * M_PI / 180.0;

	int meshSetCount = meshVector.size();
	for(int i=0; i<meshSetCount; ++i) {
		simplifier.mergeCoplanarFaces(meshVector[i].get(), MINIMUM_NORMAL_ANGLE);
//		simplifier.simplify(meshVector[i].get(), 1e-6, 1e-6, MINIMUM_NORMAL_ANGLE, 1e-6);
//		simplifier.improveMesh_conservative(meshVector[i].get());
		if(removeLowVolume)
			simplifier.removeLowVolumeManifolds( meshVector[i].get(), 0.000000001 );
	}

}

IBKMK::Vector3D fromCarveVector(const carve::geom::vector<3>& vect) {
	IBKMK::Vector3D res(vect.x, vect.y, vect.z);
	return res;
}

carve::geom::vector<3> toCarveVector(const IBKMK::Vector3D& vect) {
	carve::geom::vector<3> res = carve::geom::VECTOR(vect.m_x, vect.m_y, vect.m_z);
	return res;
}

carve::geom::plane<3> pointsToPlane(const IBKMK::Vector3D& x, const IBKMK::Vector3D& y, const IBKMK::Vector3D& z) {
	IBKMK::Vector3D xy = y - x;
	IBKMK::Vector3D xz = z - x;
	IBKMK::Vector3D N = xy.crossProduct(xz);
	IBKMK::Vector3D N0 = N * ( 1.0 / N.magnitude());
	double d = x.scalarProduct(N0);
	carve::geom::plane<3> plane(toCarveVector(N0),d);
	return plane;
}

bool nearEqual(const carve::geom::vector<3>& v1, const carve::geom::vector<3>& v2) {
	if(!IBK::near_equal(v1.x,v2.x))
		return false;
	if(!IBK::near_equal(v1.y,v2.y))
		return false;
	if(!IBK::near_equal(v1.z,v2.z))
		return false;

	return true;
}

bool nearEqual(const IBKMK::Vector2D& v1, const IBKMK::Vector2D& v2) {
	if(!IBK::near_equal(v1.m_x,v2.m_x))
		return false;
	if(!IBK::near_equal(v1.m_y,v2.m_y))
		return false;

	return true;
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

static IBKMK::Polygon2D projectPolygon3D(const IBKMK::Polygon3D& poly3D, const IBKMK::Vector3D& pos, const IBKMK::Vector3D& localX, const IBKMK::Vector3D& localY) {
	IBKMK::Polygon2D res;
	std::vector<IBKMK::Vector2D> poly2D;
	const std::vector<IBKMK::Vector3D>& vertexes = poly3D.vertexes();
	poly2D.reserve(vertexes.size());
	for (unsigned int i=0; i<vertexes.size(); ++i) {
		const IBKMK::Vector3D & v = vertexes[i];
		double x,y;
		if (IBKMK::planeCoordinates(pos, localX, localY, v, x, y)) {
			poly2D.push_back( IBKMK::Vector2D(x,y) );
		}
		else {
			return res;
		}
	}
	res.setVertexes(poly2D);
	return res;
}

bool intersects(const std::vector<IBKMK::Vector3D>& poly1, const std::vector<IBKMK::Vector3D>& poly2) {
	IBKMK::Polygon3D polygon1(poly1);
	IBKMK::Polygon3D polygon2(poly2);
	IBKMK::Polygon2D polygon2D1 = polygon1.polyline();
	IBKMK::Polygon2D polygon2D2 = projectPolygon3D(polygon2, polygon1.vertexes()[0], polygon1.localX(), polygon1.localY());
	const std::vector<IBKMK::Vector2D>& vertices1 = polygon2D1.vertexes();
	const std::vector<IBKMK::Vector2D>& vertices2 = polygon2D2.vertexes();
	// check if polygones are identical
	if(vertices1.size() == vertices2.size()) {
		bool equal = true;
		for(size_t i=0; i<vertices1.size(); ++i) {
			if(!nearEqual(vertices1[i],vertices2[i])) {
				equal = false;
				break;
			}
		}
		if(equal)
			return true;
	}

	// check if points of plygon 1 are inside polygon 2
	int pointsInside = 0;
	int pointsOutside = 0;
	std::vector<IBK::point2D<double>> touchingPoints;
	for(size_t i=0; i<vertices2.size(); ++i) {
		const IBKMK::Vector2D& p = vertices2[i];
		IBK::point2D<double> currentPoint(p.m_x,p.m_y);
		int res = IBKMK::pointInPolygon(polygon2D1.vertexes(), currentPoint);
		if(res == -1) {
			++pointsOutside;
		}
		else if(res == 1) {
			++pointsInside;
		}
		else {
			touchingPoints.push_back(currentPoint);
		}
	}
	// if at least one point is inside we have an intersection
	if(pointsInside > 0)
		return true;

	const std::vector<IBKMK::Vector2D>& vertices = polygon2D2.vertexes();
	for(size_t i=1; i<vertices.size(); ++i) {
		IBK::point2D<double> v1(vertices[i-1].m_x,vertices[i-1].m_y);
		IBK::point2D<double> v2(vertices[i].m_x,vertices[i].m_y);
		IBK::point2D<double> intersect;;

		if(polygon2D1.intersectsLine2D(v1,v2,intersect)) {
			bool touchingLine = false;
			if(!touchingPoints.empty()) {
				std::vector<IBK::point2D<double>>::iterator fit1 = std::find(touchingPoints.begin(), touchingPoints.end(), v1);
				std::vector<IBK::point2D<double>>::iterator fit2 = std::find(touchingPoints.begin(), touchingPoints.end(), v2);
				touchingLine = fit1 != touchingPoints.end() && fit2 != touchingPoints.end();
			}
			if(!touchingLine)
				return true;
		}
	}

	return false;
}

void clearZeros(std::vector<carve::mesh::Vertex<3>::vector_t>& vertices) {
	for(auto& vert : vertices) {
		if(IBK::near_zero(vert.x))
			vert.x = 0;
		if(IBK::near_zero(vert.y))
			vert.y = 0;
		if(IBK::near_zero(vert.z))
			vert.z = 0;
	}
}

std::unique_ptr<carve::mesh::MeshSet<3>> faceToMeshSet(carve::mesh::Face<3>* face) {
	std::vector<carve::mesh::Vertex<3>* > verticesTmp;
	face->getVertices(verticesTmp);
	std::vector<carve::mesh::Vertex<3>::vector_t> vertices(verticesTmp.size());
	std::vector<int> faceIndices(verticesTmp.size()+1, verticesTmp.size());
	for(size_t i=0; i<verticesTmp.size(); ++i) {
		vertices[i] = verticesTmp[i]->v;
		faceIndices[i+1] = i;
	}
	clearZeros(vertices);
	std::unique_ptr<carve::mesh::MeshSet<3>> meshset(new carve::mesh::MeshSet<3>(vertices,1,faceIndices));
	if( !meshset->isClosed() ) 	{
		closeMeshSet(meshset.get());
	}
	return meshset;
}

std::unique_ptr<carve::mesh::MeshSet<3>> vectToMeshSet(const std::vector<IBKMK::Vector3D>& vect) {
	std::vector<carve::mesh::Vertex<3>::vector_t> vertices(vect.size());
	std::vector<int> faceIndices(vect.size()+1, vect.size());
	for(size_t i=0; i<vect.size(); ++i) {
		vertices[i].x = vect[i].m_x;
		vertices[i].y = vect[i].m_y;
		vertices[i].z = vect[i].m_z;
		faceIndices[i+1] = i;
	}
	clearZeros(vertices);
	std::unique_ptr<carve::mesh::MeshSet<3>> meshset(new carve::mesh::MeshSet<3>(vertices,1,faceIndices));
	if( !meshset->isClosed() ) 	{
		closeMeshSet(meshset.get());
	}
	return meshset;
}

std::unique_ptr<carve::mesh::MeshSet<3>> vectToMeshSet(const std::vector<IBKMK::Vector2D>& vect) {
	IBK_ASSERT(!vect.empty());

	const double THICKNESS = 0.001;
	size_t numMainVertices = vect.size();
	int numEdges = int(numMainVertices);
	int numFaces = numEdges + 2;
	int numFaceIndices = 2 * (numMainVertices + 1) + numEdges * 5;
	std::vector<int> faceIndices(numFaceIndices, 0);
	std::vector<carve::mesh::Vertex<3>::vector_t> vertices(numMainVertices*2);
	size_t secondi = numMainVertices;
	faceIndices[0] = numMainVertices;
	faceIndices[numMainVertices+1] = numMainVertices;
	for(size_t i=0; i<numMainVertices; ++i, ++secondi) {
		vertices[i].x = vect[i].m_x;
		vertices[i].y = vect[i].m_y;
		vertices[i].z = THICKNESS;
		faceIndices[i+1] = i;
		vertices[secondi].x = vect[i].m_x;
		vertices[secondi].y = vect[i].m_y;
		vertices[secondi].z = THICKNESS*-1;
		faceIndices[i+numMainVertices+2] = secondi;

		faceIndices[2*numMainVertices+2 + i*5] = 4;

		if(i<numMainVertices-1) {
			faceIndices[2*numMainVertices+2 + i*5 + 1] = i;
			faceIndices[2*numMainVertices+2 + i*5 + 2] = i+1;
			faceIndices[2*numMainVertices+2 + i*5 + 3] = i+numMainVertices+1;
			faceIndices[2*numMainVertices+2 + i*5 + 4] = i+numMainVertices;
		}
		else {
			faceIndices[2*numMainVertices+2 + i*5 + 1] = i;
			faceIndices[2*numMainVertices+2 + i*5 + 2] = 0;
			faceIndices[2*numMainVertices+2 + i*5 + 3] = numMainVertices;
			faceIndices[2*numMainVertices+2 + i*5 + 4] = numMainVertices+i;
		}
	}
	clearZeros(vertices);
	std::unique_ptr<carve::mesh::MeshSet<3>> meshset(new carve::mesh::MeshSet<3>(vertices,numFaces,faceIndices));
	if( !meshset->isClosed() ) 	{
		closeMeshSet(meshset.get());
	}
	return meshset;

}

bool intersects(carve::mesh::Face<3>* face1, carve::mesh::Face<3>* face2) {
	carve::csg::CSG csg;
	carve::csg::CSG::OP op = carve::csg::CSG::INTERSECTION;

	std::unique_ptr<carve::mesh::MeshSet<3>> meshset1 = faceToMeshSet(face1);
	std::unique_ptr<carve::mesh::MeshSet<3>> meshset2 = faceToMeshSet(face2);
//	bool valid1 = checkMeshSetValidAndClosed(meshset1.get());
//	bool valid2 = checkMeshSetValidAndClosed(meshset2.get());
//	if(!valid1 || !valid2) {
//		return false;
//	}

	carve::mesh::MeshSet<3>* res = csg.compute(meshset1.get(), meshset2.get(), op);
	if(res == nullptr)
		return false;

	if(res->meshes.empty()) {
		return false;
	}
	return true;
}

bool intersects(carve::mesh::Face<3>* face1, carve::mesh::MeshSet<3>* meshset2) {
	carve::csg::CSG csg;
	carve::csg::CSG::OP op = carve::csg::CSG::INTERSECTION;

	std::unique_ptr<carve::mesh::MeshSet<3>> meshset1 = faceToMeshSet(face1);
//	bool valid1 = checkMeshSetValidAndClosed(meshset1.get());
//	bool valid2 = checkMeshSetValidAndClosed(meshset2);
//	if(!valid1 || !valid2) {
//		return false;
//	}

	try {
		carve::mesh::MeshSet<3>* res = csg.compute(meshset1.get(), meshset2, op);
		if(res == nullptr)
			return false;

		if(res->meshes.empty()) {
			return false;
		}
		return true;
	}
	catch (std::exception& e) {
		return false;
	}
}

bool intersects2(const std::vector<IBKMK::Vector3D>& poly1, const std::vector<IBKMK::Vector3D>& poly2) {
	carve::csg::CSG csg;
	carve::csg::CSG::OP op = carve::csg::CSG::INTERSECTION;

	std::unique_ptr<carve::mesh::MeshSet<3>> meshset1 = vectToMeshSet(poly1);
	std::unique_ptr<carve::mesh::MeshSet<3>> meshset2 = vectToMeshSet(poly2);
//	bool valid1 = checkMeshSetValidAndClosed(meshset1.get());
//	bool valid2 = checkMeshSetValidAndClosed(meshset2.get());
//	if(!valid1 || !valid2) {
//		return false;
//	}


	carve::mesh::MeshSet<3>* res = csg.compute(meshset1.get(), meshset2.get(), op);
	if(res == nullptr)
		return false;

	if(res->meshes.empty()) {
		return false;
	}
	return true;
}

bool intersects2From2D(const std::vector<IBKMK::Vector3D>& poly1, const std::vector<IBKMK::Vector3D>& poly2) {
	carve::csg::CSG csg;
	carve::csg::CSG::OP op = carve::csg::CSG::INTERSECTION;

	IBKMK::Polygon3D polygon1(poly1);
	IBKMK::Polygon3D polygon2(poly2);
	IBKMK::Polygon2D polygon2D1 = polygon1.polyline();
	IBKMK::Polygon2D polygon2D2 = projectPolygon3D(polygon2, polygon1.vertexes()[0], polygon1.localX(), polygon1.localY());
	std::vector<IBKMK::Vector2D> vertices1 = polygon2D1.vertexes();
	std::vector<IBKMK::Vector2D> vertices2 = polygon2D2.vertexes();

	std::unique_ptr<carve::mesh::MeshSet<3>> meshset1 = vectToMeshSet(vertices1);
	std::unique_ptr<carve::mesh::MeshSet<3>> meshset2 = vectToMeshSet(vertices2);
	bool valid1 = checkMeshSetValidAndClosed(meshset1.get());
	bool valid2 = checkMeshSetValidAndClosed(meshset2.get());
	if(!valid1 || !valid2) {
		return false;
	}

	if(intersects(poly1,poly2)) {
		int test = 1;
	}


	try {
		carve::mesh::MeshSet<3>* res = csg.compute(meshset1.get(), meshset2.get(), op, nullptr, carve::csg::CSG::CLASSIFY_EDGE);
		if(res == nullptr)
			return false;

		if(res->meshes.empty()) {
			return false;
		}
		return true;
	}
	catch (...) {
		return false;
	}
}

//\brief: collect connected edges and create face
carve::mesh::MeshSet<3>::face_t* createFaceFromEdgeLoop(carve::mesh::MeshSet<3>::edge_t* start) {
	carve::mesh::MeshSet<3>::edge_t* e = start;
	std::vector<carve::mesh::MeshSet<3>::edge_t*> loop_edges;
	do {
		if( e->rev != nullptr ) {
			return nullptr;
		}
		loop_edges.push_back(e);
		e = e->perimNext();
	} while( e != start );

	const size_t N = loop_edges.size();
	for( size_t i = 0; i < N; ++i ) {
		loop_edges[i]->rev = new carve::mesh::MeshSet<3>::edge_t(loop_edges[i]->v2(), nullptr);
	}

	for( size_t i = 0; i < N; ++i ) {
		carve::mesh::MeshSet<3>::edge_t* openEdge = loop_edges[i];
		carve::mesh::MeshSet<3>::edge_t* openEdgeNext = loop_edges[(i + 1) % N];
		carve::mesh::MeshSet<3>::edge_t* e1 = openEdge->rev;
		carve::mesh::MeshSet<3>::edge_t* e2 = openEdgeNext->rev;
		e1->prev = e2;
		e2->next = e1;

		e1->rev = openEdge;
		e2->rev = openEdgeNext;
	}

	carve::mesh::MeshSet<3>::face_t* f = new carve::mesh::MeshSet<3>::face_t(start->rev);

	if( f->n_edges != N ) {
		delete f;
		return nullptr;
	}

	return f;
}

void closeMeshSet(carve::mesh::MeshSet<3>* meshset) {
	// try to fix open mesh
	for( size_t i = 0; i < meshset->meshes.size(); ++i ) {
		carve::mesh::MeshSet<3>::mesh_t *mesh = meshset->meshes[i];
		const size_t numOpenEdgesInitial = mesh->open_edges.size();
		if( numOpenEdgesInitial == 0 ) {
			continue;
		}
		for( size_t kk = 0; kk < numOpenEdgesInitial; ++kk ) {
			const size_t numOpenEdges = mesh->open_edges.size();
			if( numOpenEdges == 0 ) {
				break;
			}

			mesh->faces.reserve(numOpenEdges + 1);

			carve::mesh::MeshSet<3>::edge_t *start = mesh->open_edges[0];

			carve::mesh::MeshSet<3>::edge_t *openEdge1 = nullptr;
			carve::mesh::MeshSet<3>::edge_t *openEdge2 = nullptr;
			std::vector<carve::mesh::MeshSet<3>::edge_t *> edges_to_close;
			edges_to_close.resize(numOpenEdges);
			carve::mesh::MeshSet<3>::edge_t *edge = start;
			size_t j = 0;
			size_t numOpenEdgesCurrentLoop = 0;
			do {
				edges_to_close[j++] = edge;

				carve::mesh::MeshSet<3>::edge_t *currentEdge = edge;
				carve::mesh::MeshSet<3>::edge_t *nextEdge = currentEdge->perimNext();
				++numOpenEdgesCurrentLoop;

				if( openEdge1 == nullptr )
				{
					// check if nextEdge is also an open edge
					for( size_t mm = 0; mm < mesh->open_edges.size(); ++mm )
					{
						carve::mesh::MeshSet<3>::edge_t* e = mesh->open_edges[mm];
						if( e == nextEdge )
						{
							openEdge1 = currentEdge;
							openEdge2 = nextEdge;
							break;
						}
					}
				}
				edge = nextEdge;
			} while( edge != start );

			if( numOpenEdgesCurrentLoop == 3 ) {
				if( openEdge1 != nullptr ) {
					// close with triangle
					carve::mesh::MeshSet<3>::face_t *closingTriangle = createFaceFromEdgeLoop(openEdge1);
					if( closingTriangle != nullptr ) {
						closingTriangle->mesh = mesh;
						mesh->faces.push_back(closingTriangle);
					}
				}
			}
			else if( numOpenEdgesCurrentLoop > 3 ) {
				if( openEdge1 != nullptr && openEdge2 != nullptr ) {
					// add triangle with 2 open edges and a new edge
					carve::mesh::MeshSet<3>::face_t *triangle = new carve::mesh::MeshSet<3>::face_t(openEdge1->v2(), openEdge1->v1(), openEdge2->v2());
					triangle->mesh = mesh;
					openEdge1->rev = triangle->edge;
					triangle->edge->rev = openEdge1;
					mesh->faces.push_back(triangle);

					carve::mesh::MeshSet<3>::edge_t *e1 = openEdge1->rev;
					carve::mesh::MeshSet<3>::edge_t *e2 = e1->prev;
					openEdge2->rev = e2;
					e2->rev = openEdge2;
					//e1->validateLoop();
				}
			}

			meshset->collectVertices();
			mesh->cacheEdges();
			mesh->calcOrientation();
		}
	}
}

bool checkMeshSetNonNegativeAndClosed( carve::mesh::MeshSet<3>* mesh_set ) {
	bool meshes_closed = true;
	if( !mesh_set->isClosed() ) 	{
		closeMeshSet(mesh_set);
	}

	for( size_t i = 0; i < mesh_set->meshes.size(); ++i ) {
		carve::mesh::Mesh<3>* mesh_i = mesh_set->meshes[i];
		if( mesh_i ) {
			if( mesh_i->isNegative() ) {
				mesh_i->invert();
				if( mesh_i->isNegative() ) {
					mesh_i->recalc();
					mesh_i->calcOrientation();
//					if( mesh_i->isNegative() ) {
//						std::cout << "could not invert mesh_set->meshes[" << i << "] " << std::endl;
//					}
				}
			}

			if( !mesh_i->isClosed() ) {
				meshes_closed = false;
			}

			if( mesh_i->open_edges.size() > 0 ) {
				meshes_closed = false;
			}
		}
	}
	return meshes_closed;
}

bool checkFaceIntegrity( carve::mesh::MeshSet<3>* meshset ) {
	for( size_t i = 0; i < meshset->meshes.size(); ++i ) {
		carve::mesh::Mesh<3>* mesh_i = meshset->meshes[i];
		if( mesh_i->open_edges.size() > 0 ) {
			//return false;
		}
		std::vector<carve::mesh::Face<3>* >& vec_faces = mesh_i->faces;
		for( size_t j = 0; j < vec_faces.size(); ++j ) {
			carve::mesh::Face<3>* face = vec_faces[j];
			carve::mesh::Edge<3>* e = face->edge;
			if( e == nullptr ) {
				continue;
			}

			const size_t n_edges = face->n_edges;
			for( size_t i_edge = 0; i_edge < n_edges; ++i_edge ) {
				if( !e->rev ) {
					return false;
				}
				if( e->rev->next == nullptr ) {
					return false;
				}
				if( !e->rev->next ) {
					return false;
				}
				if( !e->rev->next->next ) {
					return false;
				}
				if( !e->rev->next->next->vert ) {
					return false;
				}

				if( !e->next ) {
					return false;
				}
				if( e->next == nullptr ) {
					return false;
				}
				if( !e->prev ) {
					return false;
				}
				if( !e->rev ) {
					return false;
				}
				if( !e->vert ) {
					return false;
				}
				if( !e->face ) {
					return false;
				}

				e = e->next;
			}
			if( e != face->edge ) {
				return false;
			}
		}
	}
	return true;
}

bool checkMeshSetValidAndClosed( carve::mesh::MeshSet<3>* mesh_set) {
	if( !mesh_set ) {
		return false;
	}
	if( mesh_set->meshes.size() == 0 ) {
		return false;
	}

	if( !checkFaceIntegrity( mesh_set ) ) {
		return false;
	}
	std::stringstream err;
	bool meshes_closed = checkMeshSetNonNegativeAndClosed( mesh_set );
	if( meshes_closed ) {
		// check volume
		double meshset_volume = 0;
		for( size_t kk = 0; kk < mesh_set->meshes.size(); ++kk ) {
			carve::mesh::Mesh<3>* mesh = mesh_set->meshes[kk];
			double mesh_volume = mesh->volume();

			if( mesh_volume < 0 ) {
				mesh->invert();
				if( mesh->isNegative() ) {
					mesh->recalc();
					mesh->calcOrientation();
//					if( mesh->isNegative() ) {
//						std::cout << "could not invert negative mesh[" << kk << "] " << std::endl;
//					}
				}
				mesh_volume = mesh->volume();
			}

			if( mesh_volume < 0 ) {
				err << "mesh_volume < 0" << std::endl;
			}

			meshset_volume += mesh_volume;
		}
	}
	else {
		err << "mesh_set not closed" << std::endl;
	}

	if( err.tellp() > 0 ) {
		return false;
	}
	return true;
}

ObjectTypes typeFromObjectType(const std::string& typestring) {
	return OT_All;
}

std::string guidFromObject(IfcRoot* object) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
	return converterX.to_bytes(object->m_GlobalId->m_value);
}


} // end namespace
