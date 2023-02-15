#ifndef IFCImportPlugin_H
#define IFCImportPlugin_H

#include <SVImportPluginInterface.h>

#include <IFCC_IFCReader.h>

#include "ImportIFCMessageHandler.h"

#define IFCImportPlugin_iid "de.dresden-tu.arch.ibk.IFCImportPlugin/1.0"

class IFCImportPlugin : public QObject, public SVImportPluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID IFCImportPlugin_iid)
	Q_INTERFACES(SVImportPluginInterface)

public:
	explicit IFCImportPlugin(QObject *parent = nullptr);
	virtual bool import(QWidget * parent, QString& projectText) override;
	virtual QString title() const override;
	virtual QString importMenuCaption() const override;
	virtual void setLanguage(QString langId, QString appname) override;

private:
	IFCC::IFCReader			m_reader;
	ImportIFCMessageHandler m_msgHandler;
};

#endif // IFCImportPlugin_H
