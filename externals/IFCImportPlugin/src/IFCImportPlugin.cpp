#include "IFCImportPlugin.h"

#include <IFCC_IFCReader.h>

#include "ImportWizard.h"

IFCImportPlugin::IFCImportPlugin(QObject *parent)
{
}

bool IFCImportPlugin::import(QWidget * parent, VICUS::Project& prj) {
	ImportWizard wz(parent, &m_reader);

	wz.setProject(&prj);

	if (wz.exec() == QDialog::Rejected)
		return false;

	if(m_reader.m_convertCompletedSuccessfully) {
//		m_reader.setVicusProject(&prj);
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
