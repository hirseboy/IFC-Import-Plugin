#ifndef IFCC_IFCReaderH
#define IFCC_IFCReaderH

#include <IBK_assert.h>
#include <IBK_Path.h>

#include <IBKMK_Polygon3D.h>

#include <ReaderSTEP.h>

#include <ifcpp/IFC4/include/IfcSpaceTypeEnum.h>

#include "IFCC_GeometryConverter.h"
#include "IFCC_Types.h"
#include "IFCC_Space.h"
#include "IFCC_BuildingElement.h"
#include "IFCC_Building.h"
#include "IFCC_BuildingStorey.h"
#include "IFCC_Site.h"
#include "IFCC_Material.h"
#include "IFCC_Construction.h"
#include "IFCC_Database.h"
#include "IFCC_Instances.h"


namespace IFCC {


class IFCReader
{
public:
	IFCReader();

	explicit IFCReader(const std::wstring& filename);

	bool convertToSimVicus();

	void writeXML(const IBK::Path & filename) const;

	std::wstring					m_filename;
	std::shared_ptr<BuildingModel>	m_model;
	GeometryConverter				m_geometryConverter;
	bool							m_hasError;
	bool							m_hasWarning;
	std::string						m_errorText;
	std::string						m_warningText;
	std::string						m_progressText;

	objectShapeTypeVector_t			m_elementEntitesShape;
	objectShapeGUIDMap_t			m_spatialEntitesShape;
	objectShapeGUIDMap_t			m_spaceEntitesShape;
	objectShapeGUIDMap_t			m_unknownEntitesShape;
	objectShapeGUIDMap_t			m_buildingsShape;
	objectShapeGUIDMap_t			m_storeysShape;
	objectShapeGUIDMap_t			m_openingsShape;
	objectShapeGUIDMap_t			m_externalSpatialShapes;
	objectShapeGUIDMap_t			m_spatialZoneShapes;
	shared_ptr<ProductShapeData>	m_siteShape;

	// all building elemnts which are part of the construction and can contain openings
	// can be: Wall, Roof, Slab
	std::vector<BuildingElement>						m_constructionElements;
	// all building elemnts which can be a opening
	// Window, Door
	std::vector<BuildingElement>						m_openingElemnts;
	std::vector<BuildingElement>						m_otherElemnts;

	std::vector<Opening>								m_openings;
	Site												m_site;
	Database											m_database;
	Instances											m_instances;


private:

	static void messageTarget( void* obj_ptr, shared_ptr<StatusCallback::Message> t );

	bool read();

	void splitShapeData();

	bool typeByGuid(const std::string& guid, std::pair<ObjectTypes,std::shared_ptr<ProductShapeData>>& res);
};

} // end namespace

#endif // IFCC_IFCREADER_H
