#pragma once

#include <string>
#include <vector>

#include <rapidxml/rapidxml.hpp>
#include <indurop/detail/file_system.h>

class Module;

//xml_node<>* sFirst(char* name, xml_node<> *node);
//xml_node<>* sParent(char* name, xml_node<> *node);
rapidxml::xml_node<>* sNext(char* name, rapidxml::xml_node<> *node);
rapidxml::xml_node<>* searchNode(char* name, rapidxml::xml_node<> *node);
int getModuleType(rapidxml::xml_node<> *node);
int getOperationType(rapidxml::xml_node<> *node);
std::string getName(rapidxml::xml_node<> *node);
long long getPeriod(rapidxml::xml_node<> *node);
int getDeadline(rapidxml::xml_node<> * node);
Module makeModule(rapidxml::xml_node<> *node, irp::filesystem::Path const& dir = irp::filesystem::Path());
std::vector<Module> makeVectorModule(char* name, rapidxml::xml_node<>* node, irp::filesystem::Path const& dir = irp::filesystem::Path());
std::vector<Module> getModuleFromXML(std::string filename);
