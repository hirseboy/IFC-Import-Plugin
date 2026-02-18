#ifndef IFCC_ProgressHandlerH
#define IFCC_ProgressHandlerH

#include <functional>
#include <string>

#include <QString>

#include <IBK_NotificationHandler.h>

namespace IFCC {

/*! Bridges IBK::NotificationHandler to a std::function callback with sub-range mapping.
	Maps local 0.0-1.0 progress to a configurable sub-range of the global 0-100 scale.
	Throttles updates to minimum 1% delta to avoid excessive processEvents() calls.
*/
class ProgressHandler : public IBK::NotificationHandler {
public:
	/*! Constructor.
		\param callback Function receiving global percentage [0..100] and status text.
		\param rangeStart Start of the global sub-range in [0..1].
		\param rangeEnd End of the global sub-range in [0..1].
	*/
	ProgressHandler(std::function<void(int, QString)> callback, double rangeStart, double rangeEnd);

	/*! Notification without progress value - emits at current range start. */
	void notify() override;

	/*! Notification with local percentage in [0..1]. */
	void notify(double percentage) override;

	/*! Notification with local percentage in [0..1] and status text. */
	void notify(double percentage, const char* text) override;

	/*! Set a new sub-range for subsequent calls. Resets throttle state.
		\param rangeStart Start of the global sub-range in [0..1].
		\param rangeEnd End of the global sub-range in [0..1].
	*/
	void setRange(double rangeStart, double rangeEnd);

private:
	std::function<void(int, QString)>	m_callback;
	double								m_rangeStart;	///< Global range start in [0..1].
	double								m_rangeEnd;		///< Global range end in [0..1].
	int									m_lastReported;	///< Last reported global percentage for throttling.
};

} // namespace IFCC

#endif // IFCC_ProgressHandlerH
