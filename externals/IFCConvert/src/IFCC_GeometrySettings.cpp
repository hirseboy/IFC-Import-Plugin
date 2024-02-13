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

#include "IFCC_GeometrySettings.h"

namespace IFCC {

	GeometrySettings::GeometrySettings() 	{
		m_render_object_filter.insert(1287392070);  // IfcFeatureElementSubtraction
	}

	/**\brief Render filter decides if a IfcObjectDefinition should be rendered.
	  The default filter will render all objects except objects based on IfcFeatureElementSubtraction.*/
	bool GeometrySettings::skipRenderObject(uint32_t classID) 	{
		if( m_render_object_filter.find(classID) != m_render_object_filter.end() ) {
			return true;
		}
		return false;
	}

	void GeometrySettings::setEpsilonCoplanarDistance(double eps) {
		m_epsCoplanarDistance = eps;
	}

	double GeometrySettings::getEpsilonCoplanarDistance() {
		return m_epsCoplanarDistance;
	}

	void GeometrySettings::setEpsilonCoplanarAngle(double eps) {
		m_epsCoplanarAngle = eps;
	}

	double GeometrySettings::getEpsilonCoplanarAngle() {
		return m_epsCoplanarAngle;
	}

	double GeometrySettings::minimumSurfaceArea() const 	{
		return m_minimumSurfaceArea;
	}

	void GeometrySettings::setMinimumSurfaceArea(double minimumSurfaceArea) 	{
		m_minimumSurfaceArea = minimumSurfaceArea;
	}

	// struct GeomProcessing

	GeomProcessingParams::GeomProcessingParams(double epsMergePoints, double epsMergeAlignedEdgesAngle, double minFaceArea, double minSurfaceArea) {
		m_epsMergePoints = epsMergePoints;
		m_epsMergeAlignedEdgesAngle = epsMergeAlignedEdgesAngle;
		m_minFaceArea = minFaceArea;
		m_minimumSurfaceArea = minSurfaceArea;
	}

	GeomProcessingParams::GeomProcessingParams( shared_ptr<GeometrySettings>& generalSettings ) {
		m_epsMergePoints = generalSettings->getEpsilonCoplanarDistance();// m_epsCoplanarDistance;
		m_epsMergeAlignedEdgesAngle = generalSettings->getEpsilonCoplanarAngle();// m_epsCoplanarAngle;
		m_minFaceArea = generalSettings->getEpsilonCoplanarDistance() * 0.01;// m_epsCoplanarDistance * 0.01;
		m_minimumSurfaceArea = generalSettings->minimumSurfaceArea();
	}

	GeomProcessingParams::GeomProcessingParams( shared_ptr<GeometrySettings>& generalSettings, BuildingEntity* ifc_entity, StatusCallback* callbackFunc) {
		m_epsMergePoints = generalSettings->getEpsilonCoplanarDistance();// generalSettings->m_epsCoplanarDistance;
		m_epsMergeAlignedEdgesAngle = generalSettings->getEpsilonCoplanarAngle();//generalSettings->m_epsCoplanarAngle;
		m_ifc_entity = ifc_entity;
		m_callbackFunc = callbackFunc;
		m_minFaceArea = generalSettings->getEpsilonCoplanarDistance() * 0.01;// generalSettings->m_epsCoplanarDistance*0.01;
		m_minimumSurfaceArea = generalSettings->minimumSurfaceArea();
	}


} // end namespace
