/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "xml_utils.h"
#include <kcl_os.h>
#include "kcl_io.h"
#include "glb_kcl_adapter.h"


TiXmlElement* addelem( TiXmlDocument &doc, const char* element_name, const char* element_text)
{
	TiXmlElement * element = new TiXmlElement( element_name );
	TiXmlText * text = new TiXmlText( element_text );
	element->LinkEndChild( text );
	doc.LinkEndChild( element );
	return element;
}


TiXmlElement* addelem( TiXmlElement *elem, const char* element_name, const char* element_text)
{
	TiXmlElement * element = new TiXmlElement( element_name );
	TiXmlText * text = new TiXmlText( element_text );
	element->LinkEndChild( text );
	elem->LinkEndChild( element );

	return element;
}


TiXmlElement* addelem( TiXmlElement *elem, const char* element_name, int i)
{
	char tmp[512] ={0};

	sprintf( tmp, "%d", i);

	TiXmlElement * element = new TiXmlElement( element_name );
	TiXmlText * text = new TiXmlText( tmp );
	element->LinkEndChild( text );
	elem->LinkEndChild( element );

	return element;
}


TiXmlElement* addelem( TiXmlElement *elem, const char* element_name, unsigned int i)
{
	char tmp[512] ={0};

	sprintf( tmp, "%d", i);

	TiXmlElement * element = new TiXmlElement( element_name );
	TiXmlText * text = new TiXmlText( tmp );
	element->LinkEndChild( text );
	elem->LinkEndChild( element );

	return element;
}


TiXmlElement* addelem( TiXmlElement *elem, const char* element_name, float f)
{
	char tmp[512] ={0};

	sprintf( tmp, "%.1f", f);

	TiXmlElement * element = new TiXmlElement( element_name );
	TiXmlText * text = new TiXmlText( tmp );
	element->LinkEndChild( text );
	elem->LinkEndChild( element );

	return element;
}


TiXmlComment* addcomment( TiXmlElement *elem, const char* element_text)
{
	TiXmlComment* element = new TiXmlComment(element_text);
	elem->LinkEndChild( element );
	return element;
}


xml_writer::xml_writer()
{
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "no" );
	m_doc.LinkEndChild( decl );
	m_xmlroot = addelem( m_doc, "GLB_TESTS", "");
}


TiXmlElement *xml_writer::xmlroot()
{
	return m_xmlroot;
}


void xml_writer::save(const char* filename)
{
	KCL::File file(filename, KCL::Write, KCL::RDir);
	m_doc.Print(file.getFilePointer());
}
