#ifndef GEG_UtilitiesH
#define GEG_UtilitiesH

#include <QString>

#include <IBK_MultiLanguageString.h>

QString fromMultiLanguageString(const IBK::MultiLanguageString& str, const std::string& preferredLanguage);

#endif // GEG_UtilitiesH
