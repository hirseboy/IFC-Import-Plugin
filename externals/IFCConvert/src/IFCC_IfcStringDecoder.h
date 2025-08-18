#ifndef IFCC_IfcStringDecoderH
#define IFCC_IfcStringDecoderH

#include <string>

namespace IFCC {

std::string decodeStepIfcString(std::string_view in_latin1);

} // namespace IFCC

#endif // IFCC_IfcStringDecoderH
