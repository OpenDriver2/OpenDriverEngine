#pragma once

#include "models.h"
#include "level.h"

//------------------------------------------------------------------------------------------------------------

class IVirtualStream;
class CBaseLevelRegion;
class CBaseLevelMap;

using OnRegionLoaded_t = void (*)(CBaseLevelRegion* region);
using OnRegionFreed_t = void (*)(CBaseLevelRegion* region);

//----------------------------------------------------------------------------------

struct SPOOL_CONTEXT
{
	IVirtualStream*			dataStream;
	OUT_CITYLUMP_INFO*		lumpInfo;
};

struct CELL_ITERATOR_CACHE
{
	ubyte computedValues[2048] = { 0 };
};

class CBaseLevelRegion
{
	friend class CBaseLevelMap;
	friend class CDriver1LevelMap;
	friend class CDriver2LevelMap;
public:
	CBaseLevelRegion();
	virtual ~CBaseLevelRegion();

	virtual void			FreeAll();
	virtual void			LoadRegionData(const SPOOL_CONTEXT& ctx) = 0;
	void					LoadAreaData(const SPOOL_CONTEXT& ctx);

	bool					IsEmpty() const;
	int						GetNumber() const;

	CELL_OBJECT*			GetCellObject(int num) const;

protected:
	static int				UnpackCellPointers(ushort* dest_ptrs, char* src_data, int cell_slots_add, int targetRegion = 0);
	
	CBaseLevelMap*			m_owner;

	Spool*					m_spoolInfo{ nullptr };
	
	ushort*					m_cellPointers{ nullptr };		// cell pointers - pointing to CELL_DATA
	CELL_OBJECT*			m_cellObjects{ nullptr };		// cell objects that represents objects placed in the world

	int						m_regionX{ -1 };
	int						m_regionZ{ -1 };
	int						m_regionNumber{ -1 };
	int						m_regionBarrelNumber{ -1 };		// required for cell iterator slots
	bool					m_loaded{ false };
};

//----------------------------------------------------------------------------------

class CBaseLevelMap
{
	friend class CBaseLevelRegion;
	friend class CDriver1LevelRegion;
	friend class CDriver2LevelRegion;
public:
	CBaseLevelMap() = default;
	virtual ~CBaseLevelMap();

	void						Init(CDriverLevelModels* models, CDriverLevelTextures* textures);
	virtual void				FreeAll();

	//----------------------------------------

	void						SetLoadingCallbacks(OnRegionLoaded_t onLoaded, OnRegionFreed_t onFreed);

	void						SetFormat(ELevelFormat format);
	ELevelFormat				GetFormat() const;
	
	//----------------------------------------

	virtual void				LoadMapLump(IVirtualStream* pFile);
	virtual void				LoadSpoolInfoLump(IVirtualStream* pFile);

	virtual int					GetAreaDataCount() const;
	virtual void				LoadInAreaTPages(const SPOOL_CONTEXT& ctx, int areaDataNum) const;
	virtual void				LoadInAreaModels(const SPOOL_CONTEXT& ctx, int areaDataNum) const;

	virtual bool				SpoolRegion(const SPOOL_CONTEXT& ctx, const XZPAIR& cell) = 0;
	virtual bool				SpoolRegion(const SPOOL_CONTEXT& ctx, int regionIdx) = 0;

	int							GetRegionIndex(const XZPAIR& cell) const;

	bool						IsRegionSpooled(const XZPAIR& cell) const;
	bool						IsRegionSpooled(int index) const;

	virtual CBaseLevelRegion*	GetRegion(const XZPAIR& cell) const = 0;
	virtual CBaseLevelRegion*	GetRegion(int regionIdx) const = 0;

	virtual void				FindSurface(const VECTOR_NOPAD& position, VECTOR_NOPAD& outPoint, sdPlane& outPlane) const = 0;
	virtual int					GetSurfaceIndex(const VECTOR_NOPAD& position) const = 0;

	// converters
	void						WorldPositionToCellXZ(XZPAIR& cell, const VECTOR_NOPAD& position, const XZPAIR& offset = {0}) const;

	const OUT_CELL_FILE_HEADER&	GetMapInfo() const;
	
	int							GetCellsAcross() const;
	int							GetCellsDown() const;

	int							GetRegionsAcross() const;
	int							GetRegionsDown() const;

protected:

	void						InitRegion(CBaseLevelRegion* region, int index) const;

	void						OnRegionLoaded(CBaseLevelRegion* region);
	void						OnRegionFreed(CBaseLevelRegion* region);

	// shared
	OUT_CELL_FILE_HEADER		m_mapInfo;

	ELevelFormat				m_format;

	CDriverLevelTextures*		m_textures{ nullptr };
	CDriverLevelModels*			m_models{ nullptr };
	
	Spool*						m_regionSpoolInfo{ nullptr };			// region data info
	ushort*						m_regionSpoolInfoOffsets{ nullptr };	// region offset table
	
	AreaDataStr*				m_areaData{ nullptr };					// region model/texture data descriptors
	AreaTpageList*				m_areaTPages{ nullptr };				// region texpage usage table
	bool*						m_areaDataStates{ nullptr };			// area data loading states

	int							m_numStraddlers{ 0 };
	
	int							m_cell_slots_add[5] { 0 };
	int							m_cell_objects_add[5] { 0 };
	int							m_PVS_size[4] { 0 };
	
	int							m_numAreas{ 0 };
	int							m_numSpoolInfoOffsets{ 0 };
	int							m_numRegionSpools{ 0 };

	int							m_regions_across{ 0 };
	int							m_regions_down{ 0 };

	CELL_OBJECT*				m_straddlers{ nullptr };

	OnRegionLoaded_t			m_onRegionLoaded{ nullptr };
	OnRegionFreed_t				m_onRegionFreed{ nullptr };
};
