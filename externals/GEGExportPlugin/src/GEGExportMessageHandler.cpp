#include "GEGExportMessageHandler.h"

GEGExportMessageHandler::GEGExportMessageHandler(QObject *parent) :
	QObject(parent)
{
}

GEGExportMessageHandler::~GEGExportMessageHandler() {
}


void GEGExportMessageHandler::msg(const std::string& msg,
	IBK::msg_type_t t,
	const char * func_id,
	int verbose_level)
{
	IBK::MessageHandler::msg(msg, t, func_id, verbose_level);
	if (verbose_level > m_requestedConsoleVerbosityLevel)
		return;

	emit msgReceived(t, QString::fromStdString(msg).trimmed());
}

