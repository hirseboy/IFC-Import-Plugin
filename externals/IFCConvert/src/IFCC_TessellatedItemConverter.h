#ifndef IFCC_TessellatedItemConverterH
#define IFCC_TessellatedItemConverterH

/* -*-c++-*- IFC++ www.ifcquery.com

MIT License

Copyright (c) 2017 Fabian Gerold

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <ifcpp/model/StatusCallback.h>
#include <ifcpp/model/UnitConverter.h>
#include <ifcpp/IFC4/include/IfcTessellatedItem.h>
#include <ifcpp/IFC4/include/IfcCartesianPointList3D.h>
#include <ifcpp/IFC4/include/IfcPolygonalFaceSet.h>
#include <ifcpp/IFC4/include/IfcTriangulatedFaceSet.h>

#include <ifcpp/geometry/Carve/GeometryInputData.h>


namespace IFCC {

///@brief imports tessellated meshes as carve meshes
//Open tasks & TODOS:
//- check each face for the correct winding order (counter-clockwise for faces, clockwise for holes)
//- check if a face is degenerated (collapsed to a line or a point) and reject it if it is
//- check for crossing edges and reject faces if they contain some
//- check if voids overlap/cross each other and reject them if they do
//- improve run time of mergeHolesIntoPoly by using a spatial structure. Currently
//  uses a naive algorithm, which is probably good enough as most polys have few
//  vertices, and few holes with few vertices (each in the single digit range)
//- check closed parameter and use it to use a different addPolyhedron method
class TessellatedItemConverter : public StatusCallback {
public:
	TessellatedItemConverter(std::shared_ptr<UnitConverter> unit_converter);

	void convertTessellatedItem(std::shared_ptr<IfcTessellatedItem> const tessellated_item,
								std::shared_ptr<ItemShapeData> item_data);

	std::shared_ptr<UnitConverter> m_unit_converter;

protected:
	bool copyVertices(std::shared_ptr<IfcCartesianPointList3D> const point_list,
					  std::shared_ptr<carve::input::PolyhedronData> carve_mesh_builder);

	void convertPolygonalFaceSet(std::shared_ptr<IfcPolygonalFaceSet> const poly_face_set,
								 size_t const coordinate_count,
								 std::shared_ptr<carve::input::PolyhedronData> carve_mesh_builder);

	//also resolves indirect access via pn, giving direct indices into coord list
	void copyHoleIndices(std::vector<std::vector<int>>& hole_indices,
						 std::vector<std::vector<std::shared_ptr<IfcPositiveInteger>>> const& coord_index,
						 std::vector<std::shared_ptr<IfcPositiveInteger>> const& pn_indices,
						 size_t const coordinate_count);

	///@brief Merge boundary and hole indices together
	///@details This works because the IFC spec defines the winding order of
	///the boundary to be counter-clock-wise and the winding order of holes
	///to be clock-wise - the same order is used by carve internally. By
	///concatenating the indices and adding a back-edge index pair we basically
	///"cut" the polygon open and define the hole in a way carve recognises it.
	///
	///To find a suitable cutting edge, a naive nearest neighbour search is
	///done. This can be improved by using a spatial tree like kd-tree or
	///R-tree.
	void mergeHolesIntoPoly(std::vector<int>& vertex_indices,
							std::vector<std::vector<int>> const& hole_vertex_indices,
							std::shared_ptr<carve::input::PolyhedronData> carve_mesh_builder);

	void convertTriangulatedFaceSet(std::shared_ptr<IfcTriangulatedFaceSet> const tri_face_set,
									size_t const coordinate_count,
									std::shared_ptr<carve::input::PolyhedronData> carve_mesh_builder);

};


} // end namespace

#endif // IFCC_TessellatedItemConverterH
