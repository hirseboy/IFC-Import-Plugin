#ifndef IFCC_SweeperH
#define IFCC_SweeperH

/* -*-c++-*- IfcQuery www.ifcquery.com
*
MIT License

Copyright (c) 2017 Fabian Gerold

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <ifcpp/model/BasicTypes.h>
#include <ifcpp/model/StatusCallback.h>
#include <earcut/include/mapbox/earcut.hpp>
#include <ifcpp/geometry/IncludeCarveHeaders.h>
#include <ifcpp/geometry/GeomDebugDump.h>

#include "IFCC_CSG_Adapter.h"
#include "IFCC_GeometryInputData.h"

class GeometrySettings;
class UnitConverter;

namespace IFCC {


class Sweeper : public StatusCallback
{
public:
	shared_ptr<GeometrySettings>		m_geom_settings;
	shared_ptr<UnitConverter>			m_unit_converter;

	Sweeper(shared_ptr<GeometrySettings>& settings, shared_ptr<UnitConverter>& uc) :
		m_geom_settings(settings), m_unit_converter(uc) {}

	virtual ~Sweeper(){}

	/*\brief Extrudes a set of cross sections along a direction
	  \param[in] paths Set of cross sections to extrude
	  \param[in] dir Extrusion vector
	  \param[in] e Ifc entity that the geometry belongs to (just for error messages). Pass a nullptr if no entity at hand.
	  \param[out] item_data Container to add result polyhedron or polyline
	**/
	void extrude(const std::vector<std::vector<vec2> >& faceLoopsInput, const vec3 extrusionVector,
				 shared_ptr<ItemShapeData>& itemData, GeomProcessingParams& params);

	/*\brief Extrudes a circle cross section along a path. At turns, the points are placed in the bisecting plane
	  \param[in] curve_points Path along which the circle is swept
	  \param[in] e Ifc entity that the geometry belongs to (just for error messages). Pass a nullptr if no entity at hand.
	  \param[out] item_data Container to add result polyhedron or polyline
	  \param[in] nvc Number of vertices per circle
	  \param[in] radius_inner If positive value is given, the swept disk becomes a pipe
	**/
	void sweepDisk( const std::vector<vec3>& curve_points, shared_ptr<ItemShapeData>& item_data,
					GeomProcessingParams& params, const size_t nvc, const double radius,
					const double radius_inner = -1 );

	void findEnclosedLoops(const std::vector<std::vector<vec2> >& face_loops_input,
						   std::vector<std::vector<std::vector<vec2> > >& profile_paths_enclosed,
						   double CARVE_EPSILON);

	void triangulateLoops( const std::vector<std::vector<vec2> >& profile_paths_input,
						   std::vector<std::vector<vec2> >& face_loops_used_for_triangulation,
						   std::vector<int>& face_indexes_out, BuildingEntity* ifc_entity );

	/*\brief Extrudes a cross section along a path. At turns, the points are placed in the bisecting plane
	  \param[in] curve_points Path along which the cross section is swept
	  \param[in] profile_paths Set of cross sections to sweep
	  \param[in] e Ifc entity that the geometry belongs to (just for error messages). Pass a nullptr if no entity at hand.
	  \param[out] item_data Container to add result polyhedron or polyline
	**/
	void sweepArea(const std::vector<vec3>& curve_points, const std::vector<std::vector<vec2> >& profile_paths_input,
				   shared_ptr<ItemShapeData>& item_data, GeomProcessingParams& params);
};

} // end namespace

#endif // IFCC_SweeperH
