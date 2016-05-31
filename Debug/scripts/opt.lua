print("op.lua")
require "alt_getopt"

local long_opts = {
	verbose = "v",
	help = "h",
	fake = 0,
	len	= 1,
	output = "o",
	set_value = "S",
	["set-output"] = "o"
}
print("op.lua5")
print(alt_getopt)
local aaa = alt_getopt.get_opts
print(aaa)

--alt_getopt.get_ordered_opts (arg, "hVvo:n:S:", long_opts)
print("op.lua1")

function optest(arg)
	local ret
	local optarg
	local optind
	opts,optind,optarg = alt_getopt.get_ordered_opts (arg, "hVvo:n:S:", long_opts)
	for i,v in ipairs (opts) do
		if optarg [i] then
			io.write ("option `" .. v .. "': " .. optarg [i] .. "\n")
		else
			io.write ("option `" .. v .. "'\n")
		end
	end
end

local ret
local optarg
local optind
opts,optind,optarg = alt_getopt.get_ordered_opts (arg, "hVvo:n:S:", long_opts)
for i,v in ipairs (opts) do
	if optarg [i] then
		io.write ("option `" .. v .. "': " .. optarg [i] .. "\n")
	else
		io.write ("option `" .. v .. "'\n")
	end
end

print("op.lua2")

optarg,optind = alt_getopt.get_opts (arg, "hVvo:n:S:", long_opts)
for k,v in pairs (optarg) do
	io.write ("fin-option `" .. k .. "': " .. v .. "\n")
end

print("op.lua3")

for i = optind,#arg do
	io.write (string.format ("ARGV [%s] = %s\n", i, arg [i]))
end

print("op.lua4")
