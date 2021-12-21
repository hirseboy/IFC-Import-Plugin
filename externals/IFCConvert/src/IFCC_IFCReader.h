#ifndef IFCC_IFCReaderH
#define IFCC_IFCReaderH

#include <IBK_assert.h>
#include <IBK_Path.h>

#include <IBKMK_Polygon3D.h>

#include <ReaderSTEP.h>

#include <ifcpp/IFC4/include/IfcSpaceTypeEnum.h>

#include <VICUS_Project.h>

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

/*! Main class for IFC reading and conversion int vicus model.*/
class IFCReader
{
public:
	/*! Standard constructor. Initializes geometry converter and set the IFC file.*/
	explicit IFCReader(const std::wstring& filename);

	/*! Convert IFC data into internal format similar to SimVicus.
		It needs calling of read and splitShapeData before.
	*/
	bool convert();

	bool setVicusProject(VICUS::Project* project);

	/*! Write converted data as vicus file.*/
	void writeXML(const IBK::Path & filename) const;

	std::wstring					m_filename;				///< IFC file
	std::shared_ptr<BuildingModel>	m_model;				///< IFC model created from file
	GeometryConverter				m_geometryConverter;	///< Geometry converter for converting local to global coordinates.
	bool							m_hasError;				///< If true an error while reading IFC file was occured
	bool							m_hasWarning;			///< If true an warning while reading IFC file was occured
	std::string						m_errorText;			///< Text of error messages
	std::string						m_warningText;			///< Text of warning messages
	std::string						m_progressText;			///< Progress text

	/*! Vector for shapes of building element entities with type.*/
	objectShapeTypeVector_t			m_elementEntitesShape;
	/*! Map with GUID as key and corresponding shape of spatial entity (except site, building, storey and space).*/
	objectShapeGUIDMap_t			m_spatialEntitesShape;
	/*! Map with GUID as key and corresponding shape of space entity.*/
	objectShapeGUIDMap_t			m_spaceEntitesShape;
	/*! Map with GUID as key and corresponding shape of entity which are others than element, opening or spatial structure.*/
	objectShapeGUIDMap_t			m_unknownEntitesShape;
	/*! Map with GUID as key and corresponding shape of building entity.*/
	objectShapeGUIDMap_t			m_buildingsShape;
	/*! Map with GUID as key and corresponding shape of storey entity.*/
	objectShapeGUIDMap_t			m_storeysShape;
	/*! Map with GUID as key and corresponding shape of opening entity.*/
	objectShapeGUIDMap_t			m_openingsShape;
	/*! Map with GUID as key and corresponding shape of external spatial entity.*/
	objectShapeGUIDMap_t			m_externalSpatialShapes;
	/*! Map with GUID as key and corresponding shape of spatial zone entity.*/
	objectShapeGUIDMap_t			m_spatialZoneShapes;
	/*! Pointer to site shape.*/
	shared_ptr<ProductShapeData>	m_siteShape;

	/*! All building elements which are part of the construction and can contain openings.
		This can be: Wall, Roof, Slab
	*/
	std::vector<BuildingElement>						m_constructionElements;
	/*! All building elements which can be a opening.
		This can be: Window, Door
	*/
	std::vector<BuildingElement>						m_openingElemnts;
	/*! All other building elements.*/
	std::vector<BuildingElement>						m_otherElemnts;

	/*! Vector of opening elements.
		Openings should have connections to the corresponding opening structure and the construction element which contains the opening.
	*/
	std::vector<Opening>								m_openings;
	/*! Building site. It can contain geometric objects for surrounding.*/
	Site												m_site;
	/*! Handler class for all databases (constructions, materials, windows, glazings, components).*/
	Database											m_database;
	/*! Handler class for all component instances. Such a instance is a connection of components and surfaces from spaces.*/
	Instances											m_instances;


private:

	/*! Function for collecting messages from IFC reading process (error, warning, progress).*/
	static void messageTarget( void* obj_ptr, shared_ptr<StatusCallback::Message> t );

	/*! Read the IFC file given in constructor.*/
	bool read();

	/*! Call the geometry converter which converts IFC objects from local to global coordinate system.
		From this a vector of shape data will be cerated. Then the data are split with regard to their type.
		While this process all shape vectors will be filled.
		This function must be called before convert or write can be performed.
	*/
	void splitShapeData();

	/*! It evaluates the object and their type from element shape vector from the given GUID.
		\param guid IFC element GUID as string
		\param res Resulting pair of element pointer and its type.
	*/
	bool typeByGuid(const std::string& guid, std::pair<ObjectTypes,std::shared_ptr<ProductShapeData>>& res);
};

} // end namespace

#endif // IFCC_IFCREADER_H
