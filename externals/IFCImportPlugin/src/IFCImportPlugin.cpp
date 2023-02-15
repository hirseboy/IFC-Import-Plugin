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
	ImportWizard wz(parent, &m_reader);

	if (wz.exec() == QDialog::Rejected)
		return false;

	if(m_reader.m_convertCompletedSuccessfully) {
		m_reader.setVicusProjectText(projectText);
		return true;
	}

	return false;
}

QString IFCImportPlugin::title() const {
	return tr("Import IFC file");
}

QString IFCImportPlugin::importMenuCaption() const {
	return tr("Import IFC file");
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

	QtExt::LanguageHandler::instance().installTranslator(langId);
}

