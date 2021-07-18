

local toronado = {
	headLight = fix.SVECTOR(93,14,-351),
	frontInd = fix.SVECTOR(110,16,-340),
	backInd = fix.SVECTOR(55,32,368),
	brakeLight = fix.SVECTOR(68,33,368),
	revLight = fix.SVECTOR(57,35,359),
	policeLight = fix.SVECTOR(34,0,0),
	exhaust = fix.SVECTOR(68,68,359),
	smoke = fix.SVECTOR(0,-4,-335),
	fire = fix.SVECTOR(0,-4,-341),
	wheelDisp = {
		fix.SVECTOR(128,-30,217),
		fix.SVECTOR(128,-30,-181),
		fix.SVECTOR(-129,-30,217),
		fix.SVECTOR(-129,-30,-181),
	},
	extraInfo = -31720, -- FLAGS
	powerRatio = 4096,
	cbYoffset = 0,
	susCoeff = 4096,
	traction = 4096,
	wheelSize = 52,
	cPoints = {
		fix.SVECTOR(-126, 11, -370),
		fix.SVECTOR(125, 11, -370),
		fix.SVECTOR(-126, 2, 370),
		fix.SVECTOR(125, 2, 370),
		fix.SVECTOR(-126, 89, -367),
		fix.SVECTOR(125, 89, -367),
		fix.SVECTOR(-83, 163, -124),
		fix.SVECTOR(82, 163, -124),
		fix.SVECTOR(-83, 164, 52),
		fix.SVECTOR(82, 164, 52),
		fix.SVECTOR(-126, 91, 371),
		fix.SVECTOR(125, 91, 371),
	},
	colBox = fix.SVECTOR(129,84,375),
	cog = fix.SVECTOR(0,-85,11),
	twistRateX = 224,
	twistRateY = 224,
	twistRateZ = 1120,
	mass = 4096,
}

return {
	[1] = toronado,
}