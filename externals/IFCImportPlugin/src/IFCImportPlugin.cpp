#include "IFCImportPlugin.h"

#include <IBK_messages.h>

#include <IFCC_IFCReader.h>

#include <QtExt_Directories.h>
#include <QtExt_LanguageHandler.h>

#include "ImportWizard.h"
#include "ImportIFCMessageHandler.h"

IFCImportPlugin::IFCImportPlugin(QObject *parent)
{
	ImportIFCMessageHandler msgHandler;
	IBK::MessageHandlerRegistry::instance().setMessageHandler( &msgHandler );
	std::string errmsg;
	msgHandler.openLogFile(QtExt::Directories::globalLogFile().toStdString(), false, errmsg);
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

void IFCImportPlugin::setLanguage(QString langId) {
	QtExt::LanguageHandler::instance().installTranslator(langId);
}

