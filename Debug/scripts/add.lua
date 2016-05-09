function add(a, b)
	print("test")
	print("a : " .. a .. " b : " .. b)
	return a + b
end

function test(a, b)
	print("a.x : " .. a.x .. " b : " .. b)
	a.x = a.x + b
	return a
end

function test1(a, b)
	print("a : " .. a .. " b : " .. b)
	--return "ttttt\n"
	return debug.traceback() .. "\n"
end

print("bbbbbbb")

for i = 0, 9 do
	--a[i] = i
	--print("test ", a[i])
end
