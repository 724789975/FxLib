-------------------------------------------------
---      *** SHA-1 algorithm for Lua ***      ---
-------------------------------------------------
--- Author:  Martin Huesser                   ---
--- Date:    2008-06-16                       ---
--- License: You may use this code in your    ---
---          projects as long as this header  ---
---          stays intact.                    ---
-------------------------------------------------

module("Sha1", package.seeall)

--local mod   = math.fmod
--local floor = math.floor
bit = {}

----------------------------------------

local function cap(x)
	return math.fmod(x,4294967296)
end

----------------------------------------

function bit.bnot(x)
	return 4294967295-cap(x)
end

----------------------------------------

function bit.lshift(x,n)
	return cap(cap(x)*2^n)
end

----------------------------------------

function bit.rshift(x,n)
	return math.floor(cap(x)/2^n)
end

----------------------------------------

function bit.band(x,y)
	local z,i,j = 0,1
	for j = 0,31 do
		if (math.fmod(x,2)==1 and math.fmod(y,2)==1) then
			z = z + i
		end
		x = bit.rshift(x,1)
		y = bit.rshift(y,1)
		i = i*2
	end
	return z
end

----------------------------------------

function bit.bor(x,y)
	local z,i,j = 0,1
	for j = 0,31 do
		if (math.fmod(x,2)==1 or math.fmod(y,2)==1) then
			z = z + i
		end
		x = bit.rshift(x,1)
		y = bit.rshift(y,1)
		i = i*2
	end
	return z
end

----------------------------------------

function bit.bxor(x,y)
	local z,i,j = 0,1
	for j = 0,31 do
		if (math.fmod(x,2)~=math.fmod(y,2)) then
			z = z + i
		end
		x = bit.rshift(x,1)
		y = bit.rshift(y,1)
		i = i*2
	end
	return z
end

--[[
local strlen  = string.len
local strchar = string.char
local strbyte = string.byte
local strsub  = string.sub
local floor   = math.floor
local bnot    = bit.bnot
local band    = bit.band
local bor     = bit.bor
local bxor    = bit.bxor
local shl     = bit.lshift
local shr     = bit.rshift
--]]

local string, math = string, math
local h0
local h1
local h2
local h3
local h4

-------------------------------------------------

local function LeftRotate(val, nr)
	return bit.lshift(val, nr) + bit.rshift(val, 32 - nr)
end

-------------------------------------------------

local function ToHex(num)
--[[
	local i, d
	local str = ""
	for i = 1, 8 do
		d = bit.band(num, 15)
		if (d < 10) then
			str = string.char(d + 48) .. str
		else
			str = string.char(d + 87) .. str
		end
		num = math.floor(num / 16)
	end
	return str
--]]
	local i, d
	local str = ""
	for i = 1, 4 do
		d = bit.band(num, 255)
		str = string.char(d) .. str
		num = math.floor(num / 256)
	end
	return str
end

-------------------------------------------------

local function PreProcess(str)
	local bitlen, i
	local str2 = ""
	bitlen = string.len(str) * 8
	str = str .. string.char(128)
	i = 56 - bit.band(string.len(str), 63)
	if (i < 0) then
		i = i + 64
	end
	for i = 1, i do
		str = str .. string.char(0)
	end
	for i = 1, 8 do
		str2 = string.char(bit.band(bitlen, 255)) .. str2
		bitlen = math.floor(bitlen / 256)
	end
	return str .. str2
end

-------------------------------------------------

local function MainLoop(str)
	local a, b, c, d, e, f, k, t
	local i, j
	local w = {}
	while (str ~= "") do
		for i = 0, 15 do
			w[i] = 0
			for j = 1, 4 do
				w[i] = w[i] * 256 + string.byte(str, i * 4 + j)
			end
		end
		for i = 16, 79 do
			w[i] = LeftRotate(bit.bxor(bit.bxor(w[i - 3], w[i - 8]), bit.bxor(w[i - 14], w[i - 16])), 1)
		end
		a = h0
		b = h1
		c = h2
		d = h3
		e = h4
		for i = 0, 79 do
			if (i < 20) then
				f = bit.bor(bit.band(b, c), bit.band(bit.bnot(b), d))
				k = 1518500249
			elseif (i < 40) then
				f = bit.bxor(bit.bxor(b, c), d)
				k = 1859775393
			elseif (i < 60) then
				f = bit.bor(bit.bor(bit.band(b, c), bit.band(b, d)), bit.band(c, d))
				k = 2400959708
			else
				f = bit.bxor(bit.bxor(b, c), d)
				k = 3395469782
			end
			t = LeftRotate(a, 5) + f + e + k + w[i]	
			e = d
			d = c
			c = LeftRotate(b, 30)
			b = a
			a = t
		end
		h0 = bit.band(h0 + a, 4294967295)
		h1 = bit.band(h1 + b, 4294967295)
		h2 = bit.band(h2 + c, 4294967295)
		h3 = bit.band(h3 + d, 4294967295)
		h4 = bit.band(h4 + e, 4294967295)
		str = string.sub(str, 65)
	end
end

-------------------------------------------------

function Sha1Fun(str)
	str = PreProcess(str)
	h0  = 1732584193
	h1  = 4023233417
	h2  = 2562383102
	h3  = 0271733878
	h4  = 3285377520
	MainLoop(str)
	return  ToHex(h0) ..
		ToHex(h1) ..
		ToHex(h2) ..
		ToHex(h3) ..
		ToHex(h4)
end

-------------------------------------------------
-------------------------------------------------
-------------------------------------------------
