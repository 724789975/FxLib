--module("timer", package.seeall)
require "os"
function GetTimeSecond()
	--print ("time : " .. os.time())
	return tonumber(os.time())
end

function GetTimeStr(dwTimeSecond)
	return tostring(os.date("%Y-%m-%d %H:%M:%S", dwTimeSecond))
end

