#ifndef IFCC_ConstructionH
#define IFCC_ConstructionH

#include <vector>

#include <IBK_math.h>

#include <tinyxml.h>

namespace IFCC {

class Construction
{
public:
	Construction();

	int									m_id;
	std::vector<std::pair<int, double>>	m_layers;
	std::string							m_name;

	TiXmlElement * writeXML(TiXmlElement * parent) const;

	friend bool operator==(const Construction& left, const Construction& right) {
		if(left.m_layers.size() != right.m_layers.size())
			return false;

		for(size_t i=0; i<left.m_layers.size(); ++i) {
			if(left.m_layers[i].first != right.m_layers[i].first)
				return false;
			if(!IBK::near_equal(left.m_layers[i].second,right.m_layers[i].second))
				return false;
		}
		return true;
	}
};


} // namespace IFCC

#endif // IFCC_CONSTRUCTION_H
