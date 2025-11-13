/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
/// \file stdc.h
/// Used to define names which make porting the application easier.
#ifndef GLBSTDC_H
#define GLBSTDC_H

#include <stdlib.h>
#include <assert.h>

#ifndef GLB_Assert
#define GLB_Assert(str) assert(0 && (str))
#endif

#ifdef ZUNE_HD
#include <zdk.h>
#define log_zune(...) {char buff0[1024]; TCHAR buff1[1024]; sprintf(buff0, __VA_ARGS__);for(uint32 i=0;i<strlen(buff0);i++){if(buff0[i]=='\n'){buff0[i]=' ';}}wsprintf(buff1,_T("%S"),buff0);ZDKSystem_ShowMessageBox(buff1,MESSAGEBOX_TYPE_OK);while (MESSAGEBOX_STATE_ACTIVE == ZDKSystem_GetMessageBoxState()){ZDKGL_BeginDraw();	Sleep( 100 );ZDKGL_EndDraw();}ZDKSystem_CloseMessageBox();}
#else
#define log_zune(...)
#endif


#ifdef __ANDROID__
extern char* APK_FILE;
#endif

#ifdef OPENKODE
#include <KD/kd.h>

#define STDC_KD

#define GLBFILE		KDFile
#define GLB_EOF		KD_EOF

#define GLB_SEEK_SET	KD_SEEK_SET
#define GLB_SEEK_CUR	KD_SEEK_CUR
#define GLB_SEEK_END	KD_SEEK_END


#define glb_fprintf	kdFprintf
#define glb_fflush	kdFflush
#define glb_fread	kdFread
#define glb_fwrite	kdFwrite
#define glb_fopen	kdFopen
#define glb_fseek	kdFseek
#define glb_ftell	kdFtell
#define glb_fclose	kdFclose
#define glb_fputc	kdPutc
#define glb_ltostr	kdLtostr
#define glb_sprintf	sprintf


#define glb_memcpy	kdMemcpy
#define glb_malloc	kdMalloc
#define glb_memset	kdMemset
#define glb_memmove	kdMemmove
#define glb_free	kdFree
#define glb_realloc	kdRealloc

//#define glb_memcpy	memcpy
//#define glb_malloc	malloc
//#define glb_memset	memset
//#define glb_free	free
//#define glb_realloc	realloc


#define glb_strlen	kdStrlen
#define glb_strcmp	kdStrcmp
#define glb_memcmp	kdMemcmp

#define glb_abs(x)	(x<0)?(-x):(x)
#define glb_fabs	glb_abs

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

double glb_strtod ( const char * str, char ** endptr );
char *glb_strcpy (char * destination, const char *source);
char *glb_strncpy (char * destination, const char * source, size_t num);
char *glb_strdup (const char *src);

#ifdef __cplusplus
}
#endif






#else//not openkode

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#define GLBFILE		FILE
#define GLB_READ	"r"
#define GLB_READBINARY "rb"
#define GLB_WRITE	"w"
#define GLB_APPEND	"a"

#define GLB_SEEK_SET	SEEK_SET
#define GLB_SEEK_CUR	SEEK_CUR
#define GLB_SEEK_END	SEEK_END

#define GLBFILE	FILE
#define GLB_EOF	EOF

#define glb_fread	fread
#define glb_fwrite	fwrite
#define glb_fputc	fputc
#define glb_fopen	fopen
#define glb_fseek	fseek
#define glb_ftell	ftell
#define glb_fclose	fclose

#define glb_fprintf	fprintf
#define glb_fflush	fflush
#define glb_ltostr	ltostr




#define glb_memcpy	memcpy
#define glb_malloc	malloc
#define glb_memset	memset
#define glb_memmove memmove
#define glb_free	free
#define glb_realloc	realloc

#define glb_strlen	strlen
#ifdef __SYMBIAN32__
#define glb_strdup strdup
#elif __linux__
#define glb_strdup	strdup
#elif __ANDROID__
#define glb_strdup strdup
#elif IPHONE || __MACH__ && __APPLE__
#define glb_strdup	strdup
#elif EMSCRIPTEN
#define glb_strdup strdup
#else
#define glb_strdup	_strdup
#endif

#define glb_sprintf	sprintf
#define glb_strcmp	strcmp
#define glb_memcmp	memcmp

#define glb_strtod	strtod
#define glb_strcpy	strcpy
#define glb_strncpy	strncpy

#define glb_abs(x)	(x<0)?(-x):(x)
#define glb_fabs	glb_abs

#endif//openkode
#endif
