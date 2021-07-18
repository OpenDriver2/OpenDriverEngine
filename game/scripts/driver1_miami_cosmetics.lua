

local skylark = {
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
		fix.SVECTOR(120,-30,217),
		fix.SVECTOR(120,-30,-181),
		fix.SVECTOR(-120,-30,217),
		fix.SVECTOR(-120,-30,-181),
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
	cog = fix.SVECTOR(0,-65,-5),
	twistRateX = 224,
	twistRateY = 224,
	twistRateZ = 1120,
	mass = 4096,
}

local function ScaleUp(carCos)

	for i=1,12 do
		carCos.cPoints[i].vx = carCos.cPoints[i].vx
		carCos.cPoints[i].vy = carCos.cPoints[i].vy
		carCos.cPoints[i].vz = carCos.cPoints[i].vz
	end

	for i=1,4 do
		carCos.wheelDisp[i].vx = carCos.wheelDisp[i].vx * 1.5
		carCos.wheelDisp[i].vy = carCos.wheelDisp[i].vy * 1.5
		carCos.wheelDisp[i].vz = carCos.wheelDisp[i].vz * 1.5
	end
	
	carCos.colBox.vx = carCos.colBox.vx * 1.5
	carCos.colBox.vy = carCos.colBox.vy * 1.5
	carCos.colBox.vz = carCos.colBox.vz * 1.5
	
	carCos.cog.vx = carCos.cog.vx * 1.5
	carCos.cog.vy = carCos.cog.vy * 1.5
	carCos.cog.vz = carCos.cog.vz * 1.5
	
	carCos.twistRateX = carCos.twistRateX * 0.85
	carCos.twistRateY = carCos.twistRateY * 0.85
	carCos.twistRateZ = carCos.twistRateZ * 0.85
	
	carCos.powerRatio = carCos.powerRatio * 1.5
end

ScaleUp(skylark)

return {
	[1] = skylark,
}