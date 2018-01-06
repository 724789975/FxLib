
tableCmdOptions =
{
	["add"] = function(strParam)
		print("add : ", strParam)
	end,
	["delete"] = function(strParam)
		print("delete : ", strParam)
	end
}

function Option(strCmd, strParam)
	--print("test")
	print("cmd : ", strCmd, ", param : ", strParam)
	if tableCmdOptions[strCmd] == nil then
		print(debug.traceback(), "\n")
	else
		tableCmdOptions[strCmd](strParam)
	end
end

