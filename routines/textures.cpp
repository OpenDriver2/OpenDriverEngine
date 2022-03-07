#include "core/cmdlib.h"
#include "core/IVirtualStream.h"
#include "math/Vector.h"
#include "util/rnc2.h"

#include <string.h>

#include <nstd/String.hpp>
#include <nstd/Math.hpp>

#include "textures.h"
#include "level.h"



//-------------------------------------------------------------------------------

// unpacks texture, returns new source pointer
// there is something like RLE used
char* unpackTexture(char* src, char* dest)
{
	// start from the end
	char* ptr = dest + TEXPAGE_4BIT_SIZE - 1;

	do {
		char pix = *src++;

		if ((pix & 0x80) != 0)
		{
			char p = *src++;

			do (*ptr-- = p);
			while (pix++ <= 0);
		}
		else
		{
			do (*ptr-- = *src++);
			while (pix-- != 0);
		}
	} while (ptr >= dest);

	return src;
}

//-------------------------------------------------------------------------------

#define MAX_PAGE_CLUTS 63

struct SpooledTextureData_t
{
	int			numPalettes;
	TEXCLUT		palettes[63];
	ubyte		compTable[28];		// padding for 512-B alignment if uncompressed
	ubyte		texels[128 * 256];	// 256×256 four-bit indices
};

CTexturePage::CTexturePage()
{
	memset(&m_bitmap, 0, sizeof(m_bitmap));
}

CTexturePage::~CTexturePage()
{
	delete[] m_details;
	FreeBitmap();
}

// free texture page data and bitmap
// used for spooled
void CTexturePage::FreeBitmap()
{
	m_owner->OnTexturePageFreed(this);
	
	delete[] m_bitmap.data;
	delete[] m_bitmap.clut;

	m_bitmap.data = nullptr;
	m_bitmap.clut = nullptr;
}

//-------------------------------------------------------------
// Conversion of indexed palettized texture to 32bit RGBA
//-------------------------------------------------------------
void CTexturePage::ConvertIndexedTextureToRGBA(uint* dest_color_data, int detail, TEXCLUT* clut, bool outputBGR, bool originalTransparencyKey)
{
	TexBitmap_t& bitmap = m_bitmap;

	if (!(detail < m_numDetails))
	{
		MsgError("Cannot apply palette to non-existent detail! Programmer error?\n");
		return;
	}

	if (clut == nullptr)
		clut = &bitmap.clut[detail];

	const TEXINF& texInfo = m_details[detail].info;

	const int ox = texInfo.x;
	const int oy = texInfo.y;
	const int w = texInfo.width ? texInfo.width : TEXPAGE_SIZE_Y;	// 0 means full size
	const int h = texInfo.height ? texInfo.height : TEXPAGE_SIZE_Y;

	const int tp_wx = ox + w;
	const int tp_hy = oy + h;

	for (int y = oy; y < tp_hy; y++)
	{
		for (int x = ox; x < tp_wx; x++)
		{
			ubyte clindex = bitmap.data[y * TEXPAGE_SIZE_X + x / 2];

			if (0 != (x & 1))
				clindex >>= 4;

			clindex &= 15;

			// flip texture by Y
			const int ypos = (TEXPAGE_SIZE_Y - y - 1) * TEXPAGE_SIZE_Y;

			if(outputBGR)
			{
				TVec4D<ubyte> color = rgb5a1_ToBGRA8(clut->colors[clindex], originalTransparencyKey);
				dest_color_data[ypos + x] = *(uint*)(&color);
			}
			else
			{
				TVec4D<ubyte> color = rgb5a1_ToRGBA8(clut->colors[clindex], originalTransparencyKey);
				dest_color_data[ypos + x] = *(uint*)(&color);
			}

		}
	}
}

void CTexturePage::InitFromFile(int id, TEXPAGE_POS& tp, IVirtualStream* pFile)
{
	m_id = id;
	m_tp = tp;

	pFile->Read(&m_numDetails, 1, sizeof(int));

	// don't load empty tpage details
	if (m_numDetails)
	{
		// read texture detail info
		m_details = new TexDetailInfo_t[m_numDetails];

		for(int i = 0; i < m_numDetails; i++)
		{
			m_details[i].pageNum = m_id;
			m_details[i].detailNum = i;
			m_details[i].numExtraCLUTs = 0;
			memset(m_details[i].extraCLUTs, 0, sizeof(m_details[i].extraCLUTs));
			
			pFile->Read(&m_details[i].info, 1, sizeof(TEXINF));
		}
	}
	else
		m_details = nullptr;
}


void CTexturePage::LoadCompressedTexture(IVirtualStream* pFile)
{
	pFile->Read( &m_bitmap.numPalettes, 1, sizeof(int) );

	// allocate palettes
	m_bitmap.clut = new TEXCLUT[m_bitmap.numPalettes];

	for(int i = 0; i < m_bitmap.numPalettes; i++)
	{
		// read 16 palettes
		pFile->Read(&m_bitmap.clut[i].colors, 16, sizeof(ushort));
	}

	int imageStart = pFile->Tell();

	// read compression data
	ubyte* compressedData = new ubyte[TEXPAGE_4BIT_SIZE];
	pFile->Read(compressedData, 1, TEXPAGE_4BIT_SIZE);

	char* unpackEnd = unpackTexture((char*)compressedData, (char*)m_bitmap.data);

	delete[] compressedData;
	
	// unpack
	m_bitmap.rsize = (char*)unpackEnd - (char*)compressedData;

	// seek to the right position
	// this is necessary because it's aligned to CD block size
	pFile->Seek(imageStart + m_bitmap.rsize, VS_SEEK_SET);
}

//-------------------------------------------------------------------------------
// Loads Texture page itself with it's color lookup tables
//-------------------------------------------------------------------------------
bool CTexturePage::LoadTPageAndCluts(IVirtualStream* pFile, bool isSpooled)
{
	int rStart = pFile->Tell();

	if(m_bitmap.data)
	{
		// skip already loaded data
		pFile->Seek(m_bitmap.rsize, VS_SEEK_CUR);
		return true;
	}

	m_bitmap.data = new ubyte[TEXPAGE_4BIT_SIZE];

	if( isSpooled )
	{
		// non-compressed textures loads different way, with a fixed size
		SpooledTextureData_t* texData = new SpooledTextureData_t;
		pFile->Read( texData, 1, sizeof(SpooledTextureData_t) );

		// palettes are after them
		m_bitmap.numPalettes = texData->numPalettes;

		m_bitmap.clut = new TEXCLUT[m_bitmap.numPalettes];
		memcpy(m_bitmap.clut, texData->palettes, sizeof(TEXCLUT)* m_bitmap.numPalettes);

		memcpy(m_bitmap.data, texData->texels, TEXPAGE_4BIT_SIZE);

		// not need anymore
		delete texData;
	}
	else
	{
		// non-spooled are compressed
		LoadCompressedTexture(pFile);
	}

	m_bitmap.rsize = pFile->Tell() - rStart;
	DevMsg(SPEW_NORM, "PAGE %d (%s) datasize=%d\n", m_id, isSpooled ? "spooled" : "compressed", m_bitmap.rsize);

	m_owner->OnTexturePageLoaded(this);
	
	return true;
}

//-------------------------------------------------------------------------------
// searches for detail in this TPAGE
//-------------------------------------------------------------------------------
TexDetailInfo_t* CTexturePage::FindTextureDetail(const char* name) const
{
	for (int i = 0; i < m_numDetails; i++)
	{
		const char* pTexName = m_owner->GetTextureDetailName(&m_details[i].info);

		if (!strcmp(pTexName, name)) // FIXME: hashing and case insensitive?
			return &m_details[i];
	}

	return nullptr;
}

TexDetailInfo_t* CTexturePage::GetTextureDetail(int num) const
{
	return &m_details[num];
}

int CTexturePage::GetDetailCount() const
{
	return m_numDetails;
}

const TexBitmap_t& CTexturePage::GetBitmap() const
{
	return m_bitmap;
}

int CTexturePage::GetId() const
{
	return m_id;
}

int CTexturePage::GetFlags() const
{
	return m_tp.flags;
}

//-------------------------------------------------------------------------------

CDriverLevelTextures::CDriverLevelTextures()
{
}

CDriverLevelTextures::~CDriverLevelTextures()
{
}

//
// loads global textures (pre-loading stage)
//
void CDriverLevelTextures::LoadPermanentTPages(IVirtualStream* pFile)
{
	DevMsg(SPEW_NORM,"Loading permanent texture pages (%d)\n", m_numPermanentPages);

	// simulate sectors
	// convert current file offset to sectors
	long sector = pFile->Tell() / SPOOL_CD_BLOCK_SIZE;
	int nsectors = 0;

	for (int i = 0; i < m_numPermanentPages; i++)
		nsectors += (m_permsList[i].y + SPOOL_CD_BLOCK_SIZE-1) / SPOOL_CD_BLOCK_SIZE;
	
	// load permanent pages
	for(int i = 0; i < m_numPermanentPages; i++)
	{
		long curOfs = pFile->Tell(); 
		int tpage = m_permsList[i].x;

		// permanents are also compressed
		m_texPages[tpage].LoadTPageAndCluts(pFile, false);

		pFile->Seek(curOfs + ((m_permsList[i].y + SPOOL_CD_BLOCK_SIZE-1) & -SPOOL_CD_BLOCK_SIZE), VS_SEEK_SET);
	}

	// simulate sectors
	sector += nsectors;
	pFile->Seek(sector * SPOOL_CD_BLOCK_SIZE, VS_SEEK_SET);

	// Driver 2 - special cars only
	// Driver 1 - only player cars
	DevMsg(SPEW_NORM, "Loading special/car texture pages (%d)\n", m_numSpecPages);

	// load compressed spec pages
	// those are non-spooled ones
	for (int i = 0; i < m_numSpecPages; i++)
	{
		long curOfs = pFile->Tell();
		int tpage = m_specList[i].x;

		// permanents are compressed
		m_texPages[tpage].LoadTPageAndCluts(pFile, false);

		pFile->Seek(curOfs + ((m_specList[i].y + SPOOL_CD_BLOCK_SIZE-1) & -SPOOL_CD_BLOCK_SIZE), VS_SEEK_SET);
	}
}

//-------------------------------------------------------------
// parses texture info lumps. Quite simple
//-------------------------------------------------------------
void CDriverLevelTextures::LoadTextureInfoLump(IVirtualStream* pFile)
{
	int numPages;
	pFile->Read(&numPages, 1, sizeof(int));

	int numTextures;
	pFile->Read(&numTextures, 1, sizeof(int));

	DevMsg(SPEW_NORM, "TPage count: %d\n", numPages);
	DevMsg(SPEW_NORM, "Texture amount: %d\n", numTextures);

	// read array of texutre page info
	TEXPAGE_POS* tpage_position = new TEXPAGE_POS[numPages + 1];
	pFile->Read(tpage_position, numPages+1, sizeof(TEXPAGE_POS));

	// read page details
	m_numTexPages = numPages;
	m_texPages = new CTexturePage[numPages];

	for(int i = 0; i < numPages; i++) 
	{
		CTexturePage& tp = m_texPages[i];
		tp.m_owner = this;

		tp.InitFromFile(i, tpage_position[i], pFile);
	}

	pFile->Read(&m_numPermanentPages, 1, sizeof(int));
	DevMsg(SPEW_NORM,"Permanent TPages = %d\n", m_numPermanentPages);
	pFile->Read(m_permsList, 16, sizeof(XYPAIR));

	// Driver 2 - special cars only
	// Driver 1 - only player cars
	pFile->Read(&m_numSpecPages, 1, sizeof(int));
	pFile->Read(m_specList, 16, sizeof(XYPAIR));

	DevMsg(SPEW_NORM,"Special/Car TPages = %d\n", m_numSpecPages);

	// not needed
	delete tpage_position;
}

//-------------------------------------------------------------
// load texture names, same as model names
//-------------------------------------------------------------
void CDriverLevelTextures::LoadTextureNamesLump(IVirtualStream* pFile, int size)
{
	m_textureNamesData = new char[size+1];
	memset(m_textureNamesData, 0, size + 1);

	pFile->Read(m_textureNamesData, size, 1);

	int len = strlen(m_textureNamesData);
	int sz = 0;

	do
	{
		char* str = m_textureNamesData + sz;

		len = strlen(str);

		sz += len + 1;
	} while (sz < size);
}

void CDriverLevelTextures::LoadOverlayMapLump(IVirtualStream* pFile, int lumpSize)
{
	m_overlayMapData = new char[lumpSize];
	pFile->Read(m_overlayMapData, 1, lumpSize);
}

//-------------------------------------------------------------
// Loads car and pedestrians palletes
//-------------------------------------------------------------
void CDriverLevelTextures::LoadPalletLump(IVirtualStream* pFile)
{
	ushort* clutTablePtr;
	int total_cluts;

	pFile->Read(&total_cluts, 1, sizeof(int));

	if (total_cluts == 0)
		return;

	m_extraPalettes = new ExtClutData_t[total_cluts + 1];
	memset(m_extraPalettes, 0, sizeof(ExtClutData_t) * total_cluts);

	DevMsg(SPEW_NORM, "total_cluts: %d\n", total_cluts);

	int added_cluts = 0;
	while (true)
	{
		PALLET_INFO info;

		if (m_format == LEV_FORMAT_DRIVER1)
		{
			PALLET_INFO_D1 infod1;
			pFile->Read(&infod1, 1, sizeof(info) - sizeof(int));
			
			info.clut_number = -1; // D1 doesn't have that
			info.tpage = infod1.tpage;
			info.texnum = infod1.texnum;
			info.palette = infod1.palette;
		}
		else
		{
			pFile->Read(&info, 1, sizeof(info));
		}

		if (info.palette == -1)
			break;

		if (info.clut_number == -1)
		{
			ExtClutData_t& data = m_extraPalettes[added_cluts];
			data.texnum[data.texcnt++] = info.texnum;
			data.tpage = info.tpage;
			data.palette = info.palette;

			clutTablePtr = data.clut.colors;

			pFile->Read(clutTablePtr, 16, sizeof(ushort));

			// reference
			if(info.texnum < m_texPages[data.tpage].GetDetailCount())
			{
				TexDetailInfo_t& detail = m_texPages[data.tpage].m_details[info.texnum];
				detail.extraCLUTs[data.palette] = &data.clut;
				detail.numExtraCLUTs = Math::max(detail.numExtraCLUTs, data.palette);
			}

			added_cluts++;

			// only in D1 we need to check count
			if (m_format == LEV_FORMAT_DRIVER1)
			{
				if (added_cluts >= total_cluts)
					break;
			}
		}
		else
		{
			ExtClutData_t& data = m_extraPalettes[info.clut_number];

			// add texture number to existing clut
			data.texnum[data.texcnt++] = info.texnum;

			// reference
			if (info.texnum < m_texPages[data.tpage].GetDetailCount())
			{
				TexDetailInfo_t& detail = m_texPages[data.tpage].m_details[info.texnum];
				detail.extraCLUTs[data.palette] = &data.clut;
				detail.numExtraCLUTs = Math::max(detail.numExtraCLUTs, data.palette);
			}
		}
	}

	DevMsg(SPEW_NORM,"    added: %d\n", added_cluts);
	m_numExtraPalettes = added_cluts;
}

//----------------------------------------------------------------------------------------------------

TexDetailInfo_t* CDriverLevelTextures::FindTextureDetail(const char* name) const
{
	for(int i = 0; i < m_numTexPages; i++)
	{
		TexDetailInfo_t* found = m_texPages[i].FindTextureDetail(name);

		if (found)
			return found;
	}

	return nullptr;
}

// returns texture name
const char* CDriverLevelTextures::GetTextureDetailName(TEXINF* info) const
{
	return m_textureNamesData + info->nameoffset;
}

// unpacks RNC2 overlay map segment into RGBA buffer (32x32)
void CDriverLevelTextures::GetOverlayMapSegmentRGBA(TVec4D<ubyte>* destination, int index) const
{
	// 8 bit texture so...
	char mapBuffer[16 * 32];

	int clut_offset;

	if (m_format >= LEV_FORMAT_DRIVER2_ALPHA16)
		clut_offset = 512;
	else
		clut_offset = 328;

	ushort* offsets = (ushort*)m_overlayMapData;
	ushort* clut = (ushort*)(m_overlayMapData + clut_offset);

	UnpackRNC(m_overlayMapData + offsets[index], mapBuffer);

	// convert to RGBA
	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			int px, py;

			ubyte colorIndex = (ubyte)mapBuffer[y * 16 + x / 2];

			if (0 != (x & 1))
				colorIndex >>= 4;

			colorIndex &= 0xf;

			destination[y * 32 + x] = rgb5a1_ToBGRA8(clut[colorIndex]);
		}
	}
}

// computes overlay map segment count
int	CDriverLevelTextures::GetOverlayMapSegmentCount() const
{
	ushort* offsets = (ushort*)m_overlayMapData;
	int numValid = 0;

	// max offset count for overlay map is 256, next is palette
	for (int i = 0; i < 256; i++)
	{
		char* rncData = m_overlayMapData + offsets[i];
		if (rncData[0] == 'R' && rncData[1] == 'N' && rncData[2] == 'C')
		{
			numValid++;
		}
	}

	return numValid;
}

// release all data
void CDriverLevelTextures::FreeAll()
{
	delete[] m_textureNamesData;
	delete[] m_texPages;
	delete[] m_extraPalettes;
	delete[] m_overlayMapData;

	m_textureNamesData = nullptr;
	m_texPages = nullptr;
	m_extraPalettes = nullptr;
	m_overlayMapData = nullptr;

	m_numTexPages = 0;
	m_numPermanentPages = 0;
	m_numSpecPages = 0;
	m_numExtraPalettes = 0;
}

// getters
CTexturePage* CDriverLevelTextures::GetTPage(int page) const
{
	if (page > 0 && page < m_numTexPages)
		return &m_texPages[page];
	return nullptr;
}

int CDriverLevelTextures::GetTPageCount() const
{
	return m_numTexPages;
}

void CDriverLevelTextures::SetLoadingCallbacks(OnTexturePageLoaded_t onLoaded, OnTexturePageFreed_t onFreed)
{
	m_onTPageLoaded = onLoaded;
	m_onTPageFreed = onFreed;
}

void CDriverLevelTextures::SetFormat(ELevelFormat format)
{
	m_format = format;
}

ELevelFormat CDriverLevelTextures::GetFormat() const
{
	return m_format;
}

void CDriverLevelTextures::OnTexturePageLoaded(CTexturePage* tp)
{
	if (m_onTPageLoaded)
		m_onTPageLoaded(tp);
}

void CDriverLevelTextures::OnTexturePageFreed(CTexturePage* tp)
{
	if (m_onTPageFreed)
		m_onTPageFreed(tp);
}