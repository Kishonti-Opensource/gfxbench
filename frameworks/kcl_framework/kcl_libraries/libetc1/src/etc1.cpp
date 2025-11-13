/** \file etc1.cpp
	Definitions of functions supporting ETC1.
*/
#include "etc1.h"

#define RGBA565_RedShift        11
#define RGBA565_GreenShift      5
#define RGBA565_BlueShift       0



inline KCL::uint16 ConvertRGBTo565(KCL::uint8 red, KCL::uint8 grn, KCL::uint8 blu)
{
	return (KCL::uint16)(((KCL::uint16)(red >> 3) << RGBA565_RedShift) | ((KCL::uint16)(grn >> 2) << RGBA565_GreenShift) | ((KCL::uint16)(blu >> 3) << RGBA565_BlueShift));
}

static const KCL::int16 etc1modifiers [][4]=
{
	{-8, -2, 2, 8},
	{-17, -5, 5, 17},
	{-29, -9, 9, 29},
	{-42, -13, 13, 42},
	{-60, -18, 18, 60},
	{-80, -24, 24, 80},
	{-106, -33, 33, 106},
	{-183, -47, 47, 183}
};


//static const float etc1modstdev [8]=
//{
//	5.830951895f,
//	12.52996409f,
//	21.47091055f,
//	31.08858311f,
//	44.29446918f,
//	59.05929224f,
//	78.50159234f,
//	133.6001497f
//};


static const KCL::uint8 etc1reorder[4]=
{
	2, 3, 1, 0
};


//static const KCL::uint8 etc1reorder2[4]=
//{
//	3, 2, 0, 1
//};

void decodeETC1block (const KCL::uint8 *etc1, KCL::uint8 *rgba);
void encodeETC1block (const KCL::uint8 *rgba, KCL::uint8 *etc1);


void DecodeETC1toRGB888 (KCL::uint32 width, KCL::uint32 height, const KCL::uint8 *etc1, KCL::uint8 *rgb)
{
	KCL::uint8 etc1block[8];
	KCL::uint8 rgbablock[64];

	for(KCL::uint32 h = 0; h < height; h+=4)
	{
		for(KCL::uint32 w = 0; w < width;w+=4)
		{
			for(KCL::uint32 p = 0; p < 8; p++) 
			{
				etc1block[p] = etc1[(width*h+w*4)/2+p];
			}
			decodeETC1block (etc1block, rgbablock);

			for(KCL::uint32 h2 = 0; h2 < 4; h2++)
			{
				for(KCL::uint32 w2 = 0; w2 < 4; w2++)
				{
					for(KCL::uint32 c = 0; c < 3; c++) 
					{
						rgb[((h+h2)*width+w+w2)*3+c] = rgbablock[(h2*4+w2)*4+c];
					}
				}
			}
		}
	}
}

void DecodeETC1toRGBA8888 (KCL::uint32 width, KCL::uint32 height, const KCL::uint8 *etc1, KCL::uint8 *rgba)
{
	KCL::uint8 etc1block[8];
	KCL::uint8 rgbablock[64];

	for(KCL::uint32 h = 0; h < height; h+=4)
	{
		for(KCL::uint32 w = 0; w < width;w+=4)
		{
			for(KCL::uint32 p = 0; p < 8; p++) 
			{
				etc1block[p] = etc1[(width*h+w*4)/2+p];
			}
			decodeETC1block (etc1block, rgbablock);

			for(KCL::uint32 h2 = 0; h2 < 4; h2++)
			{
				for(KCL::uint32 w2 = 0; w2 < 4; w2++)
				{
					for(KCL::uint32 c = 0; c < 3; c++) 
					{
						rgba[((h+h2)*width+w+w2)*4+c] = rgbablock[(h2*4+w2)*4+c];
					}

					rgba[((h+h2)*width+w+w2)*4 + 3] = 255;
				}
			}
		}
	}
}

void DecodeETC1toRGB565 (KCL::uint32 width, KCL::uint32 height, const KCL::uint8 *etc1, KCL::uint16 *rgb)
{
	KCL::uint8 etc1block[8];
	KCL::uint8 rgbablock[64];

	for(KCL::uint32 h = 0; h < height; h+=4)
	{
		for(KCL::uint32 w = 0; w < width;w+=4)
		{
			for(KCL::uint32 p = 0; p < 8; p++) 
			{
				etc1block[p] = etc1[(width*h+w*4)/2+p];
			}

			decodeETC1block (etc1block, rgbablock);

			for(KCL::uint32 h2 = 0; h2 < 4; h2++)
			{
				for(KCL::uint32 w2 = 0; w2 < 4; w2++)
				{
					rgb[((h+h2)*width+w+w2)] = ConvertRGBTo565(rgbablock[(h2*4+w2)*4+0],rgbablock[(h2*4+w2)*4+1],rgbablock[(h2*4+w2)*4+2]);
				}
			}
		}
	}
}


void decodeETC1block (const KCL::uint8* etc1, KCL::uint8* rgba)
{
	unsigned char diffbit=(etc1[3] >> 1)& 1;
	unsigned char flipbit= etc1[3] & 1;
	unsigned char codeTable1=(etc1[3] & 0xE0) >> 5;
	unsigned char codeTable2=(etc1[3] & 0x1C) >> 2;

	unsigned char base1RGB[3];
	unsigned char base2RGB[3];


	if(diffbit)
	{
		for(unsigned int i=0;i<3;i++)
		{
			KCL::uint8 etcAndF8 = etc1[i] & 0xF8;
			base1RGB[i]=(etcAndF8)+((etcAndF8) >> 5);
			short temp=((etcAndF8) >> 3)+(etc1[i] & 0x03);
			if(etc1[i] & 0x04)
				temp-=4;
			base2RGB[i]=((temp << 3) & 0xF8)+((temp >> 2) & 0x07);
		}
	}
	else
	{
		for(unsigned int i=0;i<3;i++)
		{
			KCL::uint8 a = etc1[i]&0xF0;
			KCL::uint8 b = etc1[i]&0x0F;
			base1RGB[i]=(a)+((a)>>4);
			base2RGB[i]=(b)+((b)<<4);
		}
	}

	unsigned short table[2];
	table[0]=(((unsigned short)etc1[4])<<8)+etc1[5];
	table[1]=(((unsigned short)etc1[6])<<8)+etc1[7];

	for(unsigned int w=0;w<4;w++)
	{
		for(unsigned int h=0;h<4;h++)
		{
			unsigned int l = w*4+h;
			unsigned int h4w = h*4+w;
			unsigned char t=etc1reorder[((((table[0]>>(l)) & 1) << 1) + ((table[1]>>(l)) & 1))];


			if((flipbit && h<2) || ((!flipbit) && w<2))
			{
				for(unsigned int c=0;c<3;c++)
				{
					short tempc=(short)base1RGB[c]+etc1modifiers[codeTable1][t];
					if(tempc<0)
						tempc=0;
					else if(tempc>255)
						tempc=255;
					rgba[(h4w)*4+c]=(unsigned char)tempc;
				}
			}
			else
			{
				for(unsigned int c=0;c<3;c++)
				{
					short tempc=(short)base2RGB[c]+etc1modifiers[codeTable2][t];
					if(tempc<0)
						tempc=0;
					else if(tempc>255)
						tempc=255;
					rgba[(h4w)*4+c]=(unsigned char)tempc;
				}
			}
			rgba[(h4w)*4+3]=255;
		}
	}
}
