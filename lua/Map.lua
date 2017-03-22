C_InstallModule("map")

#const MOVE_RIGHT 1

local Object = require("Object")

local MapPointer = Object:extend()

function MapPointer:initialize(_ptr)
	self._ptr = _ptr
end

function MapPointer:get(dir)
	return C_MapPointer_get(self._ptr, dir)
end

function MapPointer:move(dir)
	C_MapPointer_move(self._ptr, dir)
end

local Map = Object:extend()

function Map:initialize(_ptr)
	self.x = 0
	self.y = 0
	if _ptr == nil then
		self._ptr = C_Map_New()
	else
		self._ptr = _ptr
	end
end

function Map:addID(x, y, id)
	return C_Map_addID(self._ptr, x, y, id)
end

function Map:getIDs(x, y, id)
	return C_Map_getIDs(self._ptr, x, y)
end

function Map:delID(x, y, id)
	C_Map_delID(self._ptr, x, y, id)
end

function Map:set(x, y, id)
	C_Map_set(self._ptr, x, y, id)
end

function Map:setFlags(x, y, flags)
	C_Map_setFlags(self._ptr, x, y, flags)
end


function Map:get(x, y)
	return C_Map_get(self._ptr, x, y)
end

function Map:setIfEmpty(x, y, id)
	C_Map_setIfEmpty(self._ptr, x, y, id)
end

function Map:setIfEmptyAndId(x, y, id, id2)
	C_Map_setIfEmptyAndId(self._ptr, x, y, id, id2)
end


function Map:dump()
	C_Map_dump(self._ptr)
end

local dot9 = {
	{ 0, 0, 0, 1, 1, 1, 0, 0, 0 },
	{ 0, 0, 1, 1, 1, 1, 1, 0, 0 },
	{ 0, 1, 1, 1, 1, 1, 1, 1, 0 },
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 1, 1, 1, 1, 1, 1, 1, 0 },
	{ 0, 0, 1, 1, 1, 1, 1, 0, 0 },
	{ 0, 0, 0, 1, 1, 1, 0, 0, 0 }
}

local dot7 = {
	{ 0, 0, 1, 1, 1, 0, 0 },
	{ 0, 1, 1, 1, 1, 1, 0 },
	{ 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 1, 1, 1, 1, 1, 0 },
	{ 0, 0, 1, 1, 1, 0, 0 }
}

local dot5 = {
	{ 0, 1, 1, 1, 0 },
	{ 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1 },
	{ 0, 1, 1, 1, 0 },
}

local dot4 = {
	{ 0, 1, 1, 0 },
	{ 1, 1, 1, 1 },
	{ 1, 1, 1, 1 },
	{ 0, 1, 1, 0 }
}

function Map:dot(x, y, s, id)

	local dot = nil
	if s == 4 then
		s = 4
		x = x - 3
		y = y - 3
		dot = dot4
	elseif s == 5 or s == 6 then
		s = 5
		x = x - 3
		y = y - 3
		dot = dot5
	elseif s == 7 or s == 8 then
		s = 7
		x = x - 4
		y = y - 4
		dot = dot7
	elseif s == 9 then
		x = x - 5
		y = y - 5
		dot = dot9
	end

	if dot == nil then return end

	for i = 1, s do
		for j = 1, s do
			if dot[i][j] == 1 then
				self:set(x + i, y + j, id)
			end
		end
	end

end


function Map:makeGround(x, y, id, ifEmpty, id2)

	x = x - 5
	y = y - 5

	for i = 1, 9 do
		for j = 1, 9 do
			if dot9[i][j] == 1 then
				if ifEmpty then
					self:setIfEmpty(x + i, y + j, id)
				else
					self:setIfEmptyAndId(x + i, y + j, id, id2)
				end
			end
		end
	end

end

--[[

Окружность

x = R * cos t
y = R * sin t

Сжатие (черный график)

x_1 = x / n 
y_1 = y / m

Поворот (красный)

x_2 = x_1 * cos alpha  - y_1 * sin alpha
y_2 = x_1 * sin alpha  + y_1 * cos alpha

]]


local circle = math.pi + math.pi
local circleOfsset = circle / 2 + circle / 4
local circle_2 = circle / 2
local circle_4 = circle / 4
local circle_8 = circle / 8
local circle_16 = circle / 16

function Map:makeCircle(x, y, m, n, r, id, ifEmpty, id2)

	local steps = r * 6
	local step = circle / steps / 2
	local axis = 0
	local y1 = nil
	local func

	if type(id) == "function" then
		func = id
	else
		if ifEmpty then
			func = function(map, x, y)
				map:setIfEmpty(x, y, id)
			end
		else
			func = function(map, x, y)
				map:setIfEmptyAndId(x, y, id, id2)
			end
		end
	end

	for i = 1, steps do

		axis = axis + step
		local cy = math.floor(r * math.sin(circleOfsset + axis) / n)
		if cy ~= y1 then
			y1 = cy

			-- right pixel

			local x1 = math.floor(r * math.cos(circleOfsset + axis) / m)
			local x2 = math.floor(r * math.cos(circleOfsset - axis) / m)
			if x1 > x2 then
				local t = x1
				x1 = x2
				x2 = t
			end

			local y2 = y1 + y
			for j = x1 + x, x2 + x do

				func(self, j, y2)

			end
		end

	end

end


function Map:makeFigure(r, x, y, id, opt)

	local branches = { }
	local startAxis

	if opt.branchStartAxis == -1 then
		startAxis = circle / 10000 * r:get(10000)
	else
		startAxis = opt.branchStartAxis + circleOfsset
	end

	for i = 1, opt.branches do
		local axis = startAxis
		if opt.branchStartDeviation ~= 0 then
			local j = r:get(1000)
			if j % 2 == 0 then j = j * -1 end
			local o = opt.branchStartDeviation / 1000 * j
			-- lprint("j " .. j .. " o " .. o)
			axis = axis + o
		end
		branches[i] = {
			segments = 0,
			axis = axis,
			x = x,
			y = y,
			dot = opt.dot
		}
		startAxis = startAxis + opt.branchStartAxisStep
	end

	--[[
	dump(branches)
	dump(opt)
	]]

	for s = 1, opt.branchSegments do

		for i = 1, opt.branches do
			local b = branches[i]
			for d = 1, opt.branchSegmentDotCount do
				b.x = b.x + math.cos(b.axis)
				b.y = b.y + math.sin(b.axis)
				-- lprint("dot " .. b.x .. "x" .. b.y)
				local x = math.floor(b.x)
				local y = math.floor(b.y)
				self:dot(x, y, b.dot, id)
				if opt.afterPlaceDotCB ~= nil then
					opt.afterPlaceDotCB(x, y, opt.ctx)
				end

				-- change dot size
				if opt.dotChange ~= nil then
					local change = r:get(100)
					if change < opt.dotChange then
						if b.dot == opt.dotMin then
							b.dot = b.dot + 1
						elseif b.dot == opt.dotMax then
							b.dot = b.dot - 1
						else
							change = r:get(100)
							if change < 50 then
								b.dot = b.dot + 1
							else
								b.dot = b.dot - 1
							end
						end
					end
				end
			end

			if opt.segmentCB ~= nil then
				opt.segmentCB(b.x, b.y, opt.ctx)
			end

			-- split or change direction

			-- change direction

			b.segments = b.segments + 1

			if opt.branchDividing ~= nil and b.segments == opt.branchDividing then
				b.segments = 0

				local axis = b.axis - opt.branchDivideDeviation / 2
				local d = opt.branchDivideDeviation / opt.branchDivideCount
				local bi = { i }
				for j = 1, opt.branchDivideCount do
					local ni = opt.branches + 1
					opt.branches = ni
					axis = axis + d
					local nb = {
						segments = 0,
						axis = axis,
						x = b.x,
						y = b.y,
						dot = opt.dot
					}
					branches[ni] = nb
				end

				for ind, v in pairs(bi) do
					local b = branches[v]

					local j = r:get(1000)
					if j % 2 == 0 then j = j * -1 end
					local o = opt.branchSegmentDeviation / 1000 * j
					-- lprint("o " .. o)
					b.axis = b.axis + o
				end

			else
				local j = r:get(1000)
				if j % 2 == 0 then j = j * -1 end
				local o = opt.branchSegmentDeviation / 1000 * j
				-- lprint("o " .. o)
				b.axis = b.axis + o
			end


		end



	end

end

function Map:makeGrass(r, x, y, id)
	-- local a = r:get(4) + 1
	local ctx = { x1 = x, y1 = y, x2 = x, y2 = y, x = x, y = y }

	self:makeFigure(r, x, y, id, {
		branches = 1,
		branchStartAxis = 0,
		branchStartAxisStep = circle,
		branchStartDeviation = circle,
		dot = 5,
		dotMin = 5,
		dotMax = 9,
		dotChange = 60,
		branchSegments = 6 + math.floor(math.pow(r:get(19), 3) / 400),
		branchSegmentDeviation = circle_8,
		branchSegmentLength = 4,
		branchSegmentDotCount = 2,
		ctx = ctx,
		afterPlaceDotCB = function(x, y, ctx)
			--[[
			self:makeGround(x - 4, y, 2)
			self:makeGround(x + 4, y, 2)
			self:makeGround(x, y - 4, 2)
			self:makeGround(x, y + 4, 2)
			]]

			-- self:makeCircle(x, y, 20, 2)

			if x - 5 < ctx.x1 then ctx.x1 = x - 5 end
			if x + 5 > ctx.x2 then ctx.x2 = x + 5 end
			if y - 5 < ctx.y1 then ctx.y1 = y - 5 end
			if y + 5 > ctx.y2 then ctx.y2 = y + 5 end

		end
	})

	ctx.x1 = ctx.x1 - 5
	ctx.x2 = ctx.x2 + 5
	ctx.y1 = ctx.y1 - 5
	ctx.y2 = ctx.y2 + 5
	local w = (ctx.x2 - ctx.x1) / 2
	local h = (ctx.y2 - ctx.y1) / 2
	local r, m, n
	if w > h then
		r = w
		m = 1
		n = w / h
	else
		r = h
		m = h / w
		n = 1
	end


	-- dump(ctx)
	-- lprint("r " .. math.floor(r) .. " m " .. math.floor(m) .. " n " .. math.floor(n))

	self:makeCircle(ctx.x1 + w, ctx.y1 + h, m, n, r * 1.3, 2)

end

function Map:makeRiver(r, x, y, id)

	self:makeFigure(r, x, y, id, {
		branches = 2,
		branchStartAxis = -1,
		branchStartAxisStep = circle_2,
		branchStartDeviation = 0,
		dot = 5,
		dotMin = 5,
		dotMax = 9,
		dotChange = 60,
		branchDividing = 7 + r:get(3),
		branchDivideCount = 1,
		branchDivideDeviation = circle_2,
		branchSegments = 9 + r:get(13),
		branchSegmentDeviation = circle_8,
		branchSegmentLength = 4,
		branchSegmentDotCount = 5,
		afterPlaceDotCB = function(x, y)
			self:makeGround(x - 4, y, 2, true)
			self:makeGround(x + 4, y, 2, true)
			self:makeGround(x, y - 4, 2, true)
			self:makeGround(x, y + 4, 2, true)
		end
	})

end

function Map:makeMountain(r, x, y, id)

	self:makeFigure(r, x, y, id, {
		branches = 4,
		branchStartAxis = -1,
		branchStartAxisStep = circle_4,
		branchStartDeviation = 0,
		dot = 5,
		--[[
		dotMin = 5,
		dotMax = 7,
		dotChange = 60,
		]]
		branchDividing = 4,
		branchDivideCount = 1,
		branchDivideDeviation = circle_4,
		branchSegments = 9 + r:get(11),
		branchSegmentDeviation = circle_16,
		branchSegmentLength = 1,
		branchSegmentDotCount = 2,
		segmentCB = function(x, y)
			self:makeCircle(x, y, 1, 1, 15, 2, false, 1)
		end
	})

end

function Map:getPointer(x, y)
	return MapPointer:new(C_Map_getPointer(self._ptr, x, y))	
end


return Map
 