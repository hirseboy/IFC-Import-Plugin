#ifndef GEGExportPlugin_H
#define GEGExportPlugin_H

#include <SVExportPluginInterface.h>

#include "GEGExportMessageHandler.h"

#define GEGExportPlugin_iid "de.dresden-tu.arch.ibk.GEGExportPlugin/1.0"

class GEGExportPlugin : public QObject, public SVExportPluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID GEGExportPlugin_iid FILE "metadata.json")
	Q_INTERFACES(SVExportPluginInterface)

public:
	explicit GEGExportPlugin(QObject *parent = nullptr);
	virtual bool getProject(QWidget * parent, const QString& projectText) override;
	virtual QString title() const override;
	virtual QString exportMenuCaption() const override;
	virtual void setLanguage(QString langId, QString appname) override;
	bool exportGEGFile(const QString& filename) override;

private:
	GEGExportMessageHandler m_msgHandler;
};

#endif // GEGExportPlugin_H
