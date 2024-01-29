#include "ImportIFCMessageHandler.h"

ImportIFCMessageHandler::ImportIFCMessageHandler(QObject *parent) :
	QObject(parent)
{
}

ImportIFCMessageHandler::~ImportIFCMessageHandler() {
}


void ImportIFCMessageHandler::msg(const std::string& msg,
	IBK::msg_type_t t,
	const char * func_id,
	int verbose_level)
{
	IBK::MessageHandler::msg(msg, t, func_id, verbose_level);
	if (verbose_level > m_requestedConsoleVerbosityLevel)
		return;

	emit msgReceived(t, QString::fromStdString(msg).trimmed());
}

