/**
*   XML 포맷 파싱해서 벡터로 반환하는 프로그램
*   @file xmlParsing.cpp
*   @date 2017-05-21
*   @author Lee Hwan-woong
*   @brief XML Parsing
*/

#include <xmlParsing.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <iterator>

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_utils.hpp>

#include <Module.hpp>

using irp::ModuleType;
using irp::ScheduleType;
using irp::ModuleState;
using irp::ModuleAction;

using namespace std;
using namespace rapidxml;

namespace rapidxml {
	//! Represents data loaded from a file
	template<class Ch = char>
	class file
	{
	public:

		//! Loads file into the memory. Data will be automatically destroyed by the destructor.
		//! \param filename Filename to load.
		file(const char *filename)
		{
			using namespace std;

			// Open stream
			basic_ifstream<Ch> stream(filename, ios::binary);
			if (!stream)
				throw runtime_error(string("cannot open file ") + filename);
			stream.unsetf(ios::skipws);
			
			// Determine stream size
			stream.seekg(0, ios::end);
			size_t size = stream.tellg();
			stream.seekg(0);   
			
			// Load data and add terminating 0
			m_data.resize(size + 1);
			stream.read(&m_data.front(), static_cast<streamsize>(size));
			m_data[size] = 0;
		}

		//! Loads file into the memory. Data will be automatically destroyed by the destructor
		//! \param stream Stream to load from
		file(std::basic_istream<Ch> &stream)
		{
			using namespace std;

			// Load data and add terminating 0
			stream.unsetf(ios::skipws);
			m_data.assign(istreambuf_iterator<Ch>(stream), istreambuf_iterator<Ch>());
			if (stream.fail() || stream.bad())
				throw runtime_error("error reading stream");
			m_data.push_back(0);
		}
		
		//! Gets file data.
		//! \return Pointer to data of file.
		Ch *data()
		{
			return &m_data.front();
		}

		//! Gets file data.
		//! \return Pointer to data of file.
		const Ch *data() const
		{
			return &m_data.front();
		}

		//! Gets file data size.
		//! \return Size of file data, in characters.
		std::size_t size() const
		{
			return m_data.size();
		}

	private:

		std::vector<Ch> m_data;   // File data

	};
}

xml_node<>* sNext(char* name, xml_node<> *node) {
	char* _name = name;
	xml_node<> *_node = node;

	while (strcmp(_node->name(), _name)) {
		if (_node->next_sibling() == NULL)
			return NULL;
		_node = _node->next_sibling();
	}
	return _node;
}

xml_node<>* searchNode(char* name, xml_node<> *node) {
	char* _name = name;
	xml_node<> *_node = node;
	if (!(strcmp(_name, _node->name()))) {
		return _node;
	}
	while (strcmp(_node->name(), _name)) {
		_node = _node->first_node();
		if (strcmp(_node->name(), _name)) {
			_node = sNext(_name, _node);
			if (_node == NULL)
				return NULL;
		}
	}
	if (strcmp(_node->name(), _name))
		return NULL;
	else
		return _node;
}

string getName(xml_node<> *node) {
	xml_node<> *_node = node;
	string _name;
	if (xml_node<>* p = searchNode("filename", _node))
		_name = p->value();
	return _name;
}

int getModuleType(xml_node<> *node) {
	xml_node<> *_node = node;
	int _moduletype;
	if (searchNode("moduletype", _node) == NULL)
		return NULL;

	_node = searchNode("moduletype", _node);
	_moduletype = (strcmp((_node->value()), "process"));
	if (_moduletype == 0)
		return ModuleType::PROCESS;
	else
		return ModuleType::THREAD;
}

int getOperationType(xml_node<> *node) {
	xml_node<> *_node = node;
	int _operation;
	if (searchNode("operationtype", _node) == NULL)
		return NULL;

	_node = searchNode("operationtype", _node);
	_operation = (strcmp((_node->value()), "periodic"));

	if( _operation < 0){
		return ScheduleType::NON_REAL;
	} else if ( _operation  == 0){
		return ScheduleType::PERIODIC;
	} else{
		return ScheduleType::SPORADIC;
	}

	//
	// switch (_operation) {
	// case -1:
	// 	return NON_REAL;
	// 	break;
	// case 0:
	// 	return PERIODIC;
	// 	break;
	// case 1:
	// 	return SPORADIC;
	// 	break;
	// }

	return NULL;
}

long long getPeriod(xml_node<> *node) {
	xml_node<> *_node = node;
	long long _period;
	if (searchNode("period", _node) == NULL)
		_period = 0;
	else
		_period = atoll(searchNode("period", _node)->value());

	return _period;
}

int getPrior(xml_node<> * node) {
	xml_node<> *_node = node;
	int _prior;
	if (searchNode("prior", _node) == NULL)
		_prior = 0;
	else
		_prior = atoi(searchNode("prior", _node)->value());

	return _prior;
}

int getDeadline(xml_node<> * node) {
	xml_node<> *_node = node;
	long long _deadline;
	if (searchNode("deadline", _node) == NULL)
		_deadline = 0;
	else
		_deadline = atoll(searchNode("deadline", _node)->value());

	return _deadline;
}

Module makeModule(xml_node<> *node, irp::filesystem::Path const& dir) {
	using namespace irp::filesystem;

	xml_node<> *_node = node;
	long long _period;
	Path _name = getName(_node);
	if (!_name.is_absolute())
		_name = canonical(dir / _name);

	ModuleType _moduleType = ModuleType::type(getModuleType(_node));
	ScheduleType _operationType = ScheduleType::type(getOperationType(_node));
	if (_operationType == ScheduleType::SPORADIC)
		_period = getDeadline(_node);
	else
		_period = getPeriod(_node);
	int _prior = getPrior(_node);


	Module _module(_name, _moduleType, _operationType, _period, _prior);

	if (xml_node<>* pPropertyNode = node->first_node("property"))
	{
		irp::Property& property = _module.property();
		for (xml_node<>* pNode = pPropertyNode->first_node("value")
			; pNode != nullptr; pNode = pNode->next_sibling("value"))
		{
			if (xml_attribute<>* pAttr = pNode->first_attribute("name"))
				property.set(pAttr->value(), pNode->value());
		}
	}

	return _module;
}

vector<Module> makeVectorModule(char* name, xml_node<> *node, irp::filesystem::Path const& dir) {
	vector<Module> vn;
	char* _name = name;
	xml_node<> *_node = node;
	while (_node != NULL) {
		if (searchNode(_name, _node) != NULL) {
			vn.push_back(makeModule(searchNode(_name, _node), dir));
			_node = _node->next_sibling();
		}
	}
	return vn;
}

vector<Module> getModuleFromXML(std::string filename) {
	using namespace irp::filesystem;

	Path parentDir = canonical(Path(filename)).parent_path();

	vector<Module> v;

	rapidxml::file<> xmlFile(filename.c_str());
	rapidxml::xml_document<> doc;
	doc.parse<0>(xmlFile.data());

	xml_node<>* pRoot = doc.first_node();
	if (!pRoot)
		return v;

	if (xml_node<>* pNode = pRoot->first_node("module"))
	{
		// pointing first "module" TAG. doc->root->module
		v = makeVectorModule("module", pNode, parentDir);
	}
	return v;
}
