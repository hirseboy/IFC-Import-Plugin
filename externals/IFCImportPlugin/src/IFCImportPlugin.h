#ifndef IFCImportPlugin_H
#define IFCImportPlugin_H

#include "IFCImportInterface.h"

#define IFCImportPlugin_iid "de.dresden-tu.arch.ibk.IFCImportPlugin/1.0"

class IFCImportPlugin : public QObject, public IFCImportInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID IFCImportPlugin_iid)

public:
	explicit IFCImportPlugin(QObject *parent = nullptr);
	virtual bool importFunct(VICUS::Project*);
	virtual QString name() const;
	virtual QIcon icon() const;
};

#endif // IFCImportPlugin_H
