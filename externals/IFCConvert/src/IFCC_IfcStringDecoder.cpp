#include "IFCC_IfcStringDecoder.h"

#include <vector>
#include <cstdint>>

namespace IFCC {

// ---- UTF-8 helpers -------------------------------------------------
/*! Takes a Unicode code point (char32_t) and appends it in UTF-8 encoding to a std::string.
	This is necessary because we want to decode code points internally and output UTF-8 at the end.*/
static inline void utf8_append(char32_t cp, std::string &out) {
	if (cp <= 0x7F)
		out.push_back(static_cast<char>(cp));
	else if (cp <= 0x7FF) {
		out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
		out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
	}
	else if (cp <= 0xFFFF) {
		out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
		out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
		out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
	}
	else {
		out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
		out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
		out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
		out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
	}
}

/*! Converts a single hex character (0–9, A–F, a–f) to its numerical value.
 */
static inline int hexval(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
	if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
	return -1;
}

/*! Reads a hex string and converts it to a byte array (std::vector<unsigned char>).
	Sets ok = true if everything is valid.
 */
static std::vector<unsigned char> parse_hex_bytes(std::string_view hex, bool &ok) {
	std::vector<unsigned char> bytes;
	ok = false;
	if (hex.size() % 2)
		return bytes;

	bytes.reserve(hex.size()/2);
	for (size_t i = 0; i < hex.size(); i += 2) {
		int h = hexval(hex[i]);
		int l = hexval(hex[i+1]);
		if (h < 0 || l < 0)
			return bytes;

		bytes.push_back(static_cast<unsigned char>((h<<4) | l));
	}
	ok = true;
	return bytes;
}

/*! Maps each byte value 0–255 directly to the same Unicode code point U+0000–U+00FF and converts it to UTF-8.
 *  This ensures that normal STEP characters are correctly converted from Latin-1 to UTF-8.
 */
static std::string latin1_to_utf8(std::string_view s) {
	std::string out;
	out.reserve(s.size()*2);
	for (unsigned char b : s) {
		utf8_append(static_cast<char32_t>(b), out);
	}
	return out;
}

/*! Reads 2-byte pairs in big-endian format. Also recognizes surrogate pairs (for characters > U+FFFF) and outputs them as UTF-8.
 */
static std::string utf16be_bytes_to_utf8(const std::vector<unsigned char>& bytes) {
	std::string out;
	for (size_t i = 0; i + 1 < bytes.size(); ) {
		uint16_t u = (static_cast<uint16_t>(bytes[i]) << 8) | bytes[i+1];
		i += 2;
		if (u >= 0xD800 && u <= 0xDBFF) { // high surrogate
			if (i + 1 >= bytes.size()) {
				utf8_append(0xFFFD, out);
				break;
			}
			uint16_t l = (static_cast<uint16_t>(bytes[i]) << 8) | bytes[i+1];
			i += 2;
			if (l < 0xDC00 || l > 0xDFFF) {
				utf8_append(0xFFFD, out);
				continue;
			}
			char32_t cp = 0x10000 + (((u - 0xD800) << 10) | (l - 0xDC00));
			utf8_append(cp, out);
		}
		else if (u >= 0xDC00 && u <= 0xDFFF) {
			utf8_append(0xFFFD, out);
		}
		else {
			utf8_append(static_cast<char32_t>(u), out);
		}
	}
	return out;
}

/*! Reads 4-byte values in big-endian format and outputs them as UTF-8.
 */
static std::string utf32be_bytes_to_utf8(const std::vector<unsigned char>& bytes) {
	std::string out;
	for (size_t i = 0; i + 3 < bytes.size(); i += 4) {
		char32_t cp = (static_cast<char32_t>(bytes[i]) << 24) |
					  (static_cast<char32_t>(bytes[i+1]) << 16) |
					  (static_cast<char32_t>(bytes[i+2]) <<  8) |
					  (static_cast<char32_t>(bytes[i+3]));
		if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) cp = 0xFFFD;
		utf8_append(cp, out);
	}
	return out;
}

// ---- STEP/IFC string literal decoder --------------------------------
// Handles: ''  -> '
//          \X\hh\                (Latin-1 byte(s) as hex)
//          \X2\hhhh...\X0\       (UTF-16BE hex stream until \X0\)
//          \X4\hhhhhhhh...\X0\   (UTF-32BE hex stream until \X0\)
/*! This is the actual STEP string decoder.
	It runs through every character in the Latin-1 input and checks:
	\li ‘’ → ' (double quotation marks in STEP format).
	\li \X2\…\X0\ Reads the content between \X2\ and \X0\. Expects hex string, converts to bytes and decodes as UTF-16BE.
	\li \X4\…\X0\ Same as above, but UTF-32BE.
	\li \X\hh\ Reads hex bytes, interprets them as Latin-1 and converts directly to UTF-8.
	\li Default case: Takes the Latin-1 byte and converts it to UTF-8.

	This ensures that all STEP/IFC text encodings are interpreted correctly.
 */
std::string decodeStepIfcString(std::string_view in_latin1) {
	std::string out;
	out.reserve(in_latin1.size()*2);

	auto starts_with = [](std::string_view s, size_t pos, std::string_view pat) {
		return pos + pat.size() <= s.size() && s.substr(pos, pat.size()) == pat;
	};

	for (size_t i = 0; i < in_latin1.size(); ) {
		char c = in_latin1[i];

		// doubled single-quote -> single quote
		if (c == '\'' && i + 1 < in_latin1.size() && in_latin1[i+1] == '\'') {
			out.push_back('\'');
			i += 2;
			continue;
		}

		// \X2\...\X0\   (UTF-16BE)
		if (starts_with(in_latin1, i, "\\X2\\")) {
			size_t j = i + 4;
			size_t end = std::string::npos;
			while (j < in_latin1.size()) {
				if (starts_with(in_latin1, j, "\\X0\\")) { end = j; break; }
				++j;
			}
			if (end != std::string::npos) {
				std::string_view hex = in_latin1.substr(i + 4, end - (i + 4));
				bool ok = false;
				auto bytes = parse_hex_bytes(hex, ok);
				if (ok) out += utf16be_bytes_to_utf8(bytes);
				else    out += latin1_to_utf8(in_latin1.substr(i, end + 4 - i)); // fallback raw
				i = end + 4;
				continue;
			}
		}

		// \X4\...\X0\   (UTF-32BE)
		if (starts_with(in_latin1, i, "\\X4\\")) {
			size_t j = i + 4;
			size_t end = std::string::npos;
			while (j < in_latin1.size()) {
				if (starts_with(in_latin1, j, "\\X0\\")) { end = j; break; }
				++j;
			}
			if (end != std::string::npos) {
				std::string_view hex = in_latin1.substr(i + 4, end - (i + 4));
				bool ok = false;
				auto bytes = parse_hex_bytes(hex, ok);
				if (ok) out += utf32be_bytes_to_utf8(bytes);
				else    out += latin1_to_utf8(in_latin1.substr(i, end + 4 - i));
				i = end + 4;
				continue;
			}
		}

		// \X\hh\ or \X\hhhh...\  (Latin-1 bytes from hex until next '\')
		if (starts_with(in_latin1, i, "\\X\\")) {
			size_t j = i + 3;
			while (j < in_latin1.size() && in_latin1[j] != '\\') ++j;
			if (j < in_latin1.size() && in_latin1[j] == '\\') {
				std::string_view hex = in_latin1.substr(i + 3, j - (i + 3));
				bool ok = false;
				auto bytes = parse_hex_bytes(hex, ok);
				if (ok && !bytes.empty()) {
					// bytes are Latin-1 -> map to Unicode U+00xx
					std::string tmp; tmp.reserve(bytes.size()*2);
					for (unsigned char b : bytes) utf8_append((char32_t)b, tmp);
					out += tmp;
					i = j + 1;
					continue;
				}
			}
			// fallback: copy verbatim if malformed
		}

		// default: treat byte as Latin-1
		utf8_append(static_cast<unsigned char>(c), out);
		++i;
	}

	return out;
}

} // namespace IFCC
