/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
///\file stdc_kd.c
///OpenKODE based implementation of lowlevel string handling functions.
#include "stdc.h"

#ifdef STDC_KD

#include <stdlib.h>

#ifdef __cplusplus
#define extern "C" {
#endif

char *glb_strcpy (char * destination, const char *source)
{
	int len = kdStrlen (destination);
	kdStrcpy_s (destination, len, source);
	return destination;
}

char *glb_strncpy (char * destination, const char * source, size_t num)
{
	int len = kdStrlen (destination);
	kdStrncpy_s (destination, len, source, num);
	return destination;
}

char *glb_strdup (const char *src)
{
	char *ret;
	int len = kdStrlen (src);
	ret = (char*) kdMalloc (len+1);
	strcpy(ret,src);
	//kdStrcpy_s (ret, len, src);
	return ret;
}


double glb_strtod (const char *nptr, char **endptr)
{
	return strtod (nptr, endptr);
}

#ifdef __cplusplus
}
#endif

#endif
