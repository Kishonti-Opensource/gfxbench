#ifndef HDR_H
#define HDR_H

/***********************************************************************************
Created:	17:9:2002
FileName: 	hdrloader.h
Author:		Igor Kravtchenko

Info:		Load HDR image and convert to a set of float32 RGB triplet.
************************************************************************************/

#include <string>

class HDRLoaderResult {
public:
	int width, height;
	// each pixel takes 3 float32, each component can be of any value...
	float *cols;

	HDRLoaderResult()
	{
		width = 0;
		height = 0;
		cols = 0;
	}
};

class HDRLoader {
public:
	static bool load(const std::string &fileName, HDRLoaderResult &res);
};

#endif