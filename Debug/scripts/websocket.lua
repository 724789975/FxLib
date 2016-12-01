require "base64"
require "Sha1"

function ResponseKey(strWebInfo)
	local key = string.find(strWebInfo, "Sec%-WebSocket%-Key: [%s]+")()
	local s, n = string.sub(key, "Sec%-WebSocket%-Key: ", "")
	key = s .. "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
	sh = Sha1.Sha1Fun(key)
	key = base64.ToBase64(sh)
	return key
end