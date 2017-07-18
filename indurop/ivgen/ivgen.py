#!/usr/bin/env python

import sys
import re
import os
import tokenize
import enum
from os.path import expanduser

path = sys.argv[0]
variableNames = []

extension = ".ivd"
variableDir = "./"
variableDirection = "INOUT"

for arg in sys.argv[1:] :

	if arg[0] == "-" :
		if arg[:5] == "-dir=" :
			variableDir = expanduser(arg[5:])
		elif {"-IN" : 1, "-OUT" : 1, "-INOUT" : 1}.get(arg.upper()) :
			variableDirection = arg[1:].upper()
		continue

	variableNames.append(arg)

if not variableDir :
	variableDir = "." + os.path.sep
elif variableDir[-1] != os.path.sep :
	variableDir += os.path.sep

class Type(enum.Enum) :
	PLACEHOLD = 0 
	PRIMI = 1
	ARRAY = 2
	STRUCT = 3
	ALIAS = 4
	NAMESPACE = 5

typeMap = {
	"char" : [Type.PRIMI, "char"], "signed char" : [Type.PRIMI, "int8_t"], "unsigned char" : [Type.PRIMI, "uint8_t"]
	, "short" : [Type.PRIMI, "int16_t"], "signed short" : [Type.PRIMI, "int16_t"], "unsigned short" : [Type.PRIMI, "uint16_t"]
	, "int" : [Type.PRIMI, "int32_t"], "signed int" : [Type.PRIMI, "int32_t"], "unsigned int" : [Type.PRIMI, "uint32_t"]
	, "long" : [Type.PRIMI, "int32_t"], "signed long" : [Type.PRIMI, "int32_t"], "unsigned long" : [Type.PRIMI, "uint32_t"]
	, "long long" : [Type.PRIMI, "int64_t"], "signed long long" : [Type.PRIMI, "int64_t"], "unsigned long long" : [Type.PRIMI, "uint64_t"]

	, "int8_t" : [Type.PRIMI, "int8_t"], "uint8_t" : [Type.PRIMI, "uint8_t"]
	, "int16_t" : [Type.PRIMI, "int16_t"], "uint16_t" : [Type.PRIMI, "uint16_t"]
	, "int32_t" : [Type.PRIMI, "int32_t"], "uint32_t" : [Type.PRIMI, "uint32_t"]
	, "int64_t" : [Type.PRIMI, "int64_t"], "uint64_t" : [Type.PRIMI, "uint64_t"]

	, "float" : [Type.PRIMI, "float"], "double" : [Type.PRIMI, "double"]
}

class Iterator :
	def __init__(self, list) :
		self.index = -1
		self.list = list

	def __bool__(self) :
		return self.index + 1 < len(self.list)

	def __nonzero__(self) :
		return self.__bool__()

	def next(self) :
		self.index += 1
		if self.index < len(self.list) :
			return self.list[self.index]
		self.index = len(self.list)
		return None

	def prev(self) :
		self.index -= 1
		if self.index < 0 :
			self.index = -1
			return None
		return self.list[self.index]

def getDirPath(path) :
	return path[:path.rfind(os.path.sep) + 1]

def createToken(path) :
	varDef = ""
	try :
		file = open(path, "r", encoding="utf8")
		varDef = file.read()
		file.close()
	except :
		raise

	# remove annotations
	varDef = re.sub(r"(\/\/|#).*(?:[\r\n])", "", varDef)
	varDef = varDef.replace(";", "\n")
	return Iterator(re.findall(r"typeof|alias|struct|{|}|\(|\)|\[|\]|[\w\.]+", varDef))

def findType(name, parent) :
	if not parent and parent[0] != Type.STRUCT :
		return

	splited = name.split(".")
	first = splited[0]
	second = ""
	if len(splited) != 1 :
		second = name[len(first) + 1:]

	isArray = False
	if not second :
		matched = re.match(r"(\w+)(?:\[\d*\])", first)
		if matched :
			first = matched.groups()[0]
			isArray = True

	type = []
	for member in parent[1:] :
		if member[1] == first :
			if not second :
				type = member[0]
				break
			return findType(second, member[0])

	if not isArray :
		return type
	elif peelAlias(type)[0] == Type.ARRAY :
		return type[1]
	else :
		raise RuntimeError("'" + first + "' is not a array")

def parseIvdType(tokenItor, basePath = "", parent = [Type.STRUCT]) :
	token = tokenItor.next()
	if not token :
		raise RuntimeError("variable definition is empty")

	type = []
	if token == "typeof" :
		if tokenItor.next() != "(" :
			raise RuntimeError("'typeof' syntax is invalid")
		
		name = tokenItor.next()
		while True :
			token = tokenItor.next()
			if token == ")" :
				tokenItor.prev()
				break
			name += token

		type = findType(name, parent)

		if not type :
			type = typeMap.get(name)
			if not type :
				raise RuntimeError("'" + type + "' is not found")
				placeholder = typeMap[name] = [Type.PLACEHOLD, "placehold"]

		if tokenItor.next() != ")" :
			raise RuntimeError("'typeof' syntax is invalid")

	elif token == "alias" :
		if tokenItor.next() != "(" :
			raise RuntimeError("'alias' syntax is invalid")
		
		name = tokenItor.next()
		while True :
			token = tokenItor.next()
			if token == ")" :
				tokenItor.prev()
				break;
			name += token

		type = findType(name, parent)

		if not type :
			type = typeMap.get(name)
			if not type :
				raise RuntimeError("'" + type + "' is not found")
				placeholder = typeMap[name] = [Type.PLACEHOLD, "placehold"]

		if tokenItor.next() != ")" :
			raise RuntimeError("'alias' syntax is invalid")

		return [Type.ALIAS, type, name]

	elif token == "struct" :
		if tokenItor.next() != "{" :
			raise RuntimeError("'struct' syntax is invalid")

		type = [Type.STRUCT]
		while True :
			subtype = parseIvdType(tokenItor, basePath, type)
			if not subtype :
				raise RuntimeError("cannot parse a member variable's type")

			token = tokenItor.next()
			if not token :
				raise RuntimeError("cannot parse a member variable's name")

			while True :
				if tokenItor.next() == "[" :
					size = int(tokenItor.next())
					if tokenItor.next() != "]" :
						raise RuntimeError("syntax of array is invalid")
					subtype = [Type.ARRAY, subtype, size]
				else :
					tokenItor.prev()
					break

			type.append([subtype, token])

			if tokenItor.next() == "}" :
				break
			tokenItor.prev()

	elif token == "namespace" :
		token = tokenItor.next()
		if not token :
				raise RuntimeError("syntax of namespace is invalid")
		return [Type.NAMESPACE, token]

	else :
		typeName = token
		if token == "signed" or token == "unsigned" :
			token = tokenItor.next()
			typeName += " " + token

		if token == "long" :
			if tokenItor.next() == "long" :
				typeName += " " + "long"
			else :
				tokenItor.prev()

		type = typeMap.get(typeName)
		if not type :
			raise RuntimeError("'" + typeName + "' is a invalid type")

	while True :
		if tokenItor.next() == "[" :
			size = int(tokenItor.next())
			if tokenItor.next() != "]" :
				raise RuntimeError("syntax of array is invalid")
		
			type = [Type.ARRAY, type, size]

		else :
			tokenItor.prev()
			break

	return type

def checkCompleteType(type, typeset = set()) :
	if type[0] == Type.PRIMI :
		return True
	elif type[0] == Type.STRUCT :
		typeset.add(id(type))
		for member in type[1:] :
			if id(member[0]) in typeset :
				return False
			if not checkCompleteType(member[0], typeset) :
				return False
		return True
	elif type[0] == Type.ARRAY :
		return checkCompleteType(type[1])
	return False

def toString(type, name = "", depth = 0) :
	space = ""
	if name :
		space = " "

	if type[0] == Type.PRIMI :
		return "\t" * depth + type[1] + space + name
	elif type[0] == Type.ARRAY :
		return toString(type[1], name, depth) + "[" + str(type[2]) + "]"
	elif type[0] == Type.STRUCT :
		result = "\t" * depth + "struct {\n"
		for member in type[1:] :
			result += toString(member[0], member[1], depth + 1)
			result += ";\n"
		return result + "\t" * depth + "}" + space + name
	elif type[0] == Type.ALIAS :
		return toString(type[1], name, depth)
	return "\t" * depth

def generate2(varName) :
	path = os.path.abspath(variableDir + varName + extension)
	typeName = path[:len(path) - len(extension)]

	type = typeMap[typeName] = [Type.PLACEHOLD, "placehold"]
	type[:] = parseIvdType(createToken(path), getDirPath(path))

	if not checkCompleteType(type) :
		raise RuntimeError("variable type is incomplete")

	print(toString(type, varName[varName.rfind("/") + 1:]) + ";")

	namespaces = varName.split("/")
	name = namespaces[-1]
	namespaces = namespaces[:len(namespaces) - 1]

	outDir = variableDirection
	for dir in namespaces :
		outDir += os.path.sep + dir
	outDir += os.path.sep

	if not os.path.exists(outDir) :
		os.makedirs(outDir)

	cppFile = open(outDir + name + ".cpp", "w", encoding="utf8")
	hppFile = open(outDir + name + ".h", "w", encoding="utf8")

	hppFile.write("\n#pragma once\n\n#include <indurop/indurop.h>\n\n")
	cppFile.write("\n#include \"" + name + ".h\"\n\n")
	for namespace in namespaces :
		hppFile.write("namespace " + namespace + " {\n")
		cppFile.write("namespace " + namespace + " {\n")
	hppFile.write("\n")
	cppFile.write("\n")

	hppFile.write("typedef " + toString(type, name + "VarType") + ";\n\n")
	
	pair = name + ", " + name + "VarType"
	hppFile.write("INDUROP_DECL_VARIABLE(" + pair + ")\n")
	cppFile.write("INDUROP_VARIABLE(" + pair + ")\n")

	hppFile.write("\n}" * len(namespaces) + "\n")
	cppFile.write("\n}" * len(namespaces) + "\n")

	hppFile.close()
	cppFile.close()

def parseType(tokenItor, basePath = "", parent = [Type.STRUCT]) :
	token = tokenItor.next()
	if not token :
		raise RuntimeError("variable definition is empty")

	type = []
	if token == "typeof" :
		if tokenItor.next() != "(" :
			raise RuntimeError("'typeof' syntax is invalid")
		
		token = tokenItor.next()
		if parent and parent[0] == Type.STRUCT :
			for member in parent[1:] :
				if member[1] == token :
					type = member[0]
					break

		if not type :
			path = os.path.abspath(basePath + token + extension)
			name = path[:len(path) - len(extension)]

			type = typeMap.get(name)
			if not type :
				placeholder = typeMap[name] = [Type.PLACEHOLD, "placehold"]
				try :
					type = parseType(createToken(path), getDirPath(path))
					if not type :
						raise RuntimeError("'" + token + "' is not a variable")
					placeholder[:] = type
					type = placeholder
				except :
					raise

		if tokenItor.next() != ")" :
			raise RuntimeError("'typeof' syntax is invalid")

	elif token == "alias" :
		pass

	elif token == "struct" :
		if tokenItor.next() != "{" :
			raise RuntimeError("'struct' syntax is invalid")

		type = [Type.STRUCT]
		while True :
			subtype = parseType(tokenItor, basePath, type)
			if not subtype :
				raise RuntimeError("cannot parse a member variable's type")

			token = tokenItor.next()
			if not token :
				raise RuntimeError("cannot parse a member variable's name")

			type.append([subtype, token])

			if tokenItor.next() == "}" :
				break
			tokenItor.prev()

	else :
		typeName = token
		if token == "signed" or token == "unsigned" :
			token = tokenItor.next()
			typeName += " " + token

		if token == "long" :
			if tokenItor.next() == "long" :
				typeName += " " + "long"
			else :
				tokenItor.prev()

		type = typeMap.get(typeName)
		if not type :
			raise RuntimeError("'" + typeName + "' is a invalid type")

	while True :
		if tokenItor.next() == "[" :
			size = int(tokenItor.next())
			if tokenItor.next() != "]" :
				raise RuntimeError("syntax of array is invalid")
		
			type = [Type.ARRAY, type, size]

		else :
			tokenItor.prev()
			break

	return type

def peelAlias(type) :
	while type[0] == Type.ALIAS :
		if type[1][0] == Type.ARRAY and re.match(r"(\w+)(?:\[\d*\])", type[2]) :
			type = type[1][1]
		else :
			type = type[1]
	return type

def originOfAlias(type, parent) :
	name = ""
	if type[0] == Type.ALIAS :
		matched = re.match(r"(\w+)(?:\[\d*\])?", type[2])
		splited = matched.groups()[0].split(".")

		originType = findType(splited[0], parent)
		if originType[0] != Type.ALIAS :
			return type[2]
		originName = originOfAlias(originType, parent)
		name = type[2].replace(splited[0], originName)
	return name


def writeVariable(variable, hppFile, cppFile, parent) :
	type = variable[0]
	name = variable[1]
	className = "Variable_" + name

	hppFile.write("class " + className + " : public irp::SharedVariable\n")

	hppFile.write("{\n")
	hppFile.write("public:\n")
	value_type = peelAlias(type)
	if value_type[0] == Type.ARRAY :
		hppFile.write("typedef bcc::array<" + toString(value_type[1]) + ", " + str(value_type[2]) + "> value_type;\n\n")
	else :
		hppFile.write("typedef " + toString(value_type, "value_type") + ";\n\n")

	hppFile.write("public:\n")
	hppFile.write("\t" + className + "();\n\n")

	hppFile.write("public:\n")
	if type[0] == Type.STRUCT :
		for member in type[1:] :
			hppFile.write("\tirp::VariableView<" + toString(member[0], "> " + member[1]) + ";\n")
	
	hppFile.write("\n")
	hppFile.write("public:\n")
	hppFile.write("\toperator value_type const&() const { irp::SharedVariable::pull(); return mCache; }\n")
	hppFile.write("\t" + className + "& operator=(value_type const& rhs) { mCache = rhs; irp::SharedVariable::push(); return *this; }\n")

	if type[0] == Type.ARRAY :
		hppFile.write("\tirp::VariableView<value_type::value_type> const operator[](std::size_t i) const { return irp::VariableView<value_type::value_type>(const_cast<" + className + "&>(*this), sizeof(value_type::value_type) * i); } \n")
		hppFile.write("\tirp::VariableView<value_type::value_type> operator[](std::size_t i) { return irp::VariableView<value_type::value_type>(*this, sizeof(value_type::value_type) * i); } \n")
	
	hppFile.write("\n")
	hppFile.write("private:\n")
	hppFile.write("\tvoid onPull(void const* p) const;\n")
	hppFile.write("\tvoid onPush(void* p);\n")
	hppFile.write("\tvoid* cache() const { return &mCache; }\n")

	hppFile.write("\n")
	hppFile.write("private:\n")
	hppFile.write("\tmutable value_type mCache;\n")
	
	hppFile.write("};\n")
	hppFile.write("extern " + className + " " + name + ";\n\n")

	cppFile.write(className + " " + name + ";\n\n")
	cppFile.write("namespace { char const " + className + "_type[] = ")
	if type[0] == Type.STRUCT :
		cppFile.write("\"")
		for member in type[1:] :
			cppFile.write(toString(member[0]).replace("\n", "\\n") + " " + member[1] + "\\n")
		cppFile.write("\";")
	elif type[0] == Type.ARRAY :
		cppFile.write("\"" + toString(type).replace("\n", "\\n") + "\";")
	elif type[0] == Type.ALIAS :
		cppFile.write("\"alias(" + originOfAlias(type, parent) +")\";")
	else :
		cppFile.write("\"" + toString(type).replace("\n", "\\n") + "\";")
	cppFile.write(" }\n")

	cppFile.write(className + "::" + className + "()\n")
	cppFile.write("\t: irp::SharedVariable(\"")
	cppFile.write(name + "\", ")
	cppFile.write(className + "_type, ")
	cppFile.write("sizeof(value_type)), mCache()\n")
	cppFile.write("{\n")

	if type[0] == Type.STRUCT :
		for member in type[1:] :
			if member[0][0] == Type.ARRAY :
				cppFile.write("\tfor (std::size_t i = 0, size = irp::detail::length(" + member[1] + "); i < size; ++i)\n")
				cppFile.write("\t\tthis->" + member[1] + "[i] = irp::detail::ViewInitializer(*this, ")
				cppFile.write("offsetof(value_type, " + member[1] + ") + i * sizeof(*value_type::" + member[1] + "));\n")
			else :
				cppFile.write("\t" + member[1] + " = irp::detail::ViewInitializer(*this, ")
				cppFile.write("offsetof(value_type, " + member[1] + "));\n")

	cppFile.write("}\n")


	offsetExpr = "0"
	if type[0] == Type.ALIAS :
		matched = re.match(r"([\w\.]+)(?:\[(\d+)\])?", type[2])
		if not matched :
			raise RuntimeError("alias fails")
		groups = matched.groups()
		splited = groups[0].split(".")

		originClassName = "Variable_" + splited[0]
		if len(splited) == 2:
			offsetExpr = "offsetof(" + originClassName + "::value_type, " + splited[1] + ")"

		if groups[1] :
			offsetExpr += " + sizeof(value_type) * " + groups[1]

	cppFile.write("\n")
	cppFile.write("void " + className + "::onPull(void const* p) const\n")
	cppFile.write("{\n")
	cppFile.write("\tp = reinterpret_cast<char const*>(p) + " + offsetExpr + ";\n")
	cppFile.write("\tmCache = *reinterpret_cast<value_type const*>(p);\n")
	cppFile.write("}\n")

	cppFile.write("\n")
	cppFile.write("void " + className + "::onPush(void* p)\n")
	cppFile.write("{\n")
	cppFile.write("\tp = reinterpret_cast<char*>(p) + " + offsetExpr + ";\n")
	cppFile.write("\t*reinterpret_cast<value_type*>(p) = mCache;\n")
	cppFile.write("}\n")

def generate(path) :
	token = createToken(os.path.abspath(variableDir + path))
	variables = [Type.STRUCT]

	namespace = ""
	while token :
		type = parseIvdType(token, getDirPath(path), variables)
		if type[0] == Type.NAMESPACE :
			namespace = type[1]
			continue
	
		name = token.next()
		while True :
			if token.next() == "[" :
				size = int(token.next())
				if token.next() != "]" :
					raise RuntimeError("syntax of array is invalid")
				type = [Type.ARRAY, type, size]
			else :
				token.prev()
				break

		variables.append([type, name])


	if not namespace :
		namespace = []
	else :
		namespace = namespace.split(".")

	name = path.split("/")[-1]
	name = name[:name.rfind(".")]

	outDir = variableDirection + "/"

	if not os.path.exists(outDir) :
		os.makedirs(outDir)

	cppFile = open(outDir + name + ".cpp", "w", encoding="utf8")
	hppFile = open(outDir + name + ".h", "w", encoding="utf8")

	hppFile.write("\n#pragma once\n\n#include <indurop/indurop.h>\n\n")
	cppFile.write("\n#include \"" + name + ".h\"\n\n")
	for name in namespace :
		hppFile.write("namespace " + name + " {\n")
		cppFile.write("namespace " + name + " {\n")
	hppFile.write("\n")
	cppFile.write("\n")

	for variable in  variables[1:] :
		writeVariable(variable, hppFile, cppFile, variables)

	hppFile.write("}" * len(namespace) + "\n")
	cppFile.write("}" * len(namespace) + "\n")

	hppFile.close()
	cppFile.close()


print(variableNames)
fileNameRegex = re.compile(r".*\..*$")
for variable in variableNames :
	if fileNameRegex.match(variable) :
		generate(variable)
	else :
		generate2(variable)
