/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MISC2_H
#define MISC2_H

#include <kcl_base.h>

#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>


#define FLIP_IMAGE 1
#define NO_FLIP_IMAGE 0

union endianConverterShort
{
	unsigned short v;
	struct
	{
		unsigned char a;
		unsigned char b;
	};
};

union endianConverterInt
{
	unsigned int v;
	struct
	{
		unsigned char a;
		unsigned char b;
		unsigned char c;
		unsigned char d;
	};
};

void CheckDataDirectoryValidity();
bool IsBigEndian( void );
unsigned int fgetReverseUINT( FILE *f );

unsigned int convertInt(unsigned int i);
unsigned short convertShort(unsigned short i);

//void saveBmp( const char *name, int w, int h, void *data);
bool savePng( const char *name, int w, int h, const unsigned char* data, int flip = 0);
void convertRGBAtoBGR( unsigned char* data, int pixels);

void printSrc (FILE *f, const char *src);
void CopyFiles( FILE* to, FILE* from);
bool GetNativeResolution(int &w, int &h);

void EncodeBase64URL(std::string &result, const void* data, size_t sz, bool usePercentEncodedPadding=false);


template<typename To, typename From>
To lexical_cast(From const &from)
{
	To to;
	std::stringstream os;	
	os << from;
	os >> to;
	
	return to;  
}


template<class T>
void push_back_multiple_bytes(std::vector<signed char> &destination, const T& data)
{
	const size_t N = sizeof(T);

	const char* it = (const char*)(&data);

	for(size_t i=0; i<N; ++i)
	{
		destination.push_back( *it++ );
	}
}


void encrypt(const unsigned int length, char*& data);




class CombineData
{
public:
	void Put(const unsigned int x, const unsigned int y, const unsigned int n, const KCL::int8 data);
	KCL::int8 Get(const unsigned int x, const unsigned int y, const unsigned int n);

	void Create(const unsigned int x, const unsigned int y, const unsigned int componentSize);

	KCL::int8*& GetData();

	CombineData();
	~CombineData();
private:
	KCL::uint8 m_component_num;
	KCL::uint16 m_width;
	KCL::uint16 m_height;

	KCL::int8 *m_data;
};




#endif
