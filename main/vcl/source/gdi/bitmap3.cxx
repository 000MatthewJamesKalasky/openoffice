/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <stdlib.h>

#include <vcl/bmpacc.hxx>
#include <vcl/octree.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bitmap.hxx>

#include <impoct.hxx>
#include <impvect.hxx>

// -----------
// - Defines -
// -----------

#define RGB15( _def_cR, _def_cG, _def_cB )	(((sal_uLong)(_def_cR)<<10UL)|((sal_uLong)(_def_cG)<<5UL)|(sal_uLong)(_def_cB))
#define GAMMA( _def_cVal, _def_InvGamma )	((sal_uInt8)MinMax(FRound(pow( _def_cVal/255.0,_def_InvGamma)*255.0),0L,255L))
#define MAP( cVal0, cVal1, nFrac )	((sal_uInt8)((((long)(cVal0)<<7L)+nFrac*((long)(cVal1)-(cVal0)))>>7L))

#define CALC_ERRORS																\
						nTemp   = p1T[nX++] >> 12;								\
						nBErr = MinMax( nTemp, 0, 255 );						\
						nBErr = nBErr - FloydIndexMap[ nBC = FloydMap[nBErr] ];	\
						nTemp   = p1T[nX++] >> 12;								\
						nGErr = MinMax( nTemp, 0, 255 );						\
						nGErr = nGErr - FloydIndexMap[ nGC = FloydMap[nGErr] ];	\
						nTemp   = p1T[nX] >> 12;								\
						nRErr = MinMax( nTemp, 0, 255 );						\
						nRErr = nRErr - FloydIndexMap[ nRC = FloydMap[nRErr] ];

#define CALC_TABLES3										\
						p2T[nX++] += FloydError3[nBErr];	\
						p2T[nX++] += FloydError3[nGErr];	\
						p2T[nX++] += FloydError3[nRErr];

#define CALC_TABLES5										\
						p2T[nX++] += FloydError5[nBErr];	\
						p2T[nX++] += FloydError5[nGErr];	\
						p2T[nX++] += FloydError5[nRErr];

#define CALC_TABLES7										\
						p1T[++nX] += FloydError7[nBErr];	\
						p2T[nX++] += FloydError1[nBErr];	\
						p1T[nX] += FloydError7[nGErr];		\
						p2T[nX++] += FloydError1[nGErr];	\
						p1T[nX] += FloydError7[nRErr];		\
						p2T[nX] += FloydError1[nRErr];

// -----------
// - Statics -
// -----------

sal_uLong nVCLRLut[ 6 ] = { 16, 17, 18, 19, 20, 21 };
sal_uLong nVCLGLut[ 6 ] = { 0, 6, 12, 18, 24, 30 };
sal_uLong nVCLBLut[ 6 ] = { 0, 36, 72, 108, 144, 180 };

// ------------------------------------------------------------------------

sal_uLong nVCLDitherLut[ 256 ] =
{
       0, 49152, 12288, 61440,  3072, 52224, 15360, 64512,   768, 49920, 13056,
   62208,  3840, 52992, 16128, 65280, 32768, 16384, 45056, 28672, 35840, 19456,
   48128, 31744, 33536, 17152, 45824, 29440, 36608, 20224, 48896, 32512, 8192,
   57344,  4096, 53248, 11264, 60416,  7168, 56320,  8960, 58112,  4864, 54016,
   12032, 61184,  7936, 57088, 40960, 24576, 36864, 20480, 44032, 27648, 39936,
   23552, 41728, 25344, 37632, 21248, 44800, 28416, 40704, 24320, 2048, 51200,
   14336, 63488,  1024, 50176, 13312, 62464,  2816, 51968, 15104, 64256,  1792,
   50944, 14080, 63232, 34816, 18432, 47104, 30720, 33792, 17408, 46080, 29696,
   35584, 19200, 47872, 31488, 34560, 18176, 46848, 30464, 10240, 59392,  6144,
   55296,  9216, 58368,  5120, 54272, 11008, 60160,  6912, 56064,  9984, 59136,
    5888, 55040, 43008, 26624, 38912, 22528, 41984, 25600, 37888, 21504, 43776,
   27392, 39680, 23296, 42752, 26368, 38656, 22272,   512, 49664, 12800, 61952,
    3584, 52736, 15872, 65024,   256, 49408, 12544, 61696,  3328, 52480, 15616,
   64768, 33280, 16896, 45568, 29184, 36352, 19968, 48640, 32256, 33024, 16640,
   45312, 28928, 36096, 19712, 48384, 32000,  8704, 57856,  4608, 53760, 11776,
   60928,  7680, 56832,  8448, 57600,  4352, 53504, 11520, 60672,  7424, 56576,
   41472, 25088, 37376, 20992, 44544, 28160, 40448, 24064, 41216, 24832, 37120,
   20736, 44288, 27904, 40192, 23808,  2560, 51712, 14848, 64000,  1536, 50688,
   13824, 62976,  2304, 51456, 14592, 63744,  1280, 50432, 13568, 62720, 35328,
   18944, 47616, 31232, 34304, 17920, 46592, 30208, 35072, 18688, 47360, 30976,
   34048, 17664, 46336, 29952, 10752, 59904,  6656, 55808,  9728, 58880,  5632,
   54784, 10496, 59648,  6400, 55552,  9472, 58624,  5376, 54528, 43520, 27136,
   39424, 23040, 42496, 26112, 38400, 22016, 43264, 26880, 39168, 22784, 42240,
   25856, 38144, 21760
};

// ------------------------------------------------------------------------

sal_uLong nVCLLut[ 256 ] =
{
         0,  1286,  2572,  3858,  5144,  6430,  7716,  9002,
     10288, 11574, 12860, 14146, 15432, 16718, 18004, 19290,
     20576, 21862, 23148, 24434, 25720, 27006, 28292, 29578,
     30864, 32150, 33436, 34722, 36008, 37294, 38580, 39866,
     41152, 42438, 43724, 45010, 46296, 47582, 48868, 50154,
     51440, 52726, 54012, 55298, 56584, 57870, 59156, 60442,
     61728, 63014, 64300, 65586, 66872, 68158, 69444, 70730,
     72016, 73302, 74588, 75874, 77160, 78446, 79732, 81018,
     82304, 83590, 84876, 86162, 87448, 88734, 90020, 91306,
     92592, 93878, 95164, 96450, 97736, 99022,100308,101594,
    102880,104166,105452,106738,108024,109310,110596,111882,
    113168,114454,115740,117026,118312,119598,120884,122170,
    123456,124742,126028,127314,128600,129886,131172,132458,
    133744,135030,136316,137602,138888,140174,141460,142746,
    144032,145318,146604,147890,149176,150462,151748,153034,
    154320,155606,156892,158178,159464,160750,162036,163322,
    164608,165894,167180,168466,169752,171038,172324,173610,
    174896,176182,177468,178754,180040,181326,182612,183898,
    185184,186470,187756,189042,190328,191614,192900,194186,
    195472,196758,198044,199330,200616,201902,203188,204474,
    205760,207046,208332,209618,210904,212190,213476,214762,
    216048,217334,218620,219906,221192,222478,223764,225050,
    226336,227622,228908,230194,231480,232766,234052,235338,
    236624,237910,239196,240482,241768,243054,244340,245626,
    246912,248198,249484,250770,252056,253342,254628,255914,
    257200,258486,259772,261058,262344,263630,264916,266202,
    267488,268774,270060,271346,272632,273918,275204,276490,
    277776,279062,280348,281634,282920,284206,285492,286778,
    288064,289350,290636,291922,293208,294494,295780,297066,
    298352,299638,300924,302210,303496,304782,306068,307354,
    308640,309926,311212,312498,313784,315070,316356,317642,
    318928,320214,321500,322786,324072,325358,326644,327930
};

// ------------------------------------------------------------------------

long FloydMap[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
};

// ------------------------------------------------------------------------

long FloydError1[61] =
{
    -7680, -7424, -7168, -6912, -6656, -6400, -6144,
    -5888, -5632, -5376, -5120, -4864, -4608, -4352,
    -4096, -3840, -3584, -3328, -3072, -2816, -2560,
    -2304, -2048, -1792, -1536, -1280, -1024, -768,
    -512, -256, 0, 256, 512, 768, 1024, 1280, 1536,
    1792, 2048, 2304, 2560, 2816, 3072, 3328, 3584,
    3840, 4096, 4352, 4608, 4864, 5120, 5376, 5632,
    5888, 6144, 6400, 6656, 6912, 7168, 7424, 7680
};

// ------------------------------------------------------------------------

long FloydError3[61] =
{
    -23040, -22272, -21504, -20736, -19968, -19200,
    -18432, -17664, -16896, -16128, -15360, -14592,
    -13824, -13056, -12288, -11520, -10752, -9984,
    -9216, -8448, -7680, -6912, -6144, -5376, -4608,
    -3840, -3072, -2304, -1536, -768, 0, 768, 1536,
    2304, 3072, 3840, 4608, 5376, 6144, 6912, 7680,
    8448, 9216, 9984, 10752, 11520, 12288, 13056,
    13824, 14592, 15360, 16128, 16896, 17664, 18432,
    19200, 19968, 20736, 21504, 22272, 23040
};

// ------------------------------------------------------------------------

long FloydError5[61] =
{
    -38400, -37120, -35840, -34560, -33280, -32000,
    -30720, -29440, -28160, -26880, -25600, -24320,
    -23040, -21760, -20480, -19200, -17920, -16640,
    -15360, -14080, -12800, -11520, -10240, -8960,
    -7680, -6400, -5120, -3840, -2560, -1280,   0,
    1280, 2560, 3840, 5120, 6400, 7680, 8960, 10240,
    11520, 12800, 14080, 15360, 16640, 17920, 19200,
    20480, 21760, 23040, 24320, 25600, 26880, 28160,
    29440, 30720, 32000, 33280, 34560, 35840, 37120,
    38400
};

// ------------------------------------------------------------------------

long FloydError7[61] =
{
    -53760, -51968, -50176, -48384, -46592, -44800,
    -43008, -41216, -39424, -37632, -35840, -34048,
    -32256, -30464, -28672, -26880, -25088, -23296,
    -21504, -19712, -17920, -16128, -14336, -12544,
    -10752, -8960, -7168, -5376, -3584, -1792,  0,
    1792, 3584, 5376, 7168, 8960, 10752, 12544, 14336,
    16128, 17920, 19712, 21504, 23296, 25088, 26880,
    28672, 30464, 32256, 34048, 35840, 37632, 39424,
    41216, 43008, 44800, 46592, 48384, 50176, 51968,
    53760
};

// ------------------------------------------------------------------------

long FloydIndexMap[6] =
{
	-30, 21, 72, 123, 174, 225
};

// --------------------------
// - ImplCreateDitherMatrix -
// --------------------------

void ImplCreateDitherMatrix( sal_uInt8 (*pDitherMatrix)[16][16] )
{
	double			fVal = 3.125;
	const double	fVal16 = fVal / 16.;
	long			i, j, k, l;
	sal_uInt16			pMtx[ 16 ][ 16 ];
	sal_uInt16			nMax = 0;
	static sal_uInt8 	pMagic[4][4] = { { 0, 14,  3, 13, },
                                     {11,  5,  8,  6, },
                                     {12,  2, 15,  1, },
                                     {7,   9,  4, 10 } };

	// MagicSquare aufbauen
	for ( i = 0; i < 4; i++ )
		for ( j = 0; j < 4; j++ )
			for ( k = 0; k < 4; k++ )
				for ( l = 0; l < 4; l++ )
					nMax = Max ( pMtx[ (k<<2) + i][(l<<2 ) + j] =
					(sal_uInt16) ( 0.5 + pMagic[i][j]*fVal + pMagic[k][l]*fVal16 ), nMax );

	// auf Intervall [0;254] skalieren
	for ( i = 0, fVal = 254. / nMax; i < 16; i++ )
		for( j = 0; j < 16; j++ )
			(*pDitherMatrix)[i][j] = (sal_uInt8) ( fVal * pMtx[i][j] );
}

// ----------
// - Bitmap -
// ----------

sal_Bool Bitmap::Convert( BmpConversion eConversion )
{
	const sal_uInt16	nBitCount = GetBitCount();
	sal_Bool			bRet = sal_False;

	switch( eConversion )
	{
		case( BMP_CONVERSION_1BIT_THRESHOLD ):
			bRet = ImplMakeMono( 128 );
		break;

		case( BMP_CONVERSION_1BIT_MATRIX ):
			bRet = ImplMakeMonoDither();
		break;

		case( BMP_CONVERSION_4BIT_GREYS ):
			bRet = ImplMakeGreyscales( 16 );
		break;

		case( BMP_CONVERSION_4BIT_COLORS ):
		{
			if( nBitCount < 4 )
				bRet = ImplConvertUp( 4, NULL );
			else if( nBitCount > 4 )
				bRet = ImplConvertDown( 4, NULL );
			else
				bRet = sal_True;
		}
		break;

		case( BMP_CONVERSION_4BIT_TRANS ):
		{
			Color aTrans( BMP_COL_TRANS );

			if( nBitCount < 4 )
				bRet = ImplConvertUp( 4, &aTrans );
			else
				bRet = ImplConvertDown( 4, &aTrans );
		}
		break;

		case( BMP_CONVERSION_8BIT_GREYS ):
			bRet = ImplMakeGreyscales( 256 );
		break;

		case( BMP_CONVERSION_8BIT_COLORS ):
		{
			if( nBitCount < 8 )
				bRet = ImplConvertUp( 8 );
			else if( nBitCount > 8 )
				bRet = ImplConvertDown( 8 );
			else
				bRet = sal_True;
		}
		break;

		case( BMP_CONVERSION_8BIT_TRANS ):
		{
			Color aTrans( BMP_COL_TRANS );

			if( nBitCount < 8 )
				bRet = ImplConvertUp( 8, &aTrans );
			else
				bRet = ImplConvertDown( 8, &aTrans );
		}
		break;

		case( BMP_CONVERSION_24BIT ):
		{
			if( nBitCount < 24 )
				bRet = ImplConvertUp( 24, NULL );
			else
				bRet = sal_True;
		}
		break;

		case( BMP_CONVERSION_GHOSTED ):
			bRet = ImplConvertGhosted();
		break;

		default:
			DBG_ERROR( "Bitmap::Convert(): Unsupported conversion" );
		break;
	}

	return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::ImplMakeMono( sal_uInt8 cThreshold )
{
	BitmapReadAccess*	pReadAcc = AcquireReadAccess();
	sal_Bool				bRet = sal_False;

	if( pReadAcc )
	{
		Bitmap				aNewBmp( GetSizePixel(), 1 );
		BitmapWriteAccess*	pWriteAcc = aNewBmp.AcquireWriteAccess();

		if( pWriteAcc )
		{
			const BitmapColor	aBlack( pWriteAcc->GetBestMatchingColor( Color( COL_BLACK ) ) );
			const BitmapColor	aWhite( pWriteAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );
			const long			nWidth = pWriteAcc->Width();
			const long			nHeight = pWriteAcc->Height();

			if( pReadAcc->HasPalette() )
			{
				for( long nY = 0L; nY < nHeight; nY++ )
				{
					for( long nX = 0L; nX < nWidth; nX++ )
					{
						const sal_uInt8 cIndex = pReadAcc->GetPixelIndex( nY, nX );
						if( pReadAcc->GetPaletteColor( cIndex ).GetLuminance() >=
							cThreshold )
						{
							pWriteAcc->SetPixel( nY, nX, aWhite );
						}
						else
							pWriteAcc->SetPixel( nY, nX, aBlack );
					}
				}
			}
			else
			{
				for( long nY = 0L; nY < nHeight; nY++ )
				{
					for( long nX = 0L; nX < nWidth; nX++ )
					{
						if( pReadAcc->GetPixel( nY, nX ).GetLuminance() >=
							cThreshold )
						{
							pWriteAcc->SetPixel( nY, nX, aWhite );
						}
						else
							pWriteAcc->SetPixel( nY, nX, aBlack );
					}
				}
			}

			aNewBmp.ReleaseAccess( pWriteAcc );
			bRet = sal_True;
		}

		ReleaseAccess( pReadAcc );

		if( bRet )
		{
			const MapMode	aMap( maPrefMapMode );
			const Size		aSize( maPrefSize );

			*this = aNewBmp;

			maPrefMapMode = aMap;
			maPrefSize = aSize;
		}
	}

	return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::ImplMakeMonoDither()
{
	BitmapReadAccess*	pReadAcc = AcquireReadAccess();
	sal_Bool				bRet = sal_False;

	if( pReadAcc )
	{
		Bitmap				aNewBmp( GetSizePixel(), 1 );
		BitmapWriteAccess*	pWriteAcc = aNewBmp.AcquireWriteAccess();

		if( pWriteAcc )
		{
			const BitmapColor	aBlack( pWriteAcc->GetBestMatchingColor( Color( COL_BLACK ) ) );
			const BitmapColor	aWhite( pWriteAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );
			const long			nWidth = pWriteAcc->Width();
			const long			nHeight = pWriteAcc->Height();
			sal_uInt8				pDitherMatrix[ 16 ][ 16 ];

			ImplCreateDitherMatrix( &pDitherMatrix );

			if( pReadAcc->HasPalette() )
			{
				for( long nY = 0L; nY < nHeight; nY++ )
				{
					for( long nX = 0L, nModY = nY % 16; nX < nWidth; nX++ )
					{
						const sal_uInt8 cIndex = pReadAcc->GetPixelIndex( nY, nX );
						if( pReadAcc->GetPaletteColor( cIndex ).GetLuminance() >
							pDitherMatrix[ nModY ][ nX % 16 ] )
						{
							pWriteAcc->SetPixel( nY, nX, aWhite );
						}
						else
							pWriteAcc->SetPixel( nY, nX, aBlack );
					}
				}
			}
			else
			{
				for( long nY = 0L; nY < nHeight; nY++ )
				{
					for( long nX = 0L, nModY = nY % 16; nX < nWidth; nX++ )
					{
						if( pReadAcc->GetPixel( nY, nX ).GetLuminance() >
							pDitherMatrix[ nModY ][ nX % 16 ] )
						{
							pWriteAcc->SetPixel( nY, nX, aWhite );
						}
						else
							pWriteAcc->SetPixel( nY, nX, aBlack );
					}
				}
			}

			aNewBmp.ReleaseAccess( pWriteAcc );
			bRet = sal_True;
		}

		ReleaseAccess( pReadAcc );

		if( bRet )
		{
			const MapMode	aMap( maPrefMapMode );
			const Size		aSize( maPrefSize );

			*this = aNewBmp;

			maPrefMapMode = aMap;
			maPrefSize = aSize;
		}
	}

	return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::ImplMakeGreyscales( sal_uInt16 nGreys )
{
	DBG_ASSERT( nGreys == 16 || nGreys == 256, "Only 16 or 256 grayscales are supported!" );

	BitmapReadAccess*	pReadAcc = AcquireReadAccess();
	sal_Bool				bRet = sal_False;

	if( pReadAcc )
	{
		const BitmapPalette&	rPal = GetGreyPalette( nGreys );
		sal_uLong 					nShift = ( ( nGreys == 16 ) ? 4UL : 0UL );
		sal_Bool					bPalDiffers = !pReadAcc->HasPalette() || ( rPal.GetEntryCount() != pReadAcc->GetPaletteEntryCount() );

		if( !bPalDiffers )
			bPalDiffers = ( (BitmapPalette&) rPal != pReadAcc->GetPalette() );

		if( bPalDiffers )
		{
			Bitmap				aNewBmp( GetSizePixel(), ( nGreys == 16 ) ? 4 : 8, &rPal );
			BitmapWriteAccess*	pWriteAcc = aNewBmp.AcquireWriteAccess();

			if( pWriteAcc )
			{
				const long	nWidth = pWriteAcc->Width();
				const long	nHeight = pWriteAcc->Height();

				if( pReadAcc->HasPalette() )
				{
					for( long nY = 0L; nY < nHeight; nY++ )
					{
						for( long nX = 0L; nX < nWidth; nX++ )
						{
							const sal_uInt8 cIndex = pReadAcc->GetPixelIndex( nY, nX );
							pWriteAcc->SetPixelIndex( nY, nX,
								(pReadAcc->GetPaletteColor( cIndex ).GetLuminance() >> nShift) );
						}
					}
				}
				else if( pReadAcc->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_BGR &&
						 pWriteAcc->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
				{
					nShift += 8;

					for( long nY = 0L; nY < nHeight; nY++ )
					{
						Scanline pReadScan = pReadAcc->GetScanline( nY );
						Scanline pWriteScan = pWriteAcc->GetScanline( nY );

						for( long nX = 0L; nX < nWidth; nX++ )
						{
							const sal_uLong nB = *pReadScan++;
							const sal_uLong nG = *pReadScan++;
							const sal_uLong nR = *pReadScan++;

							*pWriteScan++ = (sal_uInt8) ( ( nB * 28UL + nG * 151UL + nR * 77UL ) >> nShift );
						}
					}
				}
				else if( pReadAcc->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_RGB &&
						 pWriteAcc->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
				{
					nShift += 8;

					for( long nY = 0L; nY < nHeight; nY++ )
					{
						Scanline pReadScan = pReadAcc->GetScanline( nY );
						Scanline pWriteScan = pWriteAcc->GetScanline( nY );

						for( long nX = 0L; nX < nWidth; nX++ )
						{
							const sal_uLong nR = *pReadScan++;
							const sal_uLong nG = *pReadScan++;
							const sal_uLong nB = *pReadScan++;

							*pWriteScan++ = (sal_uInt8) ( ( nB * 28UL + nG * 151UL + nR * 77UL ) >> nShift );
						}
					}
				}
				else
				{
					for( long nY = 0L; nY < nHeight; nY++ )
						for( long nX = 0L; nX < nWidth; nX++ )
							pWriteAcc->SetPixelIndex( nY, nX, (pReadAcc->GetPixel( nY, nX ) ).GetLuminance() >> nShift );
				}

				aNewBmp.ReleaseAccess( pWriteAcc );
				bRet = sal_True;
			}

			ReleaseAccess( pReadAcc );

			if( bRet )
			{
				const MapMode	aMap( maPrefMapMode );
				const Size		aSize( maPrefSize );

				*this = aNewBmp;

				maPrefMapMode = aMap;
				maPrefSize = aSize;
			}
		}
		else
		{
			ReleaseAccess( pReadAcc );
			bRet = sal_True;
		}
	}

	return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::ImplConvertUp( sal_uInt16 nBitCount, Color* pExtColor )
{
	DBG_ASSERT( nBitCount > GetBitCount(), "New BitCount must be greater!" );

	BitmapReadAccess*	pReadAcc = AcquireReadAccess();
	sal_Bool				bRet = sal_False;

	if( pReadAcc )
	{
		BitmapPalette		aPal;
		Bitmap				aNewBmp( GetSizePixel(), nBitCount, pReadAcc->HasPalette() ? &pReadAcc->GetPalette() : &aPal );
		BitmapWriteAccess*	pWriteAcc = aNewBmp.AcquireWriteAccess();

		if( pWriteAcc )
		{
			const long	nWidth = pWriteAcc->Width();
			const long	nHeight = pWriteAcc->Height();

			if( pWriteAcc->HasPalette() )
			{
				const sal_uInt16			nOldCount = 1 << GetBitCount();
				const BitmapPalette&	rOldPal = pReadAcc->GetPalette();

				aPal.SetEntryCount( 1 << nBitCount );

				for( sal_uInt16 i = 0; i < nOldCount; i++ )
					aPal[ i ] = rOldPal[ i ];

				if( pExtColor )
					aPal[ aPal.GetEntryCount() - 1 ] = *pExtColor;

				pWriteAcc->SetPalette( aPal );

				for( long nY = 0L; nY < nHeight; nY++ )
					for( long nX = 0L; nX < nWidth; nX++ )
						pWriteAcc->SetPixel( nY, nX, pReadAcc->GetPixel( nY, nX ) );
			}
			else
			{
				if( pReadAcc->HasPalette() )
				{
					for( long nY = 0L; nY < nHeight; nY++ )
						for( long nX = 0L; nX < nWidth; nX++ )
							pWriteAcc->SetPixel( nY, nX, pReadAcc->GetPaletteColor( pReadAcc->GetPixelIndex( nY, nX ) ) );
				}
				else
				{
					for( long nY = 0L; nY < nHeight; nY++ )
						for( long nX = 0L; nX < nWidth; nX++ )
							pWriteAcc->SetPixel( nY, nX, pReadAcc->GetPixel( nY, nX ) );
				}
			}

			aNewBmp.ReleaseAccess( pWriteAcc );
			bRet = sal_True;
		}

		ReleaseAccess( pReadAcc );

		if( bRet )
		{
			const MapMode	aMap( maPrefMapMode );
			const Size		aSize( maPrefSize );

			*this = aNewBmp;

			maPrefMapMode = aMap;
			maPrefSize = aSize;
		}
	}

	return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::ImplConvertDown( sal_uInt16 nBitCount, Color* pExtColor )
{
	DBG_ASSERT( nBitCount <= GetBitCount(), "New BitCount must be lower (or equal when pExtColor is set)!" );

	BitmapReadAccess*	pReadAcc = AcquireReadAccess();
	sal_Bool				bRet = sal_False;

	if( pReadAcc )
	{
		BitmapPalette		aPal;
		Bitmap				aNewBmp( GetSizePixel(), nBitCount, &aPal );
		BitmapWriteAccess*	pWriteAcc = aNewBmp.AcquireWriteAccess();

		if( pWriteAcc )
		{
			const sal_uInt16	nCount = 1 << nBitCount;
			const long		nWidth = pWriteAcc->Width();
			const long		nWidth1 = nWidth - 1L;
			const long		nHeight = pWriteAcc->Height();
			Octree			aOctree( *pReadAcc, pExtColor ? ( nCount - 1 ) : nCount );
			InverseColorMap aColorMap( aPal = aOctree.GetPalette() );
			BitmapColor 	aColor;
			ImpErrorQuad	aErrQuad;
			ImpErrorQuad*	pErrQuad1 = new ImpErrorQuad[ nWidth ];
			ImpErrorQuad*	pErrQuad2 = new ImpErrorQuad[ nWidth ];
			ImpErrorQuad*	pQLine1 = pErrQuad1;
			ImpErrorQuad*	pQLine2 = 0;
			long			nX, nY;
			long			nYTmp = 0L;
			sal_uInt8			cIndex;
			sal_Bool			bQ1 = sal_True;

			if( pExtColor )
			{
				aPal.SetEntryCount( aPal.GetEntryCount() + 1 );
				aPal[ aPal.GetEntryCount() - 1 ] = *pExtColor;
			}

			// set Black/White always, if we have enough space
			if( aPal.GetEntryCount() < ( nCount - 1 ) )
			{
				aPal.SetEntryCount( aPal.GetEntryCount() + 2 );
				aPal[ aPal.GetEntryCount() - 2 ] = Color( COL_BLACK );
				aPal[ aPal.GetEntryCount() - 1 ] = Color( COL_WHITE );
			}

			pWriteAcc->SetPalette( aPal );

			for( nY = 0L; nY < Min( nHeight, 2L ); nY++, nYTmp++ )
			{
				for( nX = 0L, pQLine2 = !nY ? pErrQuad1 : pErrQuad2; nX < nWidth; nX++ )
				{
					if( pReadAcc->HasPalette() )
						pQLine2[ nX ] = pReadAcc->GetPaletteColor( pReadAcc->GetPixelIndex( nYTmp, nX ) );
					else
						pQLine2[ nX ] = pReadAcc->GetPixel( nYTmp, nX );
				}
			}

			for( nY = 0L; nY < nHeight; nY++, nYTmp++ )
			{
				// first pixel in the line
				cIndex = (sal_uInt8) aColorMap.GetBestPaletteIndex( pQLine1[ 0 ].ImplGetColor() );
				pWriteAcc->SetPixelIndex( nY, 0, cIndex );

				for( nX = 1L; nX < nWidth1; nX++ )
				{
					cIndex = (sal_uInt8) aColorMap.GetBestPaletteIndex( aColor = pQLine1[ nX ].ImplGetColor() );
					aErrQuad = ( ImpErrorQuad( aColor ) -= pWriteAcc->GetPaletteColor( cIndex ) );
					pQLine1[ ++nX ].ImplAddColorError7( aErrQuad );
					pQLine2[ nX-- ].ImplAddColorError1( aErrQuad );
					pQLine2[ nX-- ].ImplAddColorError5( aErrQuad );
					pQLine2[ nX++ ].ImplAddColorError3( aErrQuad );
					pWriteAcc->SetPixelIndex( nY, nX, cIndex );
				}

				// letztes ZeilenPixel
				if( nX < nWidth )
				{
					cIndex = (sal_uInt8) aColorMap.GetBestPaletteIndex( pQLine1[ nWidth1 ].ImplGetColor() );
					pWriteAcc->SetPixelIndex( nY, nX, cIndex );
				}

				// Zeilenpuffer neu fuellen/kopieren
				pQLine1 = pQLine2;
				pQLine2 = ( bQ1 = !bQ1 ) != sal_False ? pErrQuad2 : pErrQuad1;

				if( nYTmp < nHeight )
				{
					for( nX = 0L; nX < nWidth; nX++ )
					{
						if( pReadAcc->HasPalette() )
							pQLine2[ nX ] = pReadAcc->GetPaletteColor( pReadAcc->GetPixelIndex( nYTmp, nX ) );
						else
							pQLine2[ nX ] = pReadAcc->GetPixel( nYTmp, nX );
					}
				}
			}

			// Zeilenpuffer zerstoeren
			delete[] pErrQuad1;
			delete[] pErrQuad2;

			aNewBmp.ReleaseAccess( pWriteAcc );
			bRet = sal_True;
		}

		ReleaseAccess( pReadAcc );

		if( bRet )
		{
			const MapMode	aMap( maPrefMapMode );
			const Size		aSize( maPrefSize );

			*this = aNewBmp;

			maPrefMapMode = aMap;
			maPrefSize = aSize;
		}
	}

	return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::ImplConvertGhosted()
{
	Bitmap				aNewBmp;
	BitmapReadAccess*	pR = AcquireReadAccess();
	sal_Bool				bRet = sal_False;

	if( pR )
	{
		if( pR->HasPalette() )
		{
			BitmapPalette aNewPal( pR->GetPaletteEntryCount() );

			for( long i = 0, nCount = aNewPal.GetEntryCount(); i < nCount; i++ )
			{
				const BitmapColor& rOld = pR->GetPaletteColor( (sal_uInt16) i );
				aNewPal[ (sal_uInt16) i ] = BitmapColor( ( rOld.GetRed() >> 1 ) | 0x80,
													 ( rOld.GetGreen() >> 1 ) | 0x80,
													 ( rOld.GetBlue() >> 1 ) | 0x80 );
			}

			aNewBmp = Bitmap( GetSizePixel(), GetBitCount(), &aNewPal );
			BitmapWriteAccess* pW = aNewBmp.AcquireWriteAccess();

			if( pW )
			{
				pW->CopyBuffer( *pR );
				aNewBmp.ReleaseAccess( pW );
				bRet = sal_True;
			}
		}
		else
		{
			aNewBmp = Bitmap( GetSizePixel(), 24 );

			BitmapWriteAccess* pW = aNewBmp.AcquireWriteAccess();

			if( pW )
			{
				const long nWidth = pR->Width(), nHeight = pR->Height();

				for( long nY = 0; nY < nHeight; nY++ )
				{
					for( long nX = 0; nX < nWidth; nX++ )
					{
						const BitmapColor aOld( pR->GetPixel( nY, nX ) );
						pW->SetPixel( nY, nX, BitmapColor( ( aOld.GetRed() >> 1 ) | 0x80,
														   ( aOld.GetGreen() >> 1 ) | 0x80,
														   ( aOld.GetBlue() >> 1 ) | 0x80 ) );

					}
				}

				aNewBmp.ReleaseAccess( pW );
				bRet = sal_True;
			}
		}

		ReleaseAccess( pR );
	}

	if( bRet )
	{
		const MapMode	aMap( maPrefMapMode );
		const Size		aSize( maPrefSize );

		*this = aNewBmp;

		maPrefMapMode = aMap;
		maPrefSize = aSize;
	}

	return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::Scale( const double& rScaleX, const double& rScaleY, sal_uInt32 nScaleFlag )
{
	if(basegfx::fTools::equalZero(rScaleX) || basegfx::fTools::equalZero(rScaleY))
	{
		// no scale
		return true;
	}

	if(basegfx::fTools::equal(rScaleX, 1.0) && basegfx::fTools::equal(rScaleY, 1.0))
	{
		// no scale
		return true;
	}

#ifdef DBG_UTIL
	// #121233# allow to test the different scalers in debug build with source
	// level debugger (change nNumber to desired action)
	static sal_uInt16 nNumber(0);
	const sal_uInt16 nStartCount(GetBitCount());

	switch(nNumber)
	{
		case 0 : break;
		case 1: nScaleFlag = BMP_SCALE_FAST; break;
		case 2: nScaleFlag = BMP_SCALE_INTERPOLATE; break;
		case 3: nScaleFlag = BMP_SCALE_SUPER; break;
		case 4: nScaleFlag = BMP_SCALE_LANCZOS; break;
		case 5: nScaleFlag = BMP_SCALE_BICUBIC; break;
		case 6: nScaleFlag = BMP_SCALE_BILINEAR; break;
		case 7: nScaleFlag = BMP_SCALE_BOX; break;
		case 8: nScaleFlag = BMP_SCALE_BESTQUALITY; break;
		case 9: nScaleFlag = BMP_SCALE_FASTESTINTERPOLATE; break;
	}
#endif // DBG_UTIL

	bool bRetval(false);

	if(BMP_SCALE_BESTQUALITY == nScaleFlag)
	{
		// Use LANCZOS when best quality is requested
		nScaleFlag = BMP_SCALE_LANCZOS;
	}
	else if(BMP_SCALE_FASTESTINTERPOLATE == nScaleFlag)
	{
		// Use BMP_SCALE_SUPER when speed is requested, but not worst quality
		nScaleFlag = BMP_SCALE_SUPER;
	}

	switch(nScaleFlag)
	{
		default:
		case BMP_SCALE_NONE :
		{
			bRetval = false;
			break;
		}
		case BMP_SCALE_FAST :
		{
			bRetval = ImplScaleFast( rScaleX, rScaleY );
			break;
		}
		case BMP_SCALE_INTERPOLATE :
		{
			bRetval = ImplScaleInterpolate( rScaleX, rScaleY );
			break;
		}
		case BMP_SCALE_SUPER :
		{
			if(GetSizePixel().Width() < 2 || GetSizePixel().Height() < 2)
			{
				// fallback to ImplScaleFast
				bRetval = ImplScaleFast( rScaleX, rScaleY );
			}
			else
			{
				// #121233# use method from symphony
				bRetval = ImplScaleSuper( rScaleX, rScaleY );
			}
			break;
		}
		case BMP_SCALE_LANCZOS :
		{
			const Lanczos3Kernel kernel;

			bRetval = ImplScaleConvolution( rScaleX, rScaleY, kernel);
			break;
		}
		case BMP_SCALE_BICUBIC :
		{
			const BicubicKernel kernel;

			bRetval = ImplScaleConvolution( rScaleX, rScaleY, kernel );
			break;
		}
		case BMP_SCALE_BILINEAR :
		{
			const BilinearKernel kernel;

			bRetval = ImplScaleConvolution( rScaleX, rScaleY, kernel );
			break;
		}
		case BMP_SCALE_BOX :
		{
			const BoxKernel kernel;

			bRetval = ImplScaleConvolution( rScaleX, rScaleY, kernel );
			break;
		}
	}

#ifdef DBG_UTIL
	if(bRetval && nStartCount != GetBitCount())
	{
		OSL_ENSURE(false, "Bitmap::Scale has changed the ColorDepth, this should *not* happen (!)");
	}
#endif

	return bRetval;
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::Scale( const Size& rNewSize, sal_uInt32 nScaleFlag )
{
	const Size	aSize( GetSizePixel() );
	sal_Bool		bRet;

	if( aSize.Width() && aSize.Height() )
	{
		bRet = Scale( (double) rNewSize.Width() / aSize.Width(),
					  (double) rNewSize.Height() / aSize.Height(),
					  nScaleFlag );
	}
	else
		bRet = sal_True;

	return bRet;
}

// ------------------------------------------------------------------------

void Bitmap::AdaptBitCount(Bitmap& rNew) const
{
	ImplAdaptBitCount(rNew);
}

// ------------------------------------------------------------------------

void Bitmap::ImplAdaptBitCount(Bitmap& rNew) const
{
	// aNew is the result of some operation; adapt its BitCount to the original (this)
	if(GetBitCount() != rNew.GetBitCount())
	{
		switch(GetBitCount())
		{
			case 1:
			{
				rNew.Convert(BMP_CONVERSION_1BIT_THRESHOLD);
				break;
			}
			case 4:
			{
				if(HasGreyPalette())
				{
					rNew.Convert(BMP_CONVERSION_4BIT_GREYS);
				}
				else
				{
					rNew.Convert(BMP_CONVERSION_4BIT_COLORS);
				}
				break;
			}
			case 8:
			{
				if(HasGreyPalette())
				{
					rNew.Convert(BMP_CONVERSION_8BIT_GREYS);
				}
				else
				{
					rNew.Convert(BMP_CONVERSION_8BIT_COLORS);
				}
				break;
			}
			case 24:
			{
				rNew.Convert(BMP_CONVERSION_24BIT);
				break;
			}
			default:
			{
				OSL_ENSURE(false, "BitDepth adaption failed (!)");
				break;
			}
		}
	}
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::ImplScaleFast( const double& rScaleX, const double& rScaleY )
{
	const Size	aSizePix( GetSizePixel() );
	const long	nNewWidth = FRound( aSizePix.Width() * rScaleX );
	const long	nNewHeight = FRound( aSizePix.Height() * rScaleY );
	sal_Bool		bRet = sal_False;

	if( nNewWidth && nNewHeight )
	{
		BitmapReadAccess*	pReadAcc = AcquireReadAccess();
		if ( !pReadAcc )
			return sal_False;

		Bitmap				aNewBmp( Size( nNewWidth, nNewHeight ), GetBitCount(), &pReadAcc->GetPalette() );
		BitmapWriteAccess*	pWriteAcc = aNewBmp.AcquireWriteAccess();

		if( pWriteAcc )
		{
			const long	nScanlineSize = pWriteAcc->GetScanlineSize();
			const long	nNewWidth1 = nNewWidth - 1L;
			const long	nNewHeight1 = nNewHeight - 1L;
			const long	nWidth = pReadAcc->Width();
			const long	nHeight = pReadAcc->Height();
			long*		pLutX = new long[ nNewWidth ];
			long*		pLutY = new long[ nNewHeight ];
			long		nX, nY, nMapY, nActY = 0L;

			if( nNewWidth1 && nNewHeight1 )
			{
				for( nX = 0L; nX < nNewWidth; nX++ )
					pLutX[ nX ] = nX * nWidth / nNewWidth;

				for( nY = 0L; nY < nNewHeight; nY++ )
					pLutY[ nY ] = nY * nHeight / nNewHeight;

				while( nActY < nNewHeight )
				{
					nMapY = pLutY[ nActY ];

					for( nX = 0L; nX < nNewWidth; nX++ )
						pWriteAcc->SetPixel( nActY, nX, pReadAcc->GetPixel( nMapY , pLutX[ nX ] ) );

					while( ( nActY < nNewHeight1 ) && ( pLutY[ nActY + 1 ] == nMapY ) )
					{
						memcpy( pWriteAcc->GetScanline( nActY + 1L ),
								 pWriteAcc->GetScanline( nActY ), nScanlineSize );
						nActY++;
					}

					nActY++;
				}

				bRet = sal_True;
			}

			delete[] pLutX;
			delete[] pLutY;
		}

		ReleaseAccess( pReadAcc );
		aNewBmp.ReleaseAccess( pWriteAcc );

		if( bRet )
			ImplAssignWithSize( aNewBmp );
	}

	return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::ImplScaleInterpolate( const double& rScaleX, const double& rScaleY )
{
	const Size	aSizePix( GetSizePixel() );
	const long	nNewWidth = FRound( aSizePix.Width() * rScaleX );
	const long	nNewHeight = FRound( aSizePix.Height() * rScaleY );
	sal_Bool		bRet = sal_False;

	if( ( nNewWidth > 1L ) && ( nNewHeight > 1L ) )
	{
		BitmapColor 		aCol0;
		BitmapColor 		aCol1;
		BitmapReadAccess*	pReadAcc = AcquireReadAccess();
		long				nWidth = pReadAcc->Width();
		long				nHeight = pReadAcc->Height();
		Bitmap				aNewBmp( Size( nNewWidth, nHeight ), 24 );
		BitmapWriteAccess*	pWriteAcc = aNewBmp.AcquireWriteAccess();
		long*				pLutInt;
		long*				pLutFrac;
		long				nX, nY;
		long				lXB0, lXB1, lXG0, lXG1, lXR0, lXR1;
		double				fTemp;
		long				nTemp;

		if( pReadAcc && pWriteAcc )
		{
			const long		nNewWidth1 = nNewWidth - 1L;
			const long		nWidth1 = pReadAcc->Width() - 1L;
			const double	fRevScaleX = (double) nWidth1 / nNewWidth1;

			pLutInt = new long[ nNewWidth ];
			pLutFrac = new long[ nNewWidth ];

			for( nX = 0L, nTemp = nWidth - 2L; nX < nNewWidth; nX++ )
			{
				fTemp = nX * fRevScaleX;
				pLutInt[ nX ] = MinMax( (long) fTemp, 0, nTemp );
				fTemp -= pLutInt[ nX ];
				pLutFrac[ nX ] = (long) ( fTemp * 1024. );
			}

			for( nY = 0L; nY < nHeight; nY++ )
			{
				if( 1 == nWidth )
				{
					if( pReadAcc->HasPalette() )
					{
						aCol0 = pReadAcc->GetPaletteColor( pReadAcc->GetPixelIndex( nY, 0 ) );
					}
					else
					{
						aCol0 = pReadAcc->GetPixel( nY, 0 );
					}

					for( nX = 0L; nX < nNewWidth; nX++ )
					{
						pWriteAcc->SetPixel( nY, nX, aCol0 );
					}
				}
				else
				{
					for( nX = 0L; nX < nNewWidth; nX++ )
					{
						nTemp = pLutInt[ nX ];

						if( pReadAcc->HasPalette() )
						{
							aCol0 = pReadAcc->GetPaletteColor( pReadAcc->GetPixelIndex( nY, nTemp++ ) );
							aCol1 = pReadAcc->GetPaletteColor( pReadAcc->GetPixelIndex( nY, nTemp ) );
						}
						else
						{
							aCol0 = pReadAcc->GetPixel( nY, nTemp++ );
							aCol1 = pReadAcc->GetPixel( nY, nTemp );
						}

						nTemp = pLutFrac[ nX ];

						lXR1 = aCol1.GetRed() - ( lXR0 = aCol0.GetRed() );
						lXG1 = aCol1.GetGreen() - ( lXG0 = aCol0.GetGreen() );
						lXB1 = aCol1.GetBlue() - ( lXB0 = aCol0.GetBlue() );

						aCol0.SetRed( (sal_uInt8) ( ( lXR1 * nTemp + ( lXR0 << 10 ) ) >> 10 ) );
						aCol0.SetGreen( (sal_uInt8) ( ( lXG1 * nTemp + ( lXG0 << 10 ) ) >> 10 ) );
						aCol0.SetBlue( (sal_uInt8) ( ( lXB1 * nTemp + ( lXB0 << 10 ) ) >> 10 ) );

						pWriteAcc->SetPixel( nY, nX, aCol0 );
					}
				}
			}

			delete[] pLutInt;
			delete[] pLutFrac;
			bRet = sal_True;
		}

		ReleaseAccess( pReadAcc );
		aNewBmp.ReleaseAccess( pWriteAcc );

		if( bRet )
		{
			bRet = sal_False;
			const Bitmap aOriginal(*this);
			*this = aNewBmp;
			aNewBmp = Bitmap( Size( nNewWidth, nNewHeight ), 24 );
			pReadAcc = AcquireReadAccess();
			pWriteAcc = aNewBmp.AcquireWriteAccess();

			if( pReadAcc && pWriteAcc )
			{
				const long		nNewHeight1 = nNewHeight - 1L;
				const long		nHeight1 = pReadAcc->Height() - 1L;
				const double	fRevScaleY = (double) nHeight1 / nNewHeight1;

				pLutInt = new long[ nNewHeight ];
				pLutFrac = new long[ nNewHeight ];

				for( nY = 0L, nTemp = nHeight - 2L; nY < nNewHeight; nY++ )
				{
					fTemp = nY * fRevScaleY;
					pLutInt[ nY ] = MinMax( (long) fTemp, 0, nTemp );
					fTemp -= pLutInt[ nY ];
					pLutFrac[ nY ] = (long) ( fTemp * 1024. );
				}

				// after 1st step, bitmap *is* 24bit format (see above)
				OSL_ENSURE(!pReadAcc->HasPalette(), "OOps, somehow ImplScaleInterpolate in-between format has palette, should not happen (!)");

				for( nX = 0L; nX < nNewWidth; nX++ )
				{
					if( 1 == nHeight )
					{
						aCol0 = pReadAcc->GetPixel( 0, nX );

						for( nY = 0L; nY < nNewHeight; nY++ )
						{
							pWriteAcc->SetPixel( nY, nX, aCol0 );
						}
					}
					else
					{
						for( nY = 0L; nY < nNewHeight; nY++ )
						{
							nTemp = pLutInt[ nY ];

							aCol0 = pReadAcc->GetPixel( nTemp++, nX );
							aCol1 = pReadAcc->GetPixel( nTemp, nX );

							nTemp = pLutFrac[ nY ];

							lXR1 = aCol1.GetRed() - ( lXR0 = aCol0.GetRed() );
							lXG1 = aCol1.GetGreen() - ( lXG0 = aCol0.GetGreen() );
							lXB1 = aCol1.GetBlue() - ( lXB0 = aCol0.GetBlue() );

							aCol0.SetRed( (sal_uInt8) ( ( lXR1 * nTemp + ( lXR0 << 10 ) ) >> 10 ) );
							aCol0.SetGreen( (sal_uInt8) ( ( lXG1 * nTemp + ( lXG0 << 10 ) ) >> 10 ) );
							aCol0.SetBlue( (sal_uInt8) ( ( lXB1 * nTemp + ( lXB0 << 10 ) ) >> 10 ) );

							pWriteAcc->SetPixel( nY, nX, aCol0 );
						}
					}
				}

				delete[] pLutInt;
				delete[] pLutFrac;
				bRet = sal_True;
			}

			ReleaseAccess( pReadAcc );
			aNewBmp.ReleaseAccess( pWriteAcc );

			if( bRet )
			{
				aOriginal.ImplAdaptBitCount(aNewBmp);
				*this = aNewBmp;
			}
		}
	}

	if( !bRet )
	{
		bRet = ImplScaleFast( rScaleX, rScaleY );
	}

	return bRet;
}

// ------------------------------------------------------------------------
// #121233# Added BMP_SCALE_SUPER from symphony code

sal_Bool Bitmap::ImplScaleSuper(
	const double& rScaleX,
	const double& rScaleY )
{
	const Size	aSizePix( GetSizePixel() );
	bool   bHMirr = ( rScaleX < 0 );
	bool   bVMirr = ( rScaleY < 0 );
	double scaleX = bHMirr ? -rScaleX : rScaleX;
	double scaleY = bVMirr ? -rScaleY : rScaleY;
	const long	nDstW = FRound( aSizePix.Width() * scaleX );
	const long	nDstH = FRound( aSizePix.Height() * scaleY );
	const double fScaleThresh = 0.6;
	bool bRet = false;

	if( ( nDstW > 1L ) && ( nDstH > 1L ) )
	{
		BitmapColor			aCol0, aCol1, aColRes;
		BitmapReadAccess*	pAcc = AcquireReadAccess();
		long				nW = pAcc->Width() ;
		long				nH = pAcc->Height() ;
		Bitmap				aOutBmp( Size( nDstW, nDstH ), 24 );
		BitmapWriteAccess*	pWAcc = aOutBmp.AcquireWriteAccess();
		long*				pMapIX = new long[ nDstW ];
		long*				pMapIY = new long[ nDstH ];
		long*				pMapFX = new long[ nDstW ];
		long*				pMapFY = new long[ nDstH ];
		long				nX, nY, nXDst, nYDst;;
		double				fTemp;
		long				nTemp , nTempX, nTempY, nTempFX, nTempFY;
		sal_uInt8			cR0, cG0, cB0, cR1, cG1, cB1;
		long				nStartX = 0 , nStartY = 0;
		long				nEndX = nDstW - 1L;
		long				nEndY = nDstH - 1L;
		long				nMax = 1 << 7L;

		if( pAcc && pWAcc )
		{
			const double	fRevScaleX = ( nDstW > 1L ) ? ( (double) ( nW - 1 ) / ( nDstW - 1 ) ) : 0.0;
			const double	fRevScaleY = ( nDstH > 1L ) ? ( (double) ( nH - 1 ) / ( nDstH - 1 ) ) : 0.0;

			// create horizontal mapping table
			for( nX = 0L, nTempX = nW - 1L, nTemp = nW - 2L; nX < nDstW; nX++ )
			{
				fTemp = nX * fRevScaleX;

				if( bHMirr )
					fTemp = nTempX - fTemp;

				pMapFX[ nX ] = (long) ( ( fTemp - ( pMapIX[ nX ] = MinMax( (long) fTemp, 0, nTemp ) ) ) * 128. );
			}

			// create vertical mapping table
			for( nY = 0L, nTempY = nH - 1L, nTemp = nH - 2L; nY < nDstH; nY++ )
			{
				fTemp = nY * fRevScaleY;

				if( bVMirr )
					fTemp = nTempY - fTemp;

				pMapFY[ nY ] = (long) ( ( fTemp - ( pMapIY[ nY ] = MinMax( (long) fTemp, 0, nTemp ) ) ) * 128. );
			}

			if( pAcc->HasPalette() )
			{
				if( pAcc->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
				{
					if( scaleX >= fScaleThresh && scaleY >= fScaleThresh )
					{
						Scanline pLine0, pLine1;

						for( nY = nStartY, nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
						{
							nTempY = pMapIY[ nY ]; nTempFY = pMapFY[ nY ];
							pLine0 = pAcc->GetScanline( nTempY );
							pLine1 = pAcc->GetScanline( ++nTempY );

							for( nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
							{
								nTempX = pMapIX[ nX ]; nTempFX = pMapFX[ nX ];

								const BitmapColor& rCol0 = pAcc->GetPaletteColor( pLine0[ nTempX ] );
								const BitmapColor& rCol2 = pAcc->GetPaletteColor( pLine1[ nTempX ] );
								const BitmapColor& rCol1 = pAcc->GetPaletteColor( pLine0[ ++nTempX ] );
								const BitmapColor& rCol3 = pAcc->GetPaletteColor( pLine1[ nTempX ] );

								cR0 = MAP( rCol0.GetRed(), rCol1.GetRed(), nTempFX );
								cG0 = MAP( rCol0.GetGreen(), rCol1.GetGreen(), nTempFX );
								cB0 = MAP( rCol0.GetBlue(), rCol1.GetBlue(), nTempFX );

								cR1 = MAP( rCol2.GetRed(), rCol3.GetRed(), nTempFX );
								cG1 = MAP( rCol2.GetGreen(), rCol3.GetGreen(), nTempFX );
								cB1 = MAP( rCol2.GetBlue(), rCol3.GetBlue(), nTempFX );

								aColRes.SetRed( MAP( cR0, cR1, nTempFY ) );
								aColRes.SetGreen( MAP( cG0, cG1, nTempFY ) );
								aColRes.SetBlue( MAP( cB0, cB1, nTempFY ) );
								pWAcc->SetPixel( nYDst, nXDst++, aColRes );
							}
						}
					}
					else
					{
						Scanline	pTmpY;
						long		nSumR, nSumG, nSumB,nLineStart , nLineRange, nRowStart , nRowRange ;
						long		nLeft, nRight, nTop, nBottom, nWeightX, nWeightY ;
						long		nSumRowR ,nSumRowG,nSumRowB, nTotalWeightX, nTotalWeightY;

						for( nY = nStartY , nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
						{
							nTop = bVMirr ? ( nY + 1 ) : nY;
							nBottom = bVMirr ? nY : ( nY + 1 ) ;

							if( nY ==nEndY )
							{
								nLineStart = pMapIY[ nY ];
								nLineRange = 0;
							}
							else
							{
								nLineStart = pMapIY[ nTop ] ;
								nLineRange = ( pMapIY[ nBottom ] == pMapIY[ nTop ] ) ? 1 :( pMapIY[ nBottom ] - pMapIY[ nTop ] );
							}

							for( nX = nStartX , nXDst = 0L; nX <= nEndX; nX++ )
							{
								nLeft = bHMirr ? ( nX + 1 ) : nX;
								nRight = bHMirr ? nX : ( nX + 1 ) ;

								if( nX == nEndX )
								{
									nRowStart = pMapIX[ nX ];
									nRowRange = 0;
								}
								else
								{
									nRowStart = pMapIX[ nLeft ];
									nRowRange = ( pMapIX[ nRight ] == pMapIX[ nLeft ] )? 1 : ( pMapIX[ nRight ] - pMapIX[ nLeft ] );
								}

								nSumR = nSumG = nSumB = 0;
								nTotalWeightY = 0;

								for(int i = 0; i<= nLineRange; i++)
								{
									pTmpY = pAcc->GetScanline( nLineStart + i );
									nSumRowR = nSumRowG = nSumRowB = 0;
									nTotalWeightX = 0;

									for(int j = 0; j <= nRowRange; j++)
									{
										const BitmapColor& rCol = pAcc->GetPaletteColor( pTmpY[ nRowStart + j ] );

										if(nX == nEndX )
										{
											nSumRowB += rCol.GetBlue() << 7L;
											nSumRowG += rCol.GetGreen() << 7L;
											nSumRowR += rCol.GetRed() << 7L;
											nTotalWeightX += 1 << 7L;
										}
										else if( j == 0 )
										{
											nWeightX = (nMax- pMapFX[ nLeft ]) ;
											nSumRowB += ( nWeightX *rCol.GetBlue()) ;
											nSumRowG += ( nWeightX *rCol.GetGreen()) ;
											nSumRowR += ( nWeightX *rCol.GetRed()) ;
											nTotalWeightX += nWeightX;
										}
										else if ( nRowRange == j )
										{
											nWeightX = pMapFX[ nRight ] ;
											nSumRowB += ( nWeightX *rCol.GetBlue() );
											nSumRowG += ( nWeightX *rCol.GetGreen() );
											nSumRowR += ( nWeightX *rCol.GetRed() );
											nTotalWeightX += nWeightX;
										}
										else
										{
											nSumRowB += rCol.GetBlue() << 7L;
											nSumRowG += rCol.GetGreen() << 7L;
											nSumRowR += rCol.GetRed() << 7L;
											nTotalWeightX += 1 << 7L;
										}
									}

									if( nY == nEndY )
										nWeightY = nMax;
									else if( i == 0 )
										nWeightY = nMax - pMapFY[ nTop ];
									else if( nLineRange == 1 )
										nWeightY = pMapFY[ nTop ];
									else if ( nLineRange == i )
										nWeightY = pMapFY[ nBottom ];
									else
										nWeightY = nMax;

									nSumB += nWeightY * ( nSumRowB / nTotalWeightX );
									nSumG += nWeightY * ( nSumRowG / nTotalWeightX );
									nSumR += nWeightY * ( nSumRowR / nTotalWeightX );
									nTotalWeightY += nWeightY;
								}

								aColRes.SetRed( ( sal_uInt8 ) (( nSumR / nTotalWeightY ) ));
								aColRes.SetGreen( ( sal_uInt8 ) (( nSumG / nTotalWeightY) ));
								aColRes.SetBlue( ( sal_uInt8 ) (( nSumB / nTotalWeightY ) ));
								pWAcc->SetPixel( nYDst, nXDst++, aColRes );

							}
						}
					}
}
				else
				{
					if( scaleX >= fScaleThresh && scaleY >= fScaleThresh )
					{
						for( nY = nStartY, nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
						{
							nTempY = pMapIY[ nY ], nTempFY = pMapFY[ nY ];

							for( nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
							{
								nTempX = pMapIX[ nX ]; nTempFX = pMapFX[ nX ];

								aCol0 = pAcc->GetPaletteColor( pAcc->GetPixelIndex( nTempY, nTempX ) );
								aCol1 = pAcc->GetPaletteColor( pAcc->GetPixelIndex( nTempY, ++nTempX ) );
								cR0 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTempFX );
								cG0 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTempFX );
								cB0 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTempFX );

								aCol1 = pAcc->GetPaletteColor( pAcc->GetPixelIndex( ++nTempY, nTempX ) );
								aCol0 = pAcc->GetPaletteColor( pAcc->GetPixelIndex( nTempY--, --nTempX ) );
								cR1 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTempFX );
								cG1 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTempFX );
								cB1 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTempFX );

								aColRes.SetRed( MAP( cR0, cR1, nTempFY ) );
								aColRes.SetGreen( MAP( cG0, cG1, nTempFY ) );
								aColRes.SetBlue( MAP( cB0, cB1, nTempFY ) );
								pWAcc->SetPixel( nYDst, nXDst++, aColRes );
							}
						}

					}
					else
					{
						long		nSumR, nSumG, nSumB,nLineStart , nLineRange, nRowStart , nRowRange ;
						long		nLeft, nRight, nTop, nBottom, nWeightX, nWeightY ;
						long		nSumRowR ,nSumRowG,nSumRowB, nTotalWeightX, nTotalWeightY;

						for( nY = nStartY , nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
						{
							nTop = bVMirr ? ( nY + 1 ) : nY;
							nBottom = bVMirr ? nY : ( nY + 1 ) ;

							if( nY ==nEndY )
							{
								nLineStart = pMapIY[ nY ];
								nLineRange = 0;
							}
							else
							{
								nLineStart = pMapIY[ nTop ] ;
								nLineRange = ( pMapIY[ nBottom ] == pMapIY[ nTop ] ) ? 1 :( pMapIY[ nBottom ] - pMapIY[ nTop ] );
							}

							for( nX = nStartX , nXDst = 0L; nX <= nEndX; nX++ )
							{
								nLeft = bHMirr ? ( nX + 1 ) : nX;
								nRight = bHMirr ? nX : ( nX + 1 ) ;

								if( nX == nEndX )
								{
									nRowStart = pMapIX[ nX ];
									nRowRange = 0;
								}
								else
								{
									nRowStart = pMapIX[ nLeft ];
									nRowRange = ( pMapIX[ nRight ] == pMapIX[ nLeft ] )? 1 : ( pMapIX[ nRight ] - pMapIX[ nLeft ] );
								}

								nSumR = nSumG = nSumB = 0;
								nTotalWeightY = 0;

								for(int i = 0; i<= nLineRange; i++)
								{
									nSumRowR = nSumRowG = nSumRowB = 0;
									nTotalWeightX = 0;

									for(int j = 0; j <= nRowRange; j++)
									{
										aCol0 = pAcc->GetPaletteColor ( pAcc->GetPixelIndex( nLineStart + i, nRowStart + j ) );

										if(nX == nEndX )
										{

											nSumRowB += aCol0.GetBlue() << 7L;
											nSumRowG += aCol0.GetGreen() << 7L;
											nSumRowR += aCol0.GetRed() << 7L;
											nTotalWeightX += 1 << 7L;
										}
										else if( j == 0 )
										{

											nWeightX = (nMax- pMapFX[ nLeft ]) ;
											nSumRowB += ( nWeightX *aCol0.GetBlue()) ;
											nSumRowG += ( nWeightX *aCol0.GetGreen()) ;
											nSumRowR += ( nWeightX *aCol0.GetRed()) ;
											nTotalWeightX += nWeightX;
										}
										else if ( nRowRange == j )
										{

											nWeightX = pMapFX[ nRight ] ;
											nSumRowB += ( nWeightX *aCol0.GetBlue() );
											nSumRowG += ( nWeightX *aCol0.GetGreen() );
											nSumRowR += ( nWeightX *aCol0.GetRed() );
											nTotalWeightX += nWeightX;
										}
										else
										{

											nSumRowB += aCol0.GetBlue() << 7L;
											nSumRowG += aCol0.GetGreen() << 7L;
											nSumRowR += aCol0.GetRed() << 7L;
											nTotalWeightX += 1 << 7L;
										}
									}

									if( nY == nEndY )
										nWeightY = nMax;
									else if( i == 0 )
										nWeightY = nMax - pMapFY[ nTop ];
									else if( nLineRange == 1 )
										nWeightY = pMapFY[ nTop ];
									else if ( nLineRange == i )
										nWeightY = pMapFY[ nBottom ];
									else
										nWeightY = nMax;

									nSumB += nWeightY * ( nSumRowB / nTotalWeightX );
									nSumG += nWeightY * ( nSumRowG / nTotalWeightX );
									nSumR += nWeightY * ( nSumRowR / nTotalWeightX );
									nTotalWeightY += nWeightY;
								}

								aColRes.SetRed( ( sal_uInt8 ) (( nSumR / nTotalWeightY ) ));
								aColRes.SetGreen( ( sal_uInt8 ) (( nSumG / nTotalWeightY) ));
								aColRes.SetBlue( ( sal_uInt8 ) (( nSumB / nTotalWeightY ) ));
								pWAcc->SetPixel( nYDst, nXDst++, aColRes );
							}
						}
					}
				}
			}
			else
			{
				if( pAcc->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_BGR )
				{
					if( scaleX >= fScaleThresh && scaleY >= fScaleThresh )
					{
						Scanline	pLine0, pLine1, pTmp0, pTmp1;
						long		nOff;

						for( nY = nStartY, nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
						{
							nTempY = pMapIY[ nY ]; nTempFY = pMapFY[ nY ];
							pLine0 = pAcc->GetScanline( nTempY );
							pLine1 = pAcc->GetScanline( ++nTempY );

							for( nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
							{
								nOff = 3L * ( nTempX = pMapIX[ nX ] );
								nTempFX = pMapFX[ nX ];

								pTmp1 = ( pTmp0 = pLine0 + nOff ) + 3L;
								cB0 = MAP( *pTmp0, *pTmp1, nTempFX ); pTmp0++; pTmp1++;
								cG0 = MAP( *pTmp0, *pTmp1, nTempFX ); pTmp0++; pTmp1++;
								cR0 = MAP( *pTmp0, *pTmp1, nTempFX );

								pTmp1 = ( pTmp0 = pLine1 + nOff ) + 3L;
								cB1 = MAP( *pTmp0, *pTmp1, nTempFX ); pTmp0++; pTmp1++;
								cG1 = MAP( *pTmp0, *pTmp1, nTempFX ); pTmp0++; pTmp1++;
								cR1 = MAP( *pTmp0, *pTmp1, nTempFX );

								aColRes.SetRed( MAP( cR0, cR1, nTempFY ) );
								aColRes.SetGreen( MAP( cG0, cG1, nTempFY ) );
								aColRes.SetBlue( MAP( cB0, cB1, nTempFY ) );
								pWAcc->SetPixel( nYDst, nXDst++, aColRes );
							}
						}
					}
					else
					{
						Scanline	pTmpY, pTmpX;
						long		nSumR, nSumG, nSumB,nLineStart , nLineRange, nRowStart , nRowRange ;
						long		nLeft, nRight, nTop, nBottom, nWeightX, nWeightY ;
						long		nSumRowR ,nSumRowG,nSumRowB, nTotalWeightX, nTotalWeightY;

						for( nY = nStartY , nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
						{
							nTop = bVMirr ? ( nY + 1 ) : nY;
							nBottom = bVMirr ? nY : ( nY + 1 ) ;

							if( nY ==nEndY )
							{
								nLineStart = pMapIY[ nY ];
								nLineRange = 0;
							}
							else
							{
								nLineStart = pMapIY[ nTop ] ;
								nLineRange = ( pMapIY[ nBottom ] == pMapIY[ nTop ] ) ? 1 :( pMapIY[ nBottom ] - pMapIY[ nTop ] );
							}

							for( nX = nStartX , nXDst = 0L; nX <= nEndX; nX++ )
							{
								nLeft = bHMirr ? ( nX + 1 ) : nX;
								nRight = bHMirr ? nX : ( nX + 1 ) ;

								if( nX == nEndX )
								{
									nRowStart = pMapIX[ nX ];
									nRowRange = 0;
								}
								else
								{
									nRowStart = pMapIX[ nLeft ];
									nRowRange = ( pMapIX[ nRight ] == pMapIX[ nLeft ] )? 1 : ( pMapIX[ nRight ] - pMapIX[ nLeft ] );
								}

								nSumR = nSumG = nSumB = 0;
								nTotalWeightY = 0;

								for(int i = 0; i<= nLineRange; i++)
								{
									pTmpY = pAcc->GetScanline( nLineStart + i );
									pTmpX = pTmpY + 3L * nRowStart;
									nSumRowR = nSumRowG = nSumRowB = 0;
									nTotalWeightX = 0;

									for(int j = 0; j <= nRowRange; j++)
									{
										if(nX == nEndX )
										{
											nSumRowB += ( *pTmpX ) << 7L;pTmpX++;
											nSumRowG += ( *pTmpX ) << 7L;pTmpX++;
											nSumRowR += ( *pTmpX ) << 7L;pTmpX++;
											nTotalWeightX += 1 << 7L;
										}
										else if( j == 0 )
										{
											nWeightX = (nMax- pMapFX[ nLeft ]) ;
											nSumRowB += ( nWeightX *( *pTmpX )) ;pTmpX++;
											nSumRowG += ( nWeightX *( *pTmpX )) ;pTmpX++;
											nSumRowR += ( nWeightX *( *pTmpX )) ;pTmpX++;
											nTotalWeightX += nWeightX;
										}
										else if ( nRowRange == j )
										{
											nWeightX = pMapFX[ nRight ] ;
											nSumRowB += ( nWeightX *( *pTmpX ) );pTmpX++;
											nSumRowG += ( nWeightX *( *pTmpX ) );pTmpX++;
											nSumRowR += ( nWeightX *( *pTmpX ) );pTmpX++;
											nTotalWeightX += nWeightX;
										}
										else
										{
											nSumRowB += ( *pTmpX ) << 7L;pTmpX++;
											nSumRowG += ( *pTmpX ) << 7L;pTmpX++;
											nSumRowR += ( *pTmpX ) << 7L;pTmpX++;
											nTotalWeightX += 1 << 7L;
										}
									}

									if( nY == nEndY )
										nWeightY = nMax;
									else if( i == 0 )
										nWeightY = nMax - pMapFY[ nTop ];
									else if( nLineRange == 1 )
										nWeightY = pMapFY[ nTop ];
									else if ( nLineRange == i )
										nWeightY = pMapFY[ nBottom ];
									else
										nWeightY = nMax;

									nSumB += nWeightY * ( nSumRowB / nTotalWeightX );
									nSumG += nWeightY * ( nSumRowG / nTotalWeightX );
									nSumR += nWeightY * ( nSumRowR / nTotalWeightX );
									nTotalWeightY += nWeightY;
								}

								aColRes.SetRed( ( sal_uInt8 ) (( nSumR / nTotalWeightY ) ));
								aColRes.SetGreen( ( sal_uInt8 ) (( nSumG / nTotalWeightY) ));
								aColRes.SetBlue( ( sal_uInt8 ) (( nSumB / nTotalWeightY ) ));
								pWAcc->SetPixel( nYDst, nXDst++, aColRes );

							}
						}
					}
				}
				else if( pAcc->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_RGB )
				{
					if( scaleX >= fScaleThresh && scaleY >= fScaleThresh )
					{
						Scanline	pLine0, pLine1, pTmp0, pTmp1;
						long		nOff;

						for( nY = nStartY, nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
						{
							nTempY = pMapIY[ nY ]; nTempFY = pMapFY[ nY ];
							pLine0 = pAcc->GetScanline( nTempY );
							pLine1 = pAcc->GetScanline( ++nTempY );

							for( nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
							{
								nOff = 3L * ( nTempX = pMapIX[ nX ] );
								nTempFX = pMapFX[ nX ];

								pTmp1 = ( pTmp0 = pLine0 + nOff ) + 3L;
								cR0 = MAP( *pTmp0, *pTmp1, nTempFX ); pTmp0++; pTmp1++;
								cG0 = MAP( *pTmp0, *pTmp1, nTempFX ); pTmp0++; pTmp1++;
								cB0 = MAP( *pTmp0, *pTmp1, nTempFX );

								pTmp1 = ( pTmp0 = pLine1 + nOff ) + 3L;
								cR1 = MAP( *pTmp0, *pTmp1, nTempFX ); pTmp0++; pTmp1++;
								cG1 = MAP( *pTmp0, *pTmp1, nTempFX ); pTmp0++; pTmp1++;
								cB1 = MAP( *pTmp0, *pTmp1, nTempFX );

								aColRes.SetRed( MAP( cR0, cR1, nTempFY ) );
								aColRes.SetGreen( MAP( cG0, cG1, nTempFY ) );
								aColRes.SetBlue( MAP( cB0, cB1, nTempFY ) );
								pWAcc->SetPixel( nYDst, nXDst++, aColRes );
							}
						}
					}
					else
					{
						Scanline	pTmpY, pTmpX;
						long		nSumR, nSumG, nSumB,nLineStart , nLineRange, nRowStart , nRowRange ;
						long		nLeft, nRight, nTop, nBottom, nWeightX, nWeightY ;
						long		nSumRowR ,nSumRowG,nSumRowB, nTotalWeightX, nTotalWeightY;

						for( nY = nStartY , nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
						{
							nTop = bVMirr ? ( nY + 1 ) : nY;
							nBottom = bVMirr ? nY : ( nY + 1 ) ;

							if( nY ==nEndY )
							{
								nLineStart = pMapIY[ nY ];
								nLineRange = 0;
							}
							else
							{
								nLineStart = pMapIY[ nTop ] ;
								nLineRange = ( pMapIY[ nBottom ] == pMapIY[ nTop ] ) ? 1 :( pMapIY[ nBottom ] - pMapIY[ nTop ] );
							}

							for( nX = nStartX , nXDst = 0L; nX <= nEndX; nX++ )
							{
								nLeft = bHMirr ? ( nX + 1 ) : nX;
								nRight = bHMirr ? nX : ( nX + 1 ) ;

								if( nX == nEndX )
								{
									nRowStart = pMapIX[ nX ];
									nRowRange = 0;
								}
								else
								{
									nRowStart = pMapIX[ nLeft ];
									nRowRange = ( pMapIX[ nRight ] == pMapIX[ nLeft ] )? 1 : ( pMapIX[ nRight ] - pMapIX[ nLeft ] );
								}

								nSumR = nSumG = nSumB = 0;
								nTotalWeightY = 0;

								for(int i = 0; i<= nLineRange; i++)
								{
									pTmpY = pAcc->GetScanline( nLineStart + i );
									pTmpX = pTmpY + 3L * nRowStart;
									nSumRowR = nSumRowG = nSumRowB = 0;
									nTotalWeightX = 0;

									for(int j = 0; j <= nRowRange; j++)
									{
										if(nX == nEndX )
										{
											nSumRowR += ( *pTmpX ) << 7L;pTmpX++;
											nSumRowG += ( *pTmpX ) << 7L;pTmpX++;
											nSumRowB += ( *pTmpX ) << 7L;pTmpX++;
											nTotalWeightX += 1 << 7L;
										}
										else if( j == 0 )
										{
											nWeightX = (nMax- pMapFX[ nLeft ]) ;
											nSumRowR += ( nWeightX *( *pTmpX )) ;pTmpX++;
											nSumRowG += ( nWeightX *( *pTmpX )) ;pTmpX++;
											nSumRowB += ( nWeightX *( *pTmpX )) ;pTmpX++;
											nTotalWeightX += nWeightX;
										}
										else if ( nRowRange == j )
										{
											nWeightX = pMapFX[ nRight ] ;
											nSumRowR += ( nWeightX *( *pTmpX ) );pTmpX++;
											nSumRowG += ( nWeightX *( *pTmpX ) );pTmpX++;
											nSumRowB += ( nWeightX *( *pTmpX ) );pTmpX++;
											nTotalWeightX += nWeightX;
										}
										else
										{
											nSumRowR += ( *pTmpX ) << 7L;pTmpX++;
											nSumRowG += ( *pTmpX ) << 7L;pTmpX++;
											nSumRowB += ( *pTmpX ) << 7L;pTmpX++;
											nTotalWeightX += 1 << 7L;
										}
									}

									if( nY == nEndY )
										nWeightY = nMax;
									else if( i == 0 )
										nWeightY = nMax - pMapFY[ nTop ];
									else if( nLineRange == 1 )
										nWeightY = pMapFY[ nTop ];
									else if ( nLineRange == i )
										nWeightY = pMapFY[ nBottom ];
									else
										nWeightY = nMax;

									nSumB += nWeightY * ( nSumRowB / nTotalWeightX );
									nSumG += nWeightY * ( nSumRowG / nTotalWeightX );
									nSumR += nWeightY * ( nSumRowR / nTotalWeightX );
									nTotalWeightY += nWeightY;
								}

								aColRes.SetRed( ( sal_uInt8 ) (( nSumR / nTotalWeightY ) ));
								aColRes.SetGreen( ( sal_uInt8 ) (( nSumG / nTotalWeightY) ));
								aColRes.SetBlue( ( sal_uInt8 ) (( nSumB / nTotalWeightY ) ));
								pWAcc->SetPixel( nYDst, nXDst++, aColRes );

							}
						}
					}
				}
				else
				{
					if( scaleX >= fScaleThresh && scaleY >= fScaleThresh )
					{
						for( nY = nStartY, nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
						{
							nTempY = pMapIY[ nY ]; nTempFY = pMapFY[ nY ];

							for( nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
							{
								nTempX = pMapIX[ nX ]; nTempFX = pMapFX[ nX ];

								aCol0 = pAcc->GetPixel( nTempY, nTempX );
								aCol1 = pAcc->GetPixel( nTempY, ++nTempX );
								cR0 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTempFX );
								cG0 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTempFX );
								cB0 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTempFX );

								aCol1 = pAcc->GetPixel( ++nTempY, nTempX );
								aCol0 = pAcc->GetPixel( nTempY--, --nTempX );
								cR1 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTempFX );
								cG1 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTempFX );
								cB1 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTempFX );

								aColRes.SetRed( MAP( cR0, cR1, nTempFY ) );
								aColRes.SetGreen( MAP( cG0, cG1, nTempFY ) );
								aColRes.SetBlue( MAP( cB0, cB1, nTempFY ) );
								pWAcc->SetPixel( nYDst, nXDst++, aColRes );
							}
						}
					}
					else
					{
						long		nSumR, nSumG, nSumB,nLineStart , nLineRange, nRowStart , nRowRange ;
						long		nLeft, nRight, nTop, nBottom, nWeightX, nWeightY ;
						long		nSumRowR ,nSumRowG,nSumRowB, nTotalWeightX, nTotalWeightY;

						for( nY = nStartY , nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
						{
							nTop = bVMirr ? ( nY + 1 ) : nY;
							nBottom = bVMirr ? nY : ( nY + 1 ) ;

							if( nY ==nEndY )
							{
								nLineStart = pMapIY[ nY ];
								nLineRange = 0;
							}
							else
							{
								nLineStart = pMapIY[ nTop ] ;
								nLineRange = ( pMapIY[ nBottom ] == pMapIY[ nTop ] ) ? 1 :( pMapIY[ nBottom ] - pMapIY[ nTop ] );
							}

							for( nX = nStartX , nXDst = 0L; nX <= nEndX; nX++ )
							{
								nLeft = bHMirr ? ( nX + 1 ) : nX;
								nRight = bHMirr ? nX : ( nX + 1 ) ;

								if( nX == nEndX )
								{
									nRowStart = pMapIX[ nX ];
									nRowRange = 0;
								}
								else
								{
									nRowStart = pMapIX[ nLeft ];
									nRowRange = ( pMapIX[ nRight ] == pMapIX[ nLeft ] )? 1 : ( pMapIX[ nRight ] - pMapIX[ nLeft ] );
								}

								nSumR = nSumG = nSumB = 0;
								nTotalWeightY = 0;

								for(int i = 0; i<= nLineRange; i++)
								{
									nSumRowR = nSumRowG = nSumRowB = 0;
									nTotalWeightX = 0;

									for(int j = 0; j <= nRowRange; j++)
									{
										aCol0 = pAcc->GetPixel( nLineStart + i, nRowStart + j );

										if(nX == nEndX )
										{

											nSumRowB += aCol0.GetBlue() << 7L;
											nSumRowG += aCol0.GetGreen() << 7L;
											nSumRowR += aCol0.GetRed() << 7L;
											nTotalWeightX += 1 << 7L;
										}
										else if( j == 0 )
										{

											nWeightX = (nMax- pMapFX[ nLeft ]) ;
											nSumRowB += ( nWeightX *aCol0.GetBlue()) ;
											nSumRowG += ( nWeightX *aCol0.GetGreen()) ;
											nSumRowR += ( nWeightX *aCol0.GetRed()) ;
											nTotalWeightX += nWeightX;
										}
										else if ( nRowRange == j )
										{

											nWeightX = pMapFX[ nRight ] ;
											nSumRowB += ( nWeightX *aCol0.GetBlue() );
											nSumRowG += ( nWeightX *aCol0.GetGreen() );
											nSumRowR += ( nWeightX *aCol0.GetRed() );
											nTotalWeightX += nWeightX;
										}
										else
										{
											nSumRowB += aCol0.GetBlue() << 7L;
											nSumRowG += aCol0.GetGreen() << 7L;
											nSumRowR += aCol0.GetRed() << 7L;
											nTotalWeightX += 1 << 7L;
										}
									}

									if( nY == nEndY )
										nWeightY = nMax;
									else if( i == 0 )
										nWeightY = nMax - pMapFY[ nTop ];
									else if( nLineRange == 1 )
										nWeightY = pMapFY[ nTop ];
									else if ( nLineRange == i )
										nWeightY = pMapFY[ nBottom ];
									else
										nWeightY = nMax;

									nSumB += nWeightY * ( nSumRowB / nTotalWeightX );
									nSumG += nWeightY * ( nSumRowG / nTotalWeightX );
									nSumR += nWeightY * ( nSumRowR / nTotalWeightX );
									nTotalWeightY += nWeightY;
								}

								aColRes.SetRed( ( sal_uInt8 ) (( nSumR / nTotalWeightY ) ));
								aColRes.SetGreen( ( sal_uInt8 ) (( nSumG / nTotalWeightY) ));
								aColRes.SetBlue( ( sal_uInt8 ) (( nSumB / nTotalWeightY ) ));
								pWAcc->SetPixel( nYDst, nXDst++, aColRes );

							}
						}
					}
				}
			}

			bRet = true;
		}

		delete[] pMapIX;
		delete[] pMapIY;
		delete[] pMapFX;
		delete[] pMapFY;

		ReleaseAccess( pAcc );
		aOutBmp.ReleaseAccess( pWAcc );

		if( bRet )
		{
			ImplAdaptBitCount(aOutBmp);
			ImplAssignWithSize(aOutBmp);
		}

		if( !bRet )
			bRet = ImplScaleFast( scaleX, scaleY );
	}

	return bRet;
}

//-----------------------------------------------------------------------------------

namespace
{
	void ImplCalculateContributions(
		const sal_uInt32 aSourceSize,
		const sal_uInt32 aDestinationSize,
		sal_uInt32& aNumberOfContributions,
		double*& pWeights,
		sal_uInt32*& pPixels,
		sal_uInt32*& pCount,
		const Kernel& aKernel)
	{
		const double fSamplingRadius(aKernel.GetWidth());
		const double fScale(aDestinationSize / static_cast< double >(aSourceSize));
		const double fScaledRadius((fScale < 1.0) ? fSamplingRadius / fScale : fSamplingRadius);
		const double fFilterFactor((fScale < 1.0) ? fScale : 1.0);

		aNumberOfContributions = (static_cast< sal_uInt32 >(fabs(ceil(fScaledRadius))) * 2) + 1;
		const sal_uInt32 nAllocSize(aDestinationSize * aNumberOfContributions);
		pWeights = new double[nAllocSize];
		pPixels = new sal_uInt32[nAllocSize];
		pCount = new sal_uInt32[aDestinationSize];

		for(sal_uInt32 i(0); i < aDestinationSize; i++)
		{
			const sal_uInt32 aIndex(i * aNumberOfContributions);
			const double aCenter(i / fScale);
			const sal_Int32 aLeft(static_cast< sal_Int32 >(floor(aCenter - fScaledRadius)));
			const sal_Int32 aRight(static_cast< sal_Int32 >(ceil(aCenter + fScaledRadius)));
			sal_uInt32 aCurrentCount(0);

			for(sal_Int32 j(aLeft); j <= aRight; j++)
			{
				const double aWeight(aKernel.Calculate(fFilterFactor * (aCenter - static_cast< double>(j))));

				// Reduce calculations with ignoring weights of 0.0
				if(fabs(aWeight) < 0.0001)
				{
					continue;
				}

				// Handling on edges
				const sal_uInt32 aPixelIndex(MinMax(j, 0, aSourceSize - 1));
				const sal_uInt32 nIndex(aIndex + aCurrentCount);

				pWeights[nIndex] = aWeight;
				pPixels[nIndex] = aPixelIndex;

				aCurrentCount++;
			}

			pCount[i] = aCurrentCount;
		}
	}

	sal_Bool ImplScaleConvolutionHor(
		Bitmap& rSource,
		Bitmap& rTarget,
		const double& rScaleX,
		const Kernel& aKernel)
	{
		// Do horizontal filtering
		OSL_ENSURE(rScaleX > 0.0, "Error in scaling: Mirror given in non-mirror-capable method (!)");
		const sal_uInt32 nWidth(rSource.GetSizePixel().Width());
		const sal_uInt32 nNewWidth(FRound(nWidth * rScaleX));

		if(nWidth == nNewWidth)
		{
			return true;
		}

		BitmapReadAccess* pReadAcc = rSource.AcquireReadAccess();

		if(pReadAcc)
		{
			double* pWeights = 0;
			sal_uInt32* pPixels = 0;
			sal_uInt32* pCount = 0;
			sal_uInt32 aNumberOfContributions(0);

			const sal_uInt32 nHeight(rSource.GetSizePixel().Height());
			ImplCalculateContributions(nWidth, nNewWidth, aNumberOfContributions, pWeights, pPixels, pCount, aKernel);
			rTarget = Bitmap(Size(nNewWidth, nHeight), 24);
			BitmapWriteAccess* pWriteAcc = rTarget.AcquireWriteAccess();
			bool bResult(0 != pWriteAcc);

			if(bResult)
			{
				for(sal_uInt32 y(0); y < nHeight; y++)
				{
					for(sal_uInt32 x(0); x < nNewWidth; x++)
					{
						const sal_uInt32 aBaseIndex(x * aNumberOfContributions);
						double aSum(0.0);
						double aValueRed(0.0);
						double aValueGreen(0.0);
						double aValueBlue(0.0);

						for(sal_uInt32 j(0); j < pCount[x]; j++)
						{
							const sal_uInt32 aIndex(aBaseIndex + j);
							const double aWeight(pWeights[aIndex]);
							BitmapColor aColor;

							aSum += aWeight;

							if(pReadAcc->HasPalette())
							{
								aColor = pReadAcc->GetPaletteColor(pReadAcc->GetPixelIndex(y, pPixels[aIndex]));
							}
							else
							{
								aColor = pReadAcc->GetPixel(y, pPixels[aIndex]);
							}

							aValueRed += aWeight * aColor.GetRed();
							aValueGreen += aWeight * aColor.GetGreen();
							aValueBlue += aWeight * aColor.GetBlue();
						}

						const BitmapColor aResultColor(
							static_cast< sal_uInt8 >(MinMax(static_cast< sal_Int32 >(aValueRed / aSum), 0, 255)),
							static_cast< sal_uInt8 >(MinMax(static_cast< sal_Int32 >(aValueGreen / aSum), 0, 255)),
							static_cast< sal_uInt8 >(MinMax(static_cast< sal_Int32 >(aValueBlue / aSum), 0, 255)));

						pWriteAcc->SetPixel(y, x, aResultColor);
					}
				}

				rTarget.ReleaseAccess(pWriteAcc);
			}

			rSource.ReleaseAccess(pReadAcc);
			delete[] pWeights;
			delete[] pCount;
			delete[] pPixels;

			if(bResult)
			{
				return true;
			}
		}

		return false;
	}

	bool ImplScaleConvolutionVer(
		Bitmap& rSource,
		Bitmap& rTarget,
		const double& rScaleY,
		const Kernel& aKernel)
	{
		// Do vertical filtering
		OSL_ENSURE(rScaleY > 0.0, "Error in scaling: Mirror given in non-mirror-capable method (!)");
		const sal_uInt32 nHeight(rSource.GetSizePixel().Height());
		const sal_uInt32 nNewHeight(FRound(nHeight * rScaleY));

		if(nHeight == nNewHeight)
		{
			return true;
		}

		BitmapReadAccess* pReadAcc = rSource.AcquireReadAccess();

		if(pReadAcc)
		{
			double* pWeights = 0;
			sal_uInt32* pPixels = 0;
			sal_uInt32* pCount = 0;
			sal_uInt32 aNumberOfContributions(0);

			const sal_uInt32 nWidth(rSource.GetSizePixel().Width());
			ImplCalculateContributions(nHeight, nNewHeight, aNumberOfContributions, pWeights, pPixels, pCount, aKernel);
			rTarget = Bitmap(Size(nWidth, nNewHeight), 24);
			BitmapWriteAccess* pWriteAcc = rTarget.AcquireWriteAccess();
			bool bResult(0 != pWriteAcc);

			if(pWriteAcc)
			{
				for(sal_uInt32 x(0); x < nWidth; x++)
				{
					for(sal_uInt32 y(0); y < nNewHeight; y++)
					{
						const sal_uInt32 aBaseIndex(y * aNumberOfContributions);
						double aSum(0.0);
						double aValueRed(0.0);
						double aValueGreen(0.0);
						double aValueBlue(0.0);

						for(sal_uInt32 j(0); j < pCount[y]; j++)
						{
							const sal_uInt32 aIndex(aBaseIndex + j);
							const double aWeight(pWeights[aIndex]);
							BitmapColor aColor;

							aSum += aWeight;

							if(pReadAcc->HasPalette())
							{
								aColor = pReadAcc->GetPaletteColor(pReadAcc->GetPixelIndex(pPixels[aIndex], x));
							}
							else
							{
								aColor = pReadAcc->GetPixel(pPixels[aIndex], x);
							}

							aValueRed += aWeight * aColor.GetRed();
							aValueGreen += aWeight * aColor.GetGreen();
							aValueBlue += aWeight * aColor.GetBlue();
						}

						const BitmapColor aResultColor(
							static_cast< sal_uInt8 >(MinMax(static_cast< sal_Int32 >(aValueRed / aSum), 0, 255)),
							static_cast< sal_uInt8 >(MinMax(static_cast< sal_Int32 >(aValueGreen / aSum), 0, 255)),
							static_cast< sal_uInt8 >(MinMax(static_cast< sal_Int32 >(aValueBlue / aSum), 0, 255)));

						if(pWriteAcc->HasPalette())
						{
							pWriteAcc->SetPixelIndex(y, x, static_cast< sal_uInt8 >(pWriteAcc->GetBestPaletteIndex(aResultColor)));
						}
						else
						{
							pWriteAcc->SetPixel(y, x, aResultColor);
						}
					}
				}
			}

			rTarget.ReleaseAccess(pWriteAcc);
			rSource.ReleaseAccess(pReadAcc);

			delete[] pWeights;
			delete[] pCount;
			delete[] pPixels;

			if(bResult)
			{
				return true;
			}
		}

		return false;
	}
}

// #121233# Added BMP_SCALE_LANCZOS, BMP_SCALE_BICUBIC, BMP_SCALE_BILINEAR and
// BMP_SCALE_BOX derived from the original commit from Tomaž Vajngerl (see
// Bugzilla task for details) Thanks!
sal_Bool Bitmap::ImplScaleConvolution(
	const double& rScaleX,
	const double& rScaleY,
	const Kernel& aKernel)
{
	const bool bMirrorHor(rScaleX < 0.0);
	const bool bMirrorVer(rScaleY < 0.0);
	const double fScaleX(bMirrorHor ? -rScaleX : rScaleX);
	const double fScaleY(bMirrorVer ? -rScaleY : rScaleY);
	const sal_uInt32 nWidth(GetSizePixel().Width());
	const sal_uInt32 nHeight(GetSizePixel().Height());
	const sal_uInt32 nNewWidth(FRound(nWidth * fScaleX));
	const sal_uInt32 nNewHeight(FRound(nHeight * fScaleY));
	const bool bScaleHor(nWidth != nNewWidth);
	const bool bScaleVer(nHeight != nNewHeight);
	const bool bMirror(bMirrorHor || bMirrorVer);

	if(!bMirror && !bScaleHor && !bScaleVer)
	{
		return true;
	}

	bool bResult(true);
	sal_uInt32 nMirrorFlags(BMP_MIRROR_NONE);
	bool bMirrorAfter(false);

	if(bMirror)
	{
		if(bMirrorHor)
		{
			nMirrorFlags |= BMP_MIRROR_HORZ;
		}

		if(bMirrorVer)
		{
			nMirrorFlags |= BMP_MIRROR_VERT;
		}

		const sal_uInt32 nStartSize(nWidth * nHeight);
		const sal_uInt32 nEndSize(nNewWidth * nNewHeight);

		bMirrorAfter = nStartSize > nEndSize;

		if(!bMirrorAfter)
		{
			bResult = Mirror(nMirrorFlags);
		}
	}

	Bitmap aResult;

	if(bResult)
	{
		const sal_uInt32 nInBetweenSizeHorFirst(nHeight * nNewWidth);
		const sal_uInt32 nInBetweenSizeVerFirst(nNewHeight * nWidth);
		Bitmap aSource(*this);

		if(nInBetweenSizeHorFirst < nInBetweenSizeVerFirst)
		{
			if(bScaleHor)
			{
				bResult = ImplScaleConvolutionHor(aSource, aResult, fScaleX, aKernel);
			}

			if(bResult && bScaleVer)
			{
				if(bScaleHor)
				{
					// copy partial result, independent of color depth
					aSource = aResult;
				}

				bResult = ImplScaleConvolutionVer(aSource, aResult, fScaleY, aKernel);
			}
		}
		else
		{
			if(bScaleVer)
			{
				bResult = ImplScaleConvolutionVer(aSource, aResult, fScaleY, aKernel);
			}

			if(bResult && bScaleHor)
			{
				if(bScaleVer)
				{
					// copy partial result, independent of color depth
					aSource = aResult;
				}

				bResult = ImplScaleConvolutionHor(aSource, aResult, fScaleX, aKernel);
			}
		}
	}

	if(bResult && bMirrorAfter)
	{
		bResult = aResult.Mirror(nMirrorFlags);
	}

	if(bResult)
	{
		ImplAdaptBitCount(aResult);
		*this = aResult;
	}

	return bResult;
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::Dither( sal_uLong nDitherFlags )
{
	sal_Bool bRet = sal_False;

	const Size aSizePix( GetSizePixel() );

	if( aSizePix.Width() == 1 || aSizePix.Height() == 1 )
		bRet = sal_True;
	else if( nDitherFlags & BMP_DITHER_MATRIX )
		bRet = ImplDitherMatrix();
	else if( nDitherFlags & BMP_DITHER_FLOYD )
		bRet = ImplDitherFloyd();
	else if( ( nDitherFlags & BMP_DITHER_FLOYD_16 ) && ( GetBitCount() == 24 ) )
		bRet = ImplDitherFloyd16();

	return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::ImplDitherMatrix()
{
	BitmapReadAccess*	pReadAcc = AcquireReadAccess();
	Bitmap				aNewBmp( GetSizePixel(), 8 );
	BitmapWriteAccess*	pWriteAcc = aNewBmp.AcquireWriteAccess();
	sal_Bool				bRet = sal_False;

	if( pReadAcc && pWriteAcc )
	{
		const sal_uLong	nWidth = pReadAcc->Width();
		const sal_uLong	nHeight = pReadAcc->Height();
		BitmapColor	aIndex( (sal_uInt8) 0 );

		if( pReadAcc->HasPalette() )
		{
			for( sal_uLong nY = 0UL; nY < nHeight; nY++ )
			{
				for( sal_uLong nX = 0UL, nModY = ( nY & 0x0FUL ) << 4UL; nX < nWidth; nX++ )
				{
					const BitmapColor	aCol( pReadAcc->GetPaletteColor( pReadAcc->GetPixelIndex( nY, nX ) ) );
					const sal_uLong			nD = nVCLDitherLut[ nModY + ( nX & 0x0FUL ) ];
					const sal_uLong			nR = ( nVCLLut[ aCol.GetRed() ] + nD ) >> 16UL;
					const sal_uLong			nG = ( nVCLLut[ aCol.GetGreen() ] + nD ) >> 16UL;
					const sal_uLong			nB = ( nVCLLut[ aCol.GetBlue() ] + nD ) >> 16UL;

					aIndex.SetIndex( (sal_uInt8) ( nVCLRLut[ nR ] + nVCLGLut[ nG ] + nVCLBLut[ nB ] ) );
					pWriteAcc->SetPixel( nY, nX, aIndex );
				}
			}
		}
		else
		{
			for( sal_uLong nY = 0UL; nY < nHeight; nY++ )
			{
				for( sal_uLong nX = 0UL, nModY = ( nY & 0x0FUL ) << 4UL; nX < nWidth; nX++ )
				{
					const BitmapColor	aCol( pReadAcc->GetPixel( nY, nX ) );
					const sal_uLong			nD = nVCLDitherLut[ nModY + ( nX & 0x0FUL ) ];
					const sal_uLong			nR = ( nVCLLut[ aCol.GetRed() ] + nD ) >> 16UL;
					const sal_uLong			nG = ( nVCLLut[ aCol.GetGreen() ] + nD ) >> 16UL;
					const sal_uLong			nB = ( nVCLLut[ aCol.GetBlue() ] + nD ) >> 16UL;

					aIndex.SetIndex( (sal_uInt8) ( nVCLRLut[ nR ] + nVCLGLut[ nG ] + nVCLBLut[ nB ] ) );
					pWriteAcc->SetPixel( nY, nX, aIndex );
				}
			}
		}

		bRet = sal_True;
	}

	ReleaseAccess( pReadAcc );
	aNewBmp.ReleaseAccess( pWriteAcc );

	if( bRet )
	{
		const MapMode	aMap( maPrefMapMode );
		const Size		aSize( maPrefSize );

		*this = aNewBmp;

		maPrefMapMode = aMap;
		maPrefSize = aSize;
	}

	return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::ImplDitherFloyd()
{
	const Size	aSize( GetSizePixel() );
	sal_Bool		bRet = sal_False;

	if( ( aSize.Width() > 3 ) && ( aSize.Height() > 2 ) )
	{
		BitmapReadAccess*	pReadAcc = AcquireReadAccess();
		Bitmap				aNewBmp( GetSizePixel(), 8 );
		BitmapWriteAccess*	pWriteAcc = aNewBmp.AcquireWriteAccess();

		if( pReadAcc && pWriteAcc )
		{
			BitmapColor	aColor;
			long		nWidth = pReadAcc->Width();
			long		nWidth1 = nWidth - 1L;
			long		nHeight = pReadAcc->Height();
			long		nX;
			long		nW = nWidth * 3L;
			long		nW2 = nW - 3L;
			long		nRErr, nGErr, nBErr;
			long		nRC, nGC, nBC;
			long		nTemp;
			long		nZ;
			long*		p1 = new long[ nW ];
			long*		p2 = new long[ nW ];
			long*		p1T = p1;
			long*		p2T = p2;
			long*		pTmp;
			sal_Bool		bPal = pReadAcc->HasPalette();

			pTmp = p2T;

			if( bPal )
			{
				for( nZ = 0; nZ < nWidth; nZ++ )
				{
					aColor = pReadAcc->GetPaletteColor( pReadAcc->GetPixelIndex( 0, nZ ) );

					*pTmp++ = (long) aColor.GetBlue() << 12;
					*pTmp++ = (long) aColor.GetGreen() << 12;
					*pTmp++ = (long) aColor.GetRed() << 12;
				}
			}
			else
			{
				for( nZ = 0; nZ < nWidth; nZ++ )
				{
					aColor = pReadAcc->GetPixel( 0, nZ );

					*pTmp++ = (long) aColor.GetBlue() << 12;
					*pTmp++ = (long) aColor.GetGreen() << 12;
					*pTmp++ = (long) aColor.GetRed() << 12;
				}
			}

			for( long nY = 1, nYAcc = 0L; nY <= nHeight; nY++, nYAcc++ )
			{
				pTmp = p1T;
				p1T = p2T;
				p2T = pTmp;

				if( nY < nHeight )
				{
					if( bPal )
					{
						for( nZ = 0; nZ < nWidth; nZ++ )
						{
							aColor = pReadAcc->GetPaletteColor( pReadAcc->GetPixelIndex( nY, nZ ) );

							*pTmp++ = (long) aColor.GetBlue() << 12;
							*pTmp++ = (long) aColor.GetGreen() << 12;
							*pTmp++ = (long) aColor.GetRed() << 12;
						}
					}
					else
					{
						for( nZ = 0; nZ < nWidth; nZ++ )
						{
							aColor = pReadAcc->GetPixel( nY, nZ );

							*pTmp++ = (long) aColor.GetBlue() << 12;
							*pTmp++ = (long) aColor.GetGreen() << 12;
							*pTmp++ = (long) aColor.GetRed() << 12;
						}
					}
				}

				// erstes Pixel gesondert betrachten
				nX = 0;
				CALC_ERRORS;
				CALC_TABLES7;
				nX -= 5;
				CALC_TABLES5;
				pWriteAcc->SetPixelIndex( nYAcc, 0, static_cast<sal_uInt8>(nVCLBLut[ nBC ] + nVCLGLut[nGC ] + nVCLRLut[nRC ]) );

				// mittlere Pixel ueber Schleife
				long nXAcc;
				for ( nX = 3L, nXAcc = 1L; nX < nW2; nXAcc++ )
				{
					CALC_ERRORS;
					CALC_TABLES7;
					nX -= 8;
					CALC_TABLES3;
					CALC_TABLES5;
					pWriteAcc->SetPixelIndex( nYAcc, nXAcc, static_cast<sal_uInt8>(nVCLBLut[ nBC ] + nVCLGLut[nGC ] + nVCLRLut[nRC ]) );
				}

				// letztes Pixel gesondert betrachten
				CALC_ERRORS;
				nX -= 5;
				CALC_TABLES3;
				CALC_TABLES5;
				pWriteAcc->SetPixelIndex( nYAcc, nWidth1, static_cast<sal_uInt8>(nVCLBLut[ nBC ] + nVCLGLut[nGC ] + nVCLRLut[nRC ]) );
			}

			delete[] p1;
			delete[] p2;
			bRet = sal_True;
		}

		ReleaseAccess( pReadAcc );
		aNewBmp.ReleaseAccess( pWriteAcc );

		if( bRet )
		{
			const MapMode	aMap( maPrefMapMode );
			const Size		aPrefSize( maPrefSize );

			*this = aNewBmp;

			maPrefMapMode = aMap;
			maPrefSize = aPrefSize;
		}
	}

	return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::ImplDitherFloyd16()
{
	BitmapReadAccess*	pReadAcc = AcquireReadAccess();
	Bitmap				aNewBmp( GetSizePixel(), 24 );
	BitmapWriteAccess*	pWriteAcc = aNewBmp.AcquireWriteAccess();
	sal_Bool				bRet = sal_False;

	if( pReadAcc && pWriteAcc )
	{
		const long		nWidth = pWriteAcc->Width();
		const long		nWidth1 = nWidth - 1L;
		const long		nHeight = pWriteAcc->Height();
		BitmapColor 	aColor;
		BitmapColor		aBestCol;
		ImpErrorQuad	aErrQuad;
		ImpErrorQuad*	pErrQuad1 = new ImpErrorQuad[ nWidth ];
		ImpErrorQuad*	pErrQuad2 = new ImpErrorQuad[ nWidth ];
		ImpErrorQuad*	pQLine1 = pErrQuad1;
		ImpErrorQuad*	pQLine2 = 0;
		long			nX, nY;
		long			nYTmp = 0L;
		sal_Bool			bQ1 = sal_True;

		for( nY = 0L; nY < Min( nHeight, 2L ); nY++, nYTmp++ )
			for( nX = 0L, pQLine2 = !nY ? pErrQuad1 : pErrQuad2; nX < nWidth; nX++ )
				pQLine2[ nX ] = pReadAcc->GetPixel( nYTmp, nX );

		for( nY = 0L; nY < nHeight; nY++, nYTmp++ )
		{
			// erstes ZeilenPixel
			aBestCol = pQLine1[ 0 ].ImplGetColor();
			aBestCol.SetRed( ( aBestCol.GetRed() & 248 ) | 7 );
			aBestCol.SetGreen( ( aBestCol.GetGreen() & 248 ) | 7 );
			aBestCol.SetBlue( ( aBestCol.GetBlue() & 248 ) | 7 );
			pWriteAcc->SetPixel( nY, 0, aBestCol );

			for( nX = 1L; nX < nWidth1; nX++ )
			{
				aColor = pQLine1[ nX ].ImplGetColor();
				aBestCol.SetRed( ( aColor.GetRed() & 248 ) | 7 );
				aBestCol.SetGreen( ( aColor.GetGreen() & 248 ) | 7 );
				aBestCol.SetBlue( ( aColor.GetBlue() & 248 ) | 7 );
				aErrQuad = ( ImpErrorQuad( aColor ) -= aBestCol );
				pQLine1[ ++nX ].ImplAddColorError7( aErrQuad );
				pQLine2[ nX-- ].ImplAddColorError1( aErrQuad );
				pQLine2[ nX-- ].ImplAddColorError5( aErrQuad );
				pQLine2[ nX++ ].ImplAddColorError3( aErrQuad );
				pWriteAcc->SetPixel( nY, nX, aBestCol );
			}

			// letztes ZeilenPixel
			aBestCol = pQLine1[ nWidth1 ].ImplGetColor();
			aBestCol.SetRed( ( aBestCol.GetRed() & 248 ) | 7 );
			aBestCol.SetGreen( ( aBestCol.GetGreen() & 248 ) | 7 );
			aBestCol.SetBlue( ( aBestCol.GetBlue() & 248 ) | 7 );
			pWriteAcc->SetPixel( nY, nX, aBestCol );

			// Zeilenpuffer neu fuellen/kopieren
			pQLine1 = pQLine2;
			pQLine2 = ( bQ1 = !bQ1 ) != sal_False ? pErrQuad2 : pErrQuad1;

			if( nYTmp < nHeight )
				for( nX = 0L; nX < nWidth; nX++ )
					pQLine2[ nX ] = pReadAcc->GetPixel( nYTmp, nX );
		}

		// Zeilenpuffer zerstoeren
		delete[] pErrQuad1;
		delete[] pErrQuad2;
		bRet = sal_True;
	}

	ReleaseAccess( pReadAcc );
	aNewBmp.ReleaseAccess( pWriteAcc );

	if( bRet )
	{
		const MapMode	aMap( maPrefMapMode );
		const Size		aSize( maPrefSize );

		*this = aNewBmp;

		maPrefMapMode = aMap;
		maPrefSize = aSize;
	}

	return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::ReduceColors( sal_uInt16 nColorCount, BmpReduce eReduce )
{
	sal_Bool bRet;

	if( GetColorCount() <= (sal_uLong) nColorCount )
		bRet = sal_True;
	else if( nColorCount )
	{
		if( BMP_REDUCE_SIMPLE == eReduce )
			bRet = ImplReduceSimple( nColorCount );
		else if( BMP_REDUCE_POPULAR == eReduce )
			bRet = ImplReducePopular( nColorCount );
		else
			bRet = ImplReduceMedian( nColorCount );
	}
	else
		bRet = sal_False;

	return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::ImplReduceSimple( sal_uInt16 nColorCount )
{
	Bitmap				aNewBmp;
	BitmapReadAccess*	pRAcc = AcquireReadAccess();
	const sal_uInt16		nColCount = Min( nColorCount, (sal_uInt16) 256 );
	sal_uInt16				nBitCount;
	sal_Bool				bRet = sal_False;

	if( nColCount <= 2 )
		nBitCount = 1;
	else if( nColCount <= 16 )
		nBitCount = 4;
	else
		nBitCount = 8;

	if( pRAcc )
	{
		Octree					aOct( *pRAcc, nColCount );
		const BitmapPalette&	rPal = aOct.GetPalette();
		BitmapWriteAccess*		pWAcc;

		aNewBmp = Bitmap( GetSizePixel(), nBitCount, &rPal );
		pWAcc = aNewBmp.AcquireWriteAccess();

		if( pWAcc )
		{
			const long nWidth = pRAcc->Width();
			const long nHeight = pRAcc->Height();

			if( pRAcc->HasPalette() )
			{
				for( long nY = 0L; nY < nHeight; nY++ )
					for( long nX =0L; nX < nWidth; nX++ )
						pWAcc->SetPixelIndex( nY, nX, static_cast<sal_uInt8>(aOct.GetBestPaletteIndex( pRAcc->GetPaletteColor( pRAcc->GetPixelIndex( nY, nX ) ))) );
			}
			else
			{
				for( long nY = 0L; nY < nHeight; nY++ )
					for( long nX =0L; nX < nWidth; nX++ )
						pWAcc->SetPixelIndex( nY, nX, static_cast<sal_uInt8>(aOct.GetBestPaletteIndex( pRAcc->GetPixel( nY, nX ) )) );
			}

			aNewBmp.ReleaseAccess( pWAcc );
			bRet = sal_True;
		}

		ReleaseAccess( pRAcc );
	}

	if( bRet )
	{
		const MapMode	aMap( maPrefMapMode );
		const Size		aSize( maPrefSize );

		*this = aNewBmp;
		maPrefMapMode = aMap;
		maPrefSize = aSize;
	}

	return bRet;
}

// ------------------------------------------------------------------------

struct PopularColorCount
{
	sal_uInt32	mnIndex;
	sal_uInt32	mnCount;
};

// ------------------------------------------------------------------------

extern "C" int __LOADONCALLAPI ImplPopularCmpFnc( const void* p1, const void* p2 )
{
	int nRet;

	if( ( (PopularColorCount*) p1 )->mnCount < ( (PopularColorCount*) p2 )->mnCount )
		nRet = 1;
	else if( ( (PopularColorCount*) p1 )->mnCount == ( (PopularColorCount*) p2 )->mnCount )
		nRet = 0;
	else
		nRet = -1;

	return nRet;
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::ImplReducePopular( sal_uInt16 nColCount )
{
	BitmapReadAccess*	pRAcc = AcquireReadAccess();
	sal_uInt16				nBitCount;
	sal_Bool				bRet = sal_False;

	if( nColCount > 256 )
		nColCount = 256;

	if( nColCount < 17 )
		nBitCount = 4;
	else
		nBitCount = 8;

	if( pRAcc )
	{
		const sal_uInt32	nValidBits = 4;
		const sal_uInt32	nRightShiftBits = 8 - nValidBits;
		const sal_uInt32	nLeftShiftBits1 = nValidBits;
		const sal_uInt32	nLeftShiftBits2 = nValidBits << 1;
		const sal_uInt32	nColorsPerComponent = 1 << nValidBits;
		const sal_uInt32	nColorOffset = 256 / nColorsPerComponent;
		const sal_uInt32	nTotalColors = nColorsPerComponent * nColorsPerComponent * nColorsPerComponent;
		const long			nWidth = pRAcc->Width();
		const long			nHeight = pRAcc->Height();
		PopularColorCount*	pCountTable = new PopularColorCount[ nTotalColors ];
		long				nX, nY, nR, nG, nB, nIndex;

		rtl_zeroMemory( pCountTable, nTotalColors * sizeof( PopularColorCount ) );

		for( nR = 0, nIndex = 0; nR < 256; nR += nColorOffset )
		{
			for( nG = 0; nG < 256; nG += nColorOffset )
			{
				for( nB = 0; nB < 256; nB += nColorOffset )
				{
					pCountTable[ nIndex ].mnIndex = nIndex;
					nIndex++;
				}
			}
		}

		if( pRAcc->HasPalette() )
		{
			for( nY = 0L; nY < nHeight; nY++ )
			{
				for( nX = 0L; nX < nWidth; nX++ )
				{
					const BitmapColor& rCol = pRAcc->GetPaletteColor( pRAcc->GetPixelIndex( nY, nX ) );
					pCountTable[ ( ( ( (sal_uInt32) rCol.GetRed() ) >> nRightShiftBits ) << nLeftShiftBits2 ) |
								 ( ( ( (sal_uInt32) rCol.GetGreen() ) >> nRightShiftBits ) << nLeftShiftBits1 ) |
								 ( ( (sal_uInt32) rCol.GetBlue() ) >> nRightShiftBits ) ].mnCount++;
				}
			}
		}
		else
		{
			for( nY = 0L; nY < nHeight; nY++ )
			{
				for( nX = 0L; nX < nWidth; nX++ )
				{
					const BitmapColor aCol( pRAcc->GetPixel( nY, nX ) );
					pCountTable[ ( ( ( (sal_uInt32) aCol.GetRed() ) >> nRightShiftBits ) << nLeftShiftBits2 ) |
								 ( ( ( (sal_uInt32) aCol.GetGreen() ) >> nRightShiftBits ) << nLeftShiftBits1 ) |
								 ( ( (sal_uInt32) aCol.GetBlue() ) >> nRightShiftBits ) ].mnCount++;
				}
			}
		}

		BitmapPalette aNewPal( nColCount );

		qsort( pCountTable, nTotalColors, sizeof( PopularColorCount ), ImplPopularCmpFnc );

		for( sal_uInt16 n = 0; n < nColCount; n++ )
		{
			const PopularColorCount& rPop = pCountTable[ n ];
			aNewPal[ n ] = BitmapColor( (sal_uInt8) ( ( rPop.mnIndex >> nLeftShiftBits2 ) << nRightShiftBits ),
										(sal_uInt8) ( ( ( rPop.mnIndex >> nLeftShiftBits1 ) & ( nColorsPerComponent - 1 ) ) << nRightShiftBits ),
										(sal_uInt8) ( ( rPop.mnIndex & ( nColorsPerComponent - 1 ) ) << nRightShiftBits ) );
		}

		Bitmap				aNewBmp( GetSizePixel(), nBitCount, &aNewPal );
		BitmapWriteAccess*	pWAcc = aNewBmp.AcquireWriteAccess();

		if( pWAcc )
		{
			BitmapColor	aDstCol( (sal_uInt8) 0 );
			sal_uInt8*		pIndexMap = new sal_uInt8[ nTotalColors ];

			for( nR = 0, nIndex = 0; nR < 256; nR += nColorOffset )
				for( nG = 0; nG < 256; nG += nColorOffset )
					for( nB = 0; nB < 256; nB += nColorOffset )
						pIndexMap[ nIndex++ ] = (sal_uInt8) aNewPal.GetBestIndex( BitmapColor( (sal_uInt8) nR, (sal_uInt8) nG, (sal_uInt8) nB ) );

			if( pRAcc->HasPalette() )
			{
				for( nY = 0L; nY < nHeight; nY++ )
				{
					for( nX = 0L; nX < nWidth; nX++ )
					{
						const BitmapColor& rCol = pRAcc->GetPaletteColor( pRAcc->GetPixelIndex( nY, nX ) );
						aDstCol.SetIndex( pIndexMap[ ( ( ( (sal_uInt32) rCol.GetRed() ) >> nRightShiftBits ) << nLeftShiftBits2 ) |
													 ( ( ( (sal_uInt32) rCol.GetGreen() ) >> nRightShiftBits ) << nLeftShiftBits1 ) |
													 ( ( (sal_uInt32) rCol.GetBlue() ) >> nRightShiftBits ) ] );
						pWAcc->SetPixel( nY, nX, aDstCol );
					}
				}
			}
			else
			{
				for( nY = 0L; nY < nHeight; nY++ )
				{
					for( nX = 0L; nX < nWidth; nX++ )
					{
						const BitmapColor aCol( pRAcc->GetPixel( nY, nX ) );
						aDstCol.SetIndex( pIndexMap[ ( ( ( (sal_uInt32) aCol.GetRed() ) >> nRightShiftBits ) << nLeftShiftBits2 ) |
													 ( ( ( (sal_uInt32) aCol.GetGreen() ) >> nRightShiftBits ) << nLeftShiftBits1 ) |
													 ( ( (sal_uInt32) aCol.GetBlue() ) >> nRightShiftBits ) ] );
						pWAcc->SetPixel( nY, nX, aDstCol );
					}
				}
			}

			delete[] pIndexMap;
			aNewBmp.ReleaseAccess( pWAcc );
			bRet = sal_True;
		}

		delete[] pCountTable;
		ReleaseAccess( pRAcc );

		if( bRet )
		{
			const MapMode	aMap( maPrefMapMode );
			const Size		aSize( maPrefSize );

			*this = aNewBmp;
			maPrefMapMode = aMap;
			maPrefSize = aSize;
		}
	}

	return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::ImplReduceMedian( sal_uInt16 nColCount )
{
	BitmapReadAccess*	pRAcc = AcquireReadAccess();
	sal_uInt16				nBitCount;
	sal_Bool				bRet = sal_False;

	if( nColCount < 17 )
		nBitCount = 4;
	else if( nColCount < 257 )
		nBitCount = 8;
	else
	{
		DBG_ERROR( "Bitmap::ImplReduceMedian(): invalid color count!" );
		nBitCount = 8;
		nColCount = 256;
	}

	if( pRAcc )
	{
		Bitmap				aNewBmp( GetSizePixel(), nBitCount );
		BitmapWriteAccess*	pWAcc = aNewBmp.AcquireWriteAccess();

		if( pWAcc )
		{
			const sal_uLong	nSize = 32768UL * sizeof( sal_uLong );
			sal_uLong*		pColBuf = (sal_uLong*) rtl_allocateMemory( nSize );
			const long	nWidth = pWAcc->Width();
			const long	nHeight = pWAcc->Height();
			long		nIndex = 0L;

			memset( (HPBYTE) pColBuf, 0, nSize );

			// create Buffer
			if( pRAcc->HasPalette() )
			{
				for( long nY = 0L; nY < nHeight; nY++ )
				{
					for( long nX = 0L; nX < nWidth; nX++ )
					{
						const BitmapColor& rCol = pRAcc->GetPaletteColor( pRAcc->GetPixelIndex( nY, nX ) );
						pColBuf[ RGB15( rCol.GetRed() >> 3, rCol.GetGreen() >> 3, rCol.GetBlue() >> 3 ) ]++;
					}
				}
			}
			else
			{
				for( long nY = 0L; nY < nHeight; nY++ )
				{
					for( long nX = 0L; nX < nWidth; nX++ )
					{
						const BitmapColor aCol( pRAcc->GetPixel( nY, nX ) );
						pColBuf[ RGB15( aCol.GetRed() >> 3, aCol.GetGreen() >> 3, aCol.GetBlue() >> 3 ) ]++;
					}
				}
			}

			// create palette via median cut
			BitmapPalette aPal( pWAcc->GetPaletteEntryCount() );
			ImplMedianCut( pColBuf, aPal, 0, 31, 0, 31, 0, 31,
						   nColCount, nWidth * nHeight, nIndex );

			// do mapping of colors to palette
			InverseColorMap aMap( aPal );
			pWAcc->SetPalette( aPal );
			for( long nY = 0L; nY < nHeight; nY++ )
				for( long nX = 0L; nX < nWidth; nX++ )
					pWAcc->SetPixelIndex( nY, nX, static_cast<sal_uInt8>( aMap.GetBestPaletteIndex( pRAcc->GetColor( nY, nX ) )) );

			rtl_freeMemory( pColBuf );
			aNewBmp.ReleaseAccess( pWAcc );
			bRet = sal_True;
		}

		ReleaseAccess( pRAcc );

		if( bRet )
		{
			const MapMode	aMap( maPrefMapMode );
			const Size		aSize( maPrefSize );

			*this = aNewBmp;
			maPrefMapMode = aMap;
			maPrefSize = aSize;
		}
	}

	return bRet;
}

// ------------------------------------------------------------------------

void Bitmap::ImplMedianCut( sal_uLong* pColBuf, BitmapPalette& rPal,
							long nR1, long nR2, long nG1, long nG2, long nB1, long nB2,
							long nColors, long nPixels, long& rIndex )
{
	if( !nPixels )
		return;

	BitmapColor	aCol;
	const long	nRLen = nR2 - nR1;
	const long	nGLen = nG2 - nG1;
	const long	nBLen = nB2 - nB1;
	long 		nR, nG, nB;
	sal_uLong*		pBuf = pColBuf;

	if( !nRLen && !nGLen && !nBLen )
	{
		if( pBuf[ RGB15( nR1, nG1, nB1 ) ] )
		{
			aCol.SetRed( (sal_uInt8) ( nR1 << 3 ) );
			aCol.SetGreen( (sal_uInt8) ( nG1 << 3 ) );
			aCol.SetBlue( (sal_uInt8) ( nB1 << 3 ) );
			rPal[ (sal_uInt16) rIndex++ ] = aCol;
		}
	}
	else
	{
		if( 1 == nColors || 1 == nPixels )
		{
			long nPixSum = 0, nRSum = 0, nGSum = 0, nBSum = 0;

			for( nR = nR1; nR <= nR2; nR++ )
			{
				for( nG = nG1; nG <= nG2; nG++ )
				{
					for( nB = nB1; nB <= nB2; nB++ )
					{
						nPixSum = pBuf[ RGB15( nR, nG, nB ) ];

						if( nPixSum )
						{
							nRSum += nR * nPixSum;
							nGSum += nG * nPixSum;
							nBSum += nB * nPixSum;
						}
					}
				}
			}

			aCol.SetRed( (sal_uInt8) ( ( nRSum / nPixels ) << 3 ) );
			aCol.SetGreen( (sal_uInt8) ( ( nGSum / nPixels ) << 3 ) );
			aCol.SetBlue( (sal_uInt8) ( ( nBSum / nPixels ) << 3 ) );
			rPal[ (sal_uInt16) rIndex++ ] = aCol;
		}
		else
		{
			const long	nTest = ( nPixels >> 1 );
			long		nPixOld = 0;
			long		nPixNew = 0;

			if( nBLen > nGLen && nBLen > nRLen )
			{
				nB = nB1 - 1;

				while( nPixNew < nTest )
				{
					nB++, nPixOld = nPixNew;
					for( nR = nR1; nR <= nR2; nR++ )
						for( nG = nG1; nG <= nG2; nG++ )
							nPixNew += pBuf[ RGB15( nR, nG, nB ) ];
				}

				if( nB < nB2 )
				{
					ImplMedianCut( pBuf, rPal, nR1, nR2, nG1, nG2, nB1, nB, nColors >> 1, nPixNew, rIndex );
					ImplMedianCut( pBuf, rPal, nR1, nR2, nG1, nG2, nB + 1, nB2, nColors >> 1, nPixels - nPixNew, rIndex );
				}
				else
				{
					ImplMedianCut( pBuf, rPal, nR1, nR2, nG1, nG2, nB1, nB - 1, nColors >> 1, nPixOld, rIndex );
					ImplMedianCut( pBuf, rPal, nR1, nR2, nG1, nG2, nB, nB2, nColors >> 1, nPixels - nPixOld, rIndex );
				}
			}
			else if( nGLen > nRLen )
			{
				nG = nG1 - 1;

				while( nPixNew < nTest )
				{
					nG++, nPixOld = nPixNew;
					for( nR = nR1; nR <= nR2; nR++ )
						for( nB = nB1; nB <= nB2; nB++ )
							nPixNew += pBuf[ RGB15( nR, nG, nB ) ];
				}

				if( nG < nG2 )
				{
					ImplMedianCut( pBuf, rPal, nR1, nR2, nG1, nG, nB1, nB2, nColors >> 1, nPixNew, rIndex );
					ImplMedianCut( pBuf, rPal, nR1, nR2, nG + 1, nG2, nB1, nB2, nColors >> 1, nPixels - nPixNew, rIndex );
				}
				else
				{
					ImplMedianCut( pBuf, rPal, nR1, nR2, nG1, nG - 1, nB1, nB2, nColors >> 1, nPixOld, rIndex );
					ImplMedianCut( pBuf, rPal, nR1, nR2, nG, nG2, nB1, nB2, nColors >> 1, nPixels - nPixOld, rIndex );
				}
			}
			else
			{
				nR = nR1 - 1;

				while( nPixNew < nTest )
				{
					nR++, nPixOld = nPixNew;
					for( nG = nG1; nG <= nG2; nG++ )
						for( nB = nB1; nB <= nB2; nB++ )
							nPixNew += pBuf[ RGB15( nR, nG, nB ) ];
				}

				if( nR < nR2 )
				{
					ImplMedianCut( pBuf, rPal, nR1, nR, nG1, nG2, nB1, nB2, nColors >> 1, nPixNew, rIndex );
					ImplMedianCut( pBuf, rPal, nR1 + 1, nR2, nG1, nG2, nB1, nB2, nColors >> 1, nPixels - nPixNew, rIndex );
				}
				else
				{
					ImplMedianCut( pBuf, rPal, nR1, nR - 1, nG1, nG2, nB1, nB2, nColors >> 1, nPixOld, rIndex );
					ImplMedianCut( pBuf, rPal, nR, nR2, nG1, nG2, nB1, nB2, nColors >> 1, nPixels - nPixOld, rIndex );
				}
			}
		}
	}
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::Vectorize( PolyPolygon& rPolyPoly, sal_uLong nFlags, const Link* pProgress )
{
	return ImplVectorizer().ImplVectorize( *this, rPolyPoly, nFlags, pProgress );
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::Vectorize( GDIMetaFile& rMtf, sal_uInt8 cReduce, sal_uLong nFlags, const Link* pProgress )
{
	return ImplVectorizer().ImplVectorize( *this, rMtf, cReduce, nFlags, pProgress );
}

// ------------------------------------------------------------------------

sal_Bool Bitmap::Adjust( short nLuminancePercent, short nContrastPercent,
					 short nChannelRPercent, short nChannelGPercent, short nChannelBPercent,
					 double fGamma, sal_Bool bInvert )
{
	sal_Bool bRet = sal_False;

	// nothing to do => return quickly
	if( !nLuminancePercent && !nContrastPercent &&
		!nChannelRPercent && !nChannelGPercent && !nChannelBPercent &&
		( fGamma == 1.0 ) && !bInvert )
	{
		bRet = sal_True;
	}
	else
	{
		BitmapWriteAccess* pAcc = AcquireWriteAccess();

		if( pAcc )
		{
			BitmapColor		aCol;
			const long		nW = pAcc->Width();
			const long		nH = pAcc->Height();
			sal_uInt8*			cMapR = new sal_uInt8[ 256 ];
			sal_uInt8*			cMapG = new sal_uInt8[ 256 ];
			sal_uInt8*			cMapB = new sal_uInt8[ 256 ];
			long			nX, nY;
			double			fM, fROff, fGOff, fBOff, fOff;

			// calculate slope
			if( nContrastPercent >= 0 )
				fM = 128.0 / ( 128.0 - 1.27 * MinMax( nContrastPercent, 0L, 100L ) );
			else
				fM = ( 128.0 + 1.27 * MinMax( nContrastPercent, -100L, 0L ) ) / 128.0;

			// total offset = luminance offset + contrast offset
			fOff = MinMax( nLuminancePercent, -100L, 100L ) * 2.55 + 128.0 - fM * 128.0;

			// channel offset = channel offset + total offset
			fROff = nChannelRPercent * 2.55 + fOff;
			fGOff = nChannelGPercent * 2.55 + fOff;
			fBOff = nChannelBPercent * 2.55 + fOff;

			// calculate gamma value
			fGamma = ( fGamma <= 0.0 || fGamma > 10.0 ) ? 1.0 : ( 1.0 / fGamma );
			const sal_Bool bGamma = ( fGamma != 1.0 );

			// create mapping table
			for( nX = 0L; nX < 256L; nX++ )
			{
				cMapR[ nX ] = (sal_uInt8) MinMax( FRound( nX * fM + fROff ), 0L, 255L );
				cMapG[ nX ] = (sal_uInt8) MinMax( FRound( nX * fM + fGOff ), 0L, 255L );
				cMapB[ nX ] = (sal_uInt8) MinMax( FRound( nX * fM + fBOff ), 0L, 255L );

				if( bGamma )
				{
					cMapR[ nX ] = GAMMA( cMapR[ nX ], fGamma );
					cMapG[ nX ] = GAMMA( cMapG[ nX ], fGamma );
					cMapB[ nX ] = GAMMA( cMapB[ nX ], fGamma );
				}

				if( bInvert )
				{
					cMapR[ nX ] = ~cMapR[ nX ];
					cMapG[ nX ] = ~cMapG[ nX ];
					cMapB[ nX ] = ~cMapB[ nX ];
				}
			}

			// do modifying
			if( pAcc->HasPalette() )
			{
				BitmapColor aNewCol;

				for( sal_uInt16 i = 0, nCount = pAcc->GetPaletteEntryCount(); i < nCount; i++ )
				{
					const BitmapColor& rCol = pAcc->GetPaletteColor( i );
					aNewCol.SetRed( cMapR[ rCol.GetRed() ] );
					aNewCol.SetGreen( cMapG[ rCol.GetGreen() ] );
					aNewCol.SetBlue( cMapB[ rCol.GetBlue() ] );
					pAcc->SetPaletteColor( i, aNewCol );
				}
			}
			else if( pAcc->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_BGR )
			{
				for( nY = 0L; nY < nH; nY++ )
				{
					Scanline pScan = pAcc->GetScanline( nY );

					for( nX = 0L; nX < nW; nX++ )
					{
						*pScan = cMapB[ *pScan ]; pScan++;
						*pScan = cMapG[ *pScan ]; pScan++;
						*pScan = cMapR[ *pScan ]; pScan++;
					}
				}
			}
			else if( pAcc->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_RGB )
			{
				for( nY = 0L; nY < nH; nY++ )
				{
					Scanline pScan = pAcc->GetScanline( nY );

					for( nX = 0L; nX < nW; nX++ )
					{
						*pScan = cMapR[ *pScan ]; pScan++;
						*pScan = cMapG[ *pScan ]; pScan++;
						*pScan = cMapB[ *pScan ]; pScan++;
					}
				}
			}
			else
			{
				for( nY = 0L; nY < nH; nY++ )
				{
					for( nX = 0L; nX < nW; nX++ )
					{
						aCol = pAcc->GetPixel( nY, nX );
						aCol.SetRed( cMapR[ aCol.GetRed() ] );
						aCol.SetGreen( cMapG[ aCol.GetGreen() ] );
						aCol.SetBlue( cMapB[ aCol.GetBlue() ] );
						pAcc->SetPixel( nY, nX, aCol );
					}
				}
			}

			delete[] cMapR;
			delete[] cMapG;
			delete[] cMapB;
			ReleaseAccess( pAcc );
			bRet = sal_True;
		}
	}

	return bRet;
}

/* vim: set noet sw=4 ts=4: */
