require "base64"
require "Sha1"

function ResponseKey(strWebInfo)
	local key = string.gfind(strWebInfo, "Sec%-WebSocket%-Key: [%S]+")()
	local s, n = string.gsub(key, "Sec%-WebSocket%-Key: ", "")
	key = s .. "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
	sh = Sha1.Sha1Fun(key)
	key = base64.ToBase64(sh)
	return key
end
