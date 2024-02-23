#include "GEGExportPlugin.h"

#include <IBK_messages.h>
#include <IBK_NotificationHandler.h>

#include <QProgressDialog>

#include <QtExt_Directories.h>
#include <QtExt_LanguageHandler.h>
#include <QtExt_Conversions.h>

#include <VICUS_Project.h>
#include <VICUS_Constants.h>
#include <VICUS_utilities.h>
#include <VICUS_ZoneTemplate.h>

#include <QDir>

#include "GEGZone.h"


class ProgressNotifyer : public IBK::NotificationHandler {
public:
	void notify() override {}
	void notify(double percentage, const char *txt) override;
	std::unique_ptr<QProgressDialog> m_prgDlg;
};

void ProgressNotifyer::notify(double percentage, const char *txt) {
	if(m_prgDlg == nullptr)
		return;

	m_prgDlg->setValue((int)(m_prgDlg->maximum() * percentage));
	m_prgDlg->setLabelText(txt);
	qApp->processEvents();
}

GEGExportPlugin::GEGExportPlugin(QObject *parent)
{
}

bool GEGExportPlugin::getProject(QWidget * parent, const QString& projectText) {
	VICUS::Project project;
	try {
		std::unique_ptr<ProgressNotifyer> notifyer(new ProgressNotifyer);
		notifyer->m_prgDlg.reset(new QProgressDialog(QString(), QString(), 0, 100, nullptr));
		notifyer->m_prgDlg->setWindowTitle(tr("Import project"));
		notifyer->m_prgDlg->setMinimumDuration(0);
		notifyer->notify(0, "");
		project.readImportedXML(projectText, notifyer.get());
		project.updatePointers();
	}
	catch(IBK::Exception& e) {
		return false;
	}

	if(project.m_buildings.size() == 0 || project.m_buildings.size() > 1) {
		return false;
	}

	std::vector<GEGRoom>	rooms;
	std::vector<GEGConstruction>	constructions;
	int	nonValidUsageId = -1;
	int id = 0;
	int surfaceId = 1;

	for( auto& storey : project.m_buildings.front().m_buildingLevels) {
		for( auto& room : storey.m_rooms) {
			rooms.push_back(GEGRoom(id++));
			GEGRoom& currRoom = rooms.back();
			currRoom.set(room, project, nonValidUsageId);

			// get surfaces
			for(const auto& surf : room.m_surfaces) {
				currRoom.m_surfaces.push_back(GEGSurface(surfaceId++));
				GEGSurface& currSurf = currRoom.m_surfaces.back();
				currSurf.m_zoneId = currRoom.m_id;
				GEGConstruction constr = currSurf.set(surf, project);
				if(constr.valid())
					constructions.push_back(constr);
				else {
					if(!constr.m_errors.isEmpty())
						m_errors << constr.m_errors;
				}
			}
		}
	}

	std::map<int,GEGZone> zones;
	for(auto& room : rooms) {
		zones[room.m_zoneTemplateId].m_rooms.push_back(room);
	}

	id = 1;
	for(auto& zoneIt : zones) {
		GEGZone& zone = zoneIt.second;
		zone.m_id = id++;
		zone.update();
		m_zones.push_back(zone);
	}


	return true;
}

QString GEGExportPlugin::title() const {
	return tr("Export GEG file");
}

QString GEGExportPlugin::exportMenuCaption() const {
	return tr("Import IFC file");
}

void GEGExportPlugin::setLanguage(QString langId, QString appname) {
	QtExt::Directories::appname = appname;
	QtExt::Directories::devdir = appname;

	// initialize resources in dependent libraries
	Q_INIT_RESOURCE(QtExt);

	// *** Create log file directory and setup message handler ***
	QDir baseDir;
	baseDir.mkpath(QtExt::Directories::userDataDir());

	IBK::MessageHandlerRegistry::instance().setMessageHandler( &m_msgHandler );
	std::string errmsg;
	std::string logfile = QtExt::Directories::userDataDir().toStdString();
	logfile += "/GEGExportPlugin.log";
	m_msgHandler.openLogFile(logfile, false, errmsg);

	// adjust log file verbosity
	m_msgHandler.setLogfileVerbosityLevel( IBK::VL_DEVELOPER );

	QtExt::Directories::appname = "ImportIFCPlugin";

	QtExt::LanguageHandler::instance().installTranslator(langId);
}

bool GEGExportPlugin::exportGEGFile(const QString& filename) {

	return false;
}

