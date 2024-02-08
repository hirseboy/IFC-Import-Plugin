#ifndef GEGExportMessageHandlerH
#define GEGExportMessageHandlerH

#include <QObject>
#include <IBK_MessageHandler.h>

class GEGExportMessageHandler : public QObject, public IBK::MessageHandler
{
	Q_OBJECT
public:
	explicit GEGExportMessageHandler(QObject *parent = nullptr);
	virtual ~GEGExportMessageHandler();


	/*! Overloaded to received msg info. */
	virtual void msg(const std::string& msg,
		IBK::msg_type_t t = IBK::MSG_PROGRESS,
		const char * func_id = nullptr,
		int verbose_level = -1);

signals:
	/*! Emitted whenever a message was received.
		Shall be connected to the log window to display the message.
	*/
	void msgReceived(int type, QString msg);
};

#endif // GEGExportMessageHandlerH
