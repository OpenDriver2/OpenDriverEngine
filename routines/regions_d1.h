#ifndef REGION_D1
#define REGION_D1
#include "regions.h"

//----------------------------------------------------------------------------------
// DRIVER 1
//----------------------------------------------------------------------------------

class CDriver1LevelRegion;
class CDriver1LevelMap;


struct CELL_ITERATOR_D1
{
	CDriver1LevelRegion*	region;
	CELL_DATA_D1*			pcd;
	CELL_OBJECT*			pco;
};


// Driver 1 region
class CDriver1LevelRegion : public CBaseLevelRegion
{
	friend class CDriver1LevelMap;
public:
	void					FreeAll() override;
	void					LoadRegionData(const SPOOL_CONTEXT& ctx) override;

	// cell iterator
	CELL_OBJECT*			StartIterator(CELL_ITERATOR_D1* iterator, int cellNumber) const;

protected:
	CELL_DATA_D1*			m_cells{ nullptr };				// cell data that holding information about cell pointers. 3D world seeks cells first here
};


// Driver 1 level map
class CDriver1LevelMap : public CBaseLevelMap
{
	friend class CDriver1LevelRegion;
public:
	void					FreeAll() override;

	//----------------------------------------

	void 					LoadMapLump(IVirtualStream* pFile) override;
	void					LoadSpoolInfoLump(IVirtualStream* pFile) override;

	void					SpoolRegion(const SPOOL_CONTEXT& ctx, const XZPAIR& cell) override;
	void					SpoolRegion(const SPOOL_CONTEXT& ctx, int regionIdx) override;

	CBaseLevelRegion*		GetRegion(const XZPAIR& cell) const override;
	CBaseLevelRegion*		GetRegion(int regionIdx) const override;

	int						MapHeight(const VECTOR_NOPAD& position) const override;

	//----------------------------------------
	// cell iterator
	CELL_OBJECT*			GetFirstCop(CELL_ITERATOR_D1* iterator, const XZPAIR& cell) const;
	CELL_OBJECT*			GetNextCop(CELL_ITERATOR_D1* iterator) const;

protected:

	// Driver 2 - specific
	CDriver1LevelRegion*	m_regions{ nullptr };					// map of regions
};


#endif