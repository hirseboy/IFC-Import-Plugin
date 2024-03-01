#include "GEG_Utilities.h"


QString fromMultiLanguageString(const IBK::MultiLanguageString &str, const std::string& preferredLanguage) {
	std::string res = str.string(preferredLanguage);
	if(res.empty())
		res = str.string();
	return QString::fromStdString(res);
}
