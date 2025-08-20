#ifndef IFCIMPORTINTERFACE_H
#define IFCIMPORTINTERFACE_H

#include <QtPlugin>
#include <QIcon>


#define IFCImportInterface_iid "de.dresden-tu.arch.ibk.IFCImportInterface/1.1"

namespace VICUS {
	class Project;
}

class IFCImportInterface
{
public:
	IFCImportInterface()
	{}
	virtual ~IFCImportInterface() {}
	virtual bool importFunct(VICUS::Project*) = 0;
	virtual QString name() const = 0;
	virtual QIcon icon() const = 0;
};

Q_DECLARE_INTERFACE(IFCImportInterface, IFCImportInterface_iid)

#endif // IFCIMPORTINTERFACE_H
