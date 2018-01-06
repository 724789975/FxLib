require "alt_getopt"

--看命令行是干什么的 要是查看信息的话 就不进行游戏
local function Help(strParam)
	print("help info")
end

local function Version(strParam)
	print("version info")
end

local function SetPort(strParam)
	g_dwPort = tonumber(strParam)
end

local function SetIp(strParam)
	g_strIp = strParam
end

local function SetIndex_l(strParam)
	SetIndex(tonumber(strParam))
end

local tableCmdOptions =
{
	["add"] = function(strParam)
		print("add : ", strParam)
	end,
	["delete"] = function(strParam)
		print("delete : ", strParam)
	end,
	["help"] = Help,
	["h"] = Help,
	["v"] = Version,
	["version"] = Version,
	["port"] = SetPort,
	["p"] = SetPort,
	["ip"] = SetIp,
	["index"] = SetIndex_l,
}

local function CommandOption(strCmd, strParam)
	--print("test")
	print("cmd : ", strCmd, ", param : ", strParam)
	if tableCmdOptions[strCmd] == nil then
		print(debug.traceback())
	else
		tableCmdOptions[strCmd](strParam)
	end
end

local long_opts =
{
	-- 长命令 = 短命令
	--verbose = "v",
	version = "v",
	help = "h",
	port = "p",
	fake = 0,
	len	= 1,
	output = "o",
	set_value = "S",
	["set-output"] = "o",
	test = 2,
	add = 3,
	delete = 4,
	ip = 5,
	index = 6,
}

tableCommandLine = {}

function GetoptLong()
	local bRun = true
	--local ret
	local optarg = {}
	local optind = 0
	local opts = {}
	opts,optind,optarg = alt_getopt.get_ordered_opts (tableCommandLine, "hVvp:o:n:S:", long_opts)
	for i,v in ipairs (opts) do
		if v == "h" or v == "v" then
			bRun = false
		end
		if optarg [i] then
			CommandOption(v, optarg[i])
			--print("option `" .. v .. "': " .. optarg [i])
		else
			CommandOption(v, nil)
			--print("option `" .. v)
		end
	end
	return bRun
end


--example :
--
--local ret
--local optarg
--local optind
--opts,optind,optarg = alt_getopt.get_ordered_opts (arg, "hVvo:n:S:", long_opts)
--for i,v in ipairs (opts) do
--	if optarg [i] then
--		io.write ("option `" .. v .. "': " .. optarg [i] .. "\n")
--	else
--		io.write ("option `" .. v .. "'\n")
--	end
--end
--
--optarg,optind = alt_getopt.get_opts (arg, "hVvo:n:S:", long_opts)
--for k,v in pairs (optarg) do
--	io.write ("fin-option `" .. k .. "': " .. v .. "\n")
--end
--
--for i = optind,#arg do
--	io.write (string.format ("ARGV [%s] = %s\n", i, arg [i]))
--end
--
