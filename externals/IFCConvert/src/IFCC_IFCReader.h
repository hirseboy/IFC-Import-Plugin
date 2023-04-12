#ifndef IFCC_IFCReaderH
#define IFCC_IFCReaderH

#include <IBK_assert.h>
#include <IBK_Path.h>

#include <IBKMK_Polygon3D.h>

#include <ReaderSTEP.h>

#include <ifcpp/IFC4X3/include/IfcSpaceTypeEnum.h>

#include <QCoreApplication>
#include <QStringList>

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
#include "IFCC_BuildingElementsCollector.h"

#include <QProgressDialog>

namespace IFCC {

/*! Main class for IFC reading and conversion int vicus model.*/
class IFCReader : public QObject {
Q_OBJECT

public:

	enum Name_Id_Type {
		NIT_Space,
		NIT_SpaceBoundary
	};

	/*! Struct contains flags or parameter for controlling conversion.
		It should repair possible IFC problems.
	*/
	struct RepairFlags {
		bool	m_flipPolygons		= false;		///< if true all polygones will be flipped before writing to xml
		bool	m_removeDoubledSBs	= false;		///< remove space boundaries which contains the same surface

	};

	/*! Standard constructor. Initializes geometry converter.*/
	IFCReader();

	~IFCReader();

	/*! Clear complete data structure.*/
	void clear();

	/*! Clear all data sets filled by convert function.*/
	void clearConvertData();

	/*! Read the IFC file.*/
	bool read(const IBK::Path& filename, bool ignoreReadError);

	/*! Convert IFC data into internal format similar to SimVicus.
		It needs calling of read before. splitShapeData is called internally.
		When space boundaries exist they are used in order to create component instances.
		Otherwise space boundaries will be created by using a matching algorithm from building components to spaces.
		\param useSpaceBoundaries When true space boundaries will be used if the exist.
	*/
	bool convert(bool useSpaceBoundaries);

	/*! Return the total number of IFC entities. Call read before use.*/
	int totalNumberOfIFCEntities() const;

	/*! Return the total number of IFC space boundaries. Call read before use.*/
	int numberOfIFCSpaceBoundaries() const;

	/*! Check if the essential IFC objects are present after reading.
		Read must be done before and successful.
	*/
	bool checkEssentialIFCs(QString& errmsg, int& buildings, int& spaces);

	/*! Check if a space contains equal space boundaries.
		\param equalSBs Contain ids of equal space boundaries
		\return Number of qual space boundaries
	*/
	int checkForEqualSpaceBoundaries(std::vector<std::pair<int,int>>& equalSBs) const;

	/*! Check if a subsurface space boundary is contained more than once in construction space boundaries of a space.
		\param res Vector of space ids with corresponding multiple used subsurface space boundary ids
		\return Number of spaces with equal subsurfaces
	*/
	int checkForUniqueSubSurfacesInSpaces(std::vector<std::pair<int,std::vector<int>>>& res) const;

	/*! Check if the surfaces of two spaces are intersected. If this is the case two spaces have a common volume.
		This should never be the case and is an index for a faulty IFC file.
		\return Set of pairs of intersected spaces given by their ids
	*/
	std::set<std::pair<int,int>> checkForIntersectedSpace() const;

	/*! Return state of flip polygon.*/
	bool flipPolygons() const;

	/*! Set the state of flip polygon flag.
		If this is true all polygons of the space boundaries will be flipped before they are saved.
	*/
	void setFlipPolygons(bool flipPolygons);

	/*! Return state of remove doubled space boundaries flag.*/
	bool removeDoubledSBs() const;

	/*! Set the state of the remove doubled space boundaries flag.
		If true one of the two doubled space boundaries will be removed before the space is saved.
	*/
	void setRemoveDoubledSBs(bool removeDoubledSBs);

	/*! Search for object with given id and type and return its name.*/
	QString nameForId(int id, Name_Id_Type) const;

	/*! Write converted data as vicus file.*/
	void writeXML(const IBK::Path & filename) const;

	/*! Create a VICUS project text based on the current content.*/
	void setVicusProjectText(QString& projectText);

	QStringList messages() const;

	QStringList statistic() const;

	bool							m_hasError;				///< If true an error while reading IFC file was occured
	bool							m_hasWarning;			///< If true an warning while reading IFC file was occured
	std::string						m_errorText;			///< Text of error messages
	std::string						m_warningText;			///< Text of warning messages
	std::string						m_progressText;			///< Progress text
	bool							m_readCompletedSuccessfully;
	bool							m_convertCompletedSuccessfully;

	const std::vector<ConvertError>& convertErrors() const;

signals:
	void progress(int val, QString text);

private slots:
	void setProgress(int val, QString text);

private:

	void updateBuildingElements();

	IBK::Path						m_filename;				///< IFC file
	std::shared_ptr<BuildingModel>	m_model;				///< IFC model created from file
	RepairFlags						m_repairFlags;			///< Contains all flags and parameter for IFC repair
	GeometryConverter				m_geometryConverter;	///< Geometry converter for converting local to global coordinates.
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

	/*! Collection of building elements.*/
	BuildingElementsCollector		m_buildingElements;

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
	/*! Vector of errors while converting.*/
	std::vector<ConvertError>							m_convertErrors;


	/*! Function for collecting messages from IFC reading process (error, warning, progress).*/
	static void messageTarget( void* obj_ptr, shared_ptr<StatusCallback::Message> t );

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
	bool typeByGuid(const std::string& guid, std::pair<BuildingElementTypes,std::shared_ptr<ProductShapeData>>& res);

	bool		m_useSpaceBoundaries = true;

	std::unique_ptr<QProgressDialog> m_progressDialog;
};

} // end namespace

#endif // IFCC_IFCREADER_H
