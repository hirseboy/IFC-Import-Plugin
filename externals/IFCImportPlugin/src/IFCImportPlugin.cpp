#include "IFCImportPlugin.h"

#include "ImportWizard.h"

IFCImportPlugin::IFCImportPlugin(QObject *parent)
{
}

bool IFCImportPlugin::importFunct(VICUS::Project* prj) {
	ImportWizard wz;

	if (wz.exec() == QDialog::Rejected)
		return false;

	return true;
}

QString IFCImportPlugin::name() const {
	return tr("Import IFC file");
}

QIcon IFCImportPlugin::icon() const {
	QIcon icon;
	return icon;
}

