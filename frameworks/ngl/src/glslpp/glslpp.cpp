#include "glslpp.h"

extern "C"
{

#include "glsl/glcpp/glcpp.h"

	void _mesa_error_no_memory(const char *caller)
	{

	}

}


bool PreProcessShaderCode(std::string &src, std::string &log)
{
	gl_extensions extensions;
	memset(&extensions, 0, sizeof(gl_extensions));

	glcpp_parser_t *parser = glcpp_parser_create(&extensions, API_OPENGL_COMPAT);

	glcpp_lex_set_source_string(parser, src.c_str());

	glcpp_parser_parse(parser);

	src = parser->output;

	bool s = (parser->error == 0);
	log = parser->info_log;

	glcpp_parser_destroy(parser);

	return s;
}

