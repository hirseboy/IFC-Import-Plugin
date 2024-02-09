#include "GEGExportPlugin.h"

#include <IBK_messages.h>
#include <IBK_NotificationHandler.h>

#include <QProgressDialog>

#include <QtExt_Directories.h>
#include <QtExt_LanguageHandler.h>
#include <QtExt_Conversions.h>

#include <VICUS_Project.h>
#include <VICUS_Constants.h>

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
	}
	catch(IBK::Exception& e) {
		return false;
	}

	if(project.m_buildings.size() == 0 || project.m_buildings.size()) {
		return false;
	}

	std::vector<GEGZone>	zones;

	for( const auto& storey : project.m_buildings.front().m_buildingLevels) {
		for( auto room : storey.m_rooms) {
			zones.push_back(GEGZone());
			if(room.m_netFloorArea <= 0)
				room.calculateFloorArea();
			zones.back().m_ANGF = room.m_netFloorArea;
			if(room.m_volume <= 0)
				room.calculateVolume();
			zones.back().m_Vi = room.m_volume;;
			zones.back().m_Ve = room.m_volume;
			if(room.m_idZoneTemplate != VICUS::INVALID_ID) {

			}
		}
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

