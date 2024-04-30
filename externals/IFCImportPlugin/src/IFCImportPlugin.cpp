#include "IFCImportPlugin.h"

#include <IBK_messages.h>

#include <IFCC_IFCReader.h>

#include <QtExt_Directories.h>
#include <QtExt_LanguageHandler.h>

#include <QDir>

#include "ImportWizard.h"

IFCImportPlugin::IFCImportPlugin(QObject *parent)
{
}

bool IFCImportPlugin::import(QWidget * parent, QString& projectText) {
	IFCC::IFCReader			reader;

	ImportWizard wz(parent, &reader);

	if (wz.exec() == QDialog::Rejected)
		return false;

	if(reader.m_convertCompletedSuccessfully) {
		reader.setVicusProjectText(projectText);
		m_ifcFileName = QString::fromStdString(reader.filename().str());
		return true;
	}

	m_ifcFileName.clear();
	return false;
}

QString IFCImportPlugin::title() const {
	return tr("Import IFC file");
}

QString IFCImportPlugin::importMenuCaption() const {
	return tr("IFC file ...");
}

void IFCImportPlugin::setLanguage(QString langId, QString appname) {
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
	logfile += "/IFCImportPlugin.log";
	m_msgHandler.openLogFile(logfile, false, errmsg);

	// adjust log file verbosity
	m_msgHandler.setLogfileVerbosityLevel( IBK::VL_DEVELOPER );

	// reset appname to find correct translation file
	QtExt::Directories::appname = "ImportIFCPlugin";
	QtExt::LanguageHandler::instance().installTranslator(langId);
}

QString IFCImportPlugin::IFCFileName() const {
	return m_ifcFileName;
}

