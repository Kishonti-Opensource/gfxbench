/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef XML_UTILS_H
#define XML_UTILS_H

#include "tinyxml.h"


TiXmlElement* addelem( TiXmlElement *elem, const char* element_name, float f);
TiXmlElement* addelem( TiXmlElement *elem, const char* element_name, int i);
TiXmlElement* addelem( TiXmlElement *elem, const char* element_name, unsigned int i);
TiXmlElement* addelem( TiXmlElement *elem, const char* element_name, const char* element_text);
TiXmlElement* addelem( TiXmlDocument &doc, const char* element_name, const char* element_text);

TiXmlComment* addcomment( TiXmlElement *elem, const char* element_text);

class xml_writer
{

public:
	xml_writer();
	~xml_writer() {};

	TiXmlElement *xmlroot();
	void save(const char* filename);

protected:
	TiXmlDocument m_doc;
	TiXmlElement *m_xmlroot;

};

#endif
