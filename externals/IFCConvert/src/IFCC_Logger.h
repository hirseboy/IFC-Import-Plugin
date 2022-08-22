#ifndef LoggerH
#define LoggerH

#include <fstream>

namespace IFCC {


class Logger {
public:
	static Logger& instance() {
		static Logger log;
		return log;
	}

	inline void set(const std::string& filename) {
		m_out.open(filename);
		m_opened = m_out.is_open();
	}

	template<typename T>
	inline Logger& operator<<(const T& msg) {
		if(m_opened) {
			m_out << msg << std::endl;
		}
		return *this;
	}

private:
	bool m_opened = false;
	std::ofstream m_out;

};

} // end namespace

#endif // LogggerH
