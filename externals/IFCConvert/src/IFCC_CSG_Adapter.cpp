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

#include "IFCC_CSG_Adapter.h"

#include "IFCC_MeshOps.h"
#include "IFCC_MeshUtils.h"


namespace IFCC {

namespace CSG_Adapter {

	void assignResultOnFail(const shared_ptr<carve::mesh::MeshSet<3> >& op1, const shared_ptr<carve::mesh::MeshSet<3> >& op2, const carve::csg::CSG::OP operation, shared_ptr<carve::mesh::MeshSet<3> >& result)
	{
		if( operation == carve::csg::CSG::A_MINUS_B )
		{
			result = op1;
		}
		else if( operation == carve::csg::CSG::B_MINUS_A )
		{
			result = op2;
		}
		else if( operation == carve::csg::CSG::UNION )
		{
			result = op1;
		}
	}

	bool checkBoundinbBoxIntersection(const shared_ptr<carve::mesh::MeshSet<3> >& op1, const shared_ptr<carve::mesh::MeshSet<3> >& op2, const carve::csg::CSG::OP operation)
	{
		if( operation == carve::csg::CSG::UNION )
		{
			// union operation needs to be done also when there is no intersection
			return true;
		}

		carve::geom::aabb<3> bbox1 = op1->getAABB();
		carve::geom::aabb<3> bbox2 = op2->getAABB();
		double xmin1 = bbox1.pos.x - bbox1.extent.x;
		double xmin2 = bbox2.pos.x - bbox2.extent.x;
		double xmax1 = bbox1.pos.x + bbox1.extent.x;
		double xmax2 = bbox2.pos.x + bbox2.extent.x;
		if( xmin1 >= xmax2 )
		{
			return false;
		}
		if( xmax1 <= xmin2 )
		{
			return false;
		}

		double ymin1 = bbox1.pos.y - bbox1.extent.y;
		double ymin2 = bbox2.pos.y - bbox2.extent.y;
		double ymax1 = bbox1.pos.y + bbox1.extent.y;
		double ymax2 = bbox2.pos.y + bbox2.extent.y;
		if( ymin1 >= ymax2 )
		{
			return false;
		}
		if( ymax1 <= ymin2 )
		{
			return false;
		}

		double zmin1 = bbox1.pos.z - bbox1.extent.z;
		double zmin2 = bbox2.pos.z - bbox2.extent.z;
		double zmax1 = bbox1.pos.z + bbox1.extent.z;
		double zmax2 = bbox2.pos.z + bbox2.extent.z;
		if( zmin1 >= zmax2 )
		{
			return false;
		}
		if( zmax1 <= zmin2 )
		{
			return false;
		}

		double volume1 = MeshUtils::getMeshVolume(op1.get());
		double volume2 = MeshUtils::getMeshVolume(op2.get());

		if( std::abs(volume1) < 0.005 )
		{
			double maxExtent = std::max(bbox1.extent.x, std::max(bbox1.extent.y, bbox1.extent.z));
			if( maxExtent < 0.01 )
			{

				return false;
			}
		}
		if( std::abs(volume2) < 0.005 )
		{
			double maxExtent = std::max(bbox2.extent.x, std::max(bbox2.extent.y, bbox2.extent.z));
			if( maxExtent < 0.01 )
			{
				return false;
			}
		}

		return true;
	}

	bool computeCSG_Carve(const shared_ptr<carve::mesh::MeshSet<3> >& op1Orig, const shared_ptr<carve::mesh::MeshSet<3> >& op2Orig, const carve::csg::CSG::OP operation, shared_ptr<carve::mesh::MeshSet<3> >& result,
		shared_ptr<GeometrySettings>& geom_settings, bool normalizeCoords)
	{
		if( !op1Orig || !op2Orig )
		{
			assignResultOnFail(op1Orig, op2Orig, operation, result);
			return false;
		}

		if( op1Orig->vertex_storage.size() > 2000 )
		{
			assignResultOnFail(op1Orig, op2Orig, operation, result);
			return false;
		}
		
		if( op2Orig->vertex_storage.size() > 2000 )
		{
			assignResultOnFail(op1Orig, op2Orig, operation, result);
			return false;
		}
		
		bool intersecting = checkBoundinbBoxIntersection(op1Orig, op2Orig, operation);
		if( !intersecting )
		{
			assignResultOnFail(op1Orig, op2Orig, operation, result);
			return true;
		}

		shared_ptr<carve::mesh::MeshSet<3> > op1(op1Orig->clone());
		shared_ptr<carve::mesh::MeshSet<3> > op2(op2Orig->clone());

		bool result_meshset_ok = false;
		bool dumpMeshes = false;
		bool normalizeCoordsInsteadOfEpsilon = normalizeCoords;
		MeshOps::CarveMeshNormalizer normMesh(op1, op2, normalizeCoordsInsteadOfEpsilon);
		normMesh.disableNormalizeAll = false;

		double scale = normMesh.getScale();
		double CARVE_EPSILON = 1.0 * EPS_M8 / scale;
		geom_settings->setEpsilonCoplanarDistance(CARVE_EPSILON);
		double epsCoplanarDistance = geom_settings->getEpsilonCoplanarDistance();// m_epsCoplanarDistance;

		double CARVE_EPSILON_restore = epsCoplanarDistance;  // CARVE_EPSILON
		//ScopedEpsilonSetter epsilonSetter;
		if( !normalizeCoordsInsteadOfEpsilon )
		{
			//epsilonSetter.setEpsilonValue(eps);
		}

		std::stringstream strs_err;
		try
		{
			// normalize first, so that EPS values match the size of different meshes
			normMesh.normalizeMesh(op1, "op1", CARVE_EPSILON);
			normMesh.normalizeMesh(op2, "op2", CARVE_EPSILON);

			bool triangulateOperands = true;
			bool shouldBeClosedManifold = true;
			MeshOps::simplifyMeshSet(op1, geom_settings, triangulateOperands, shouldBeClosedManifold, dumpMeshes, CARVE_EPSILON);
			MeshOps::simplifyMeshSet(op2, geom_settings, triangulateOperands, shouldBeClosedManifold, dumpMeshes, CARVE_EPSILON);

			MeshOps::retriangulateMeshSetSimple(op1, false, epsCoplanarDistance, 0);
			MeshOps::retriangulateMeshSetSimple(op2, false, epsCoplanarDistance, 0);

			MeshSetInfo infoMesh1;
			MeshSetInfo infoMesh2;
			bool operand1valid = MeshUtils::checkMeshSetValidAndClosed(op1, infoMesh1, CARVE_EPSILON);
			bool operand2valid = MeshUtils::checkMeshSetValidAndClosed(op2, infoMesh2, CARVE_EPSILON);

			if( !operand1valid || !operand2valid )
			{
				assignResultOnFail(op1Orig, op2Orig, operation, result);
				return true;
			}



			////////////////////// compute carve csg operation   /////////////////////////////////////////////
			carve::csg::CSG csg(CARVE_EPSILON);
			result = shared_ptr<carve::mesh::MeshSet<3> >(csg.compute(op1.get(), op2.get(), operation, nullptr, carve::csg::CSG::CLASSIFY_EDGE));

			MeshSetInfo infoResult;
			result_meshset_ok = MeshUtils::checkMeshSetValidAndClosed(result, infoResult, CARVE_EPSILON, true, dumpMeshes );

			// TODO: check for fail with closed mesh, but not fully sliced through.

#ifdef _USE_CLASSIFY_NORMAL
			if( !result_meshset_ok )
			{
				bool shouldBeClosedManifold = true;
				MeshOps::mergeCoplanarFacesInMeshSet(op1, geom_settings, shouldBeClosedManifold, dumpMeshes, CARVE_EPSILON);
				carve::csg::CSG csg( CARVE_EPSILON );
				result = shared_ptr<carve::mesh::MeshSet<3> >(csg.compute(op1.get(), op2.get(), operation, nullptr, carve::csg::CSG::CLASSIFY_NORMAL));
				result_meshset_ok = MeshUtils::checkMeshSetValidAndClosed(result, infoResult, CARVE_EPSILON, true, dumpMeshes );

				if( result_meshset_ok )
				{
					shared_ptr<carve::mesh::MeshSet<3> > result_clone(result->clone());
					MeshOps::retriangulateMeshSetSimple(result_clone, false, CARVE_EPSILON, 0);
					result_meshset_ok = MeshUtils::checkMeshSetValidAndClosed(result, infoResult, CARVE_EPSILON);
				}
			}
#endif

			if( !result_meshset_ok )
			{
				dumpMeshes = true;


//#ifdef _USE_MANIFOLD_CSG
//				computeCSG_Manifold(op1, op2, operation, result, eps, report_callback, entity);
//				result_meshset_ok = MeshUtils::checkMeshSetValidAndClosed(result, infoResult, report_callback, entity.get());
//#endif
			}



			if( !result_meshset_ok )
			{
				strs_err << "csg operation failed" << std::endl;

			}
		}
		catch( carve::exception& ce )
		{
			strs_err << ce.str().c_str();
		}
		catch( const std::out_of_range& oor )
		{
			strs_err << oor.what();
		}
		catch( std::exception& e )
		{
			strs_err << e.what();
		}
		catch( ... )
		{
			strs_err << "csg operation failed" << std::endl;
		}

		if( strs_err.tellp() > 0 ) {

			if( !result_meshset_ok ) {
				assignResultOnFail(op1, op2, operation, result);
				if(result) {
					normMesh.deNormalizeMesh(result, "", CARVE_EPSILON);
				}
				return false;
			}
		}

		normMesh.deNormalizeMesh(result, "", CARVE_EPSILON);
		return result_meshset_ok;
	}

	void computeCSG(shared_ptr<carve::mesh::MeshSet<3> >& operand1, std::vector<shared_ptr<carve::mesh::MeshSet<3> > >& operands2, const carve::csg::CSG::OP operation, shared_ptr<carve::mesh::MeshSet<3> >& result,
		shared_ptr<GeometrySettings>& geom_settings)
	{
		if( !operand1 || operands2.size() == 0 ) {
			if( operation == carve::csg::CSG::A_MINUS_B ) {
				result = operand1;
				return;
			}

			return;
		}

		// TODO: scale here, then do all the bool ops, then unscale

		shared_ptr<carve::mesh::MeshSet<3> > resultCurrentMesh;
		bool success = false;
		std::multimap<double, shared_ptr<carve::mesh::MeshSet<3> > > mapVolumeMeshes;
		for( shared_ptr<carve::mesh::MeshSet<3> >&meshset2 : operands2 ) {
			double volume = MeshUtils::getMeshVolume(meshset2.get());
			mapVolumeMeshes.insert({ volume, meshset2 });
		}

		for( auto it = mapVolumeMeshes.rbegin(); it != mapVolumeMeshes.rend(); ++it ) {
//			double volume = it->first;
			shared_ptr<carve::mesh::MeshSet<3> >& mesh2 = it->second;

			bool normalizeCoords = true;
			success = computeCSG_Carve(operand1, mesh2, operation, resultCurrentMesh, geom_settings, normalizeCoords);
			if( success ) {
				result = resultCurrentMesh;
				
				if( operation == carve::csg::CSG::A_MINUS_B ) {
					operand1 = resultCurrentMesh;
				}
				continue;
			}
			normalizeCoords = !normalizeCoords;
			success = computeCSG_Carve(operand1, mesh2, operation, resultCurrentMesh, geom_settings, normalizeCoords);
			if( success ) {
				result = resultCurrentMesh;
				if( operation == carve::csg::CSG::A_MINUS_B ) {
					operand1 = resultCurrentMesh;
				}
				continue;
			}
		}
	}

} // end namespace CSG_Adapter

} // end namspace IFCC
