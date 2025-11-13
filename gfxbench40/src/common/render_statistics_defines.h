/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
/***********************************************************************************************/
/***********************************************************************************************/
/*                                                                                             */
/* Platform and speed independent statistics measurement control.                              */
/* Enables statistics logging in app.cpp!                                                      */
/*                                                                                             */
/* You can enable measurement of the following statistics                                      */
/*   1.   count of used textures per frame                                                     */
/*   2.a) count of passed samples per frame                                                    */
/*   2.b) overdraw rate per frame                                                              */
/*   2.c) texture read memory read bandwidth per frame                                         */
/*   3.   shader program instruction count per frame                                           */
/*	 4.	  pixel per primitive (coverage)                                					   */
/*                                                                                             */
/* Please note, that enabling any of the mentioned statistics will slow rendering, thus the    */
/* measurement of frame time and FPS will be "meaningless" in that case.                       */
/*                                                                                             */
/* Also please note, that points 2.x), 3), and 4) needs occlusion query.                       */
/* This is implemented assuming Windows environment with glew!                                 */
/*                                                                                             */
/* Pixel coverage measurements turn off other occlusion-based stats.                              */
/* It renders solids w/o depth test.                                                           */
/*                                                                                             */
/* -----------------                                                                           */
/* IMPORTANT NOTICE:                                                                           */
/* -----------------                                                                           */
/* point 3. --"shader program instruction count"-- assumes that on your computer "cgc.exe"     */
/* (publicly available tool from Nvidia, all rights belongs to them) is available!             */
/* You have to set the path of "cgc.exe" in this file (see below)!                             */
/*                                                                                             */
/***********************************************************************************************/
/***********************************************************************************************/


#ifndef RENDER_STATISTICS_DEFINES_H
#define RENDER_STATISTICS_DEFINES_H

//1.
//#define TEXTURE_COUNTING

//2.x)
//#define OCCLUSION_QUERY_BASED_STAT

//3.
//#define CALCULATE_SHADER_INSTRUCTION_COUNT

//4.
//#define MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE

#if defined MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE || defined OCCLUSION_QUERY_BASED_STAT
#define STATISTIC_SAMPLES m_glGLSamplesPassedQuery
#else
#define STATISTIC_SAMPLES 
#endif


#if defined(CALCULATE_SHADER_INSTRUCTION_COUNT) || defined(MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE)

#ifndef OCCLUSION_QUERY_BASED_STAT
#define OCCLUSION_QUERY_BASED_STAT
#endif

#endif

#ifdef WIN32
#define MY_CGC_PATH "C:\\_CGC_\\"
#else //we believe we are on LINUX
#define MY_CGC_PATH "/_CGC_/"
#endif


#if defined TEXTURE_COUNTING || defined OCCLUSION_QUERY_BASED_STAT
#define STATISTICS_LOGGING_ENABLED 1
#else
#define STATISTICS_LOGGING_ENABLED 0
#endif


#endif //RENDER_STATISTICS_DEFINES_H
