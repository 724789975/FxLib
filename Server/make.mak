
all:
	cd common
	nmake /f make.mak
	cd ..

	cd Database
	nmake /f make.mak
	cd ..

	cd LuaLib
	nmake /f make.mak
	cd ..

	cd Net
	nmake /f make.mak
	cd ..

	cd UrlRequest
	nmake /f make.mak
	cd ..

	cd RedisCon
	nmake /f make.mak
	cd ..

	cd RedisTest
	nmake /f make.mak
	cd ..

	cd TestProto
	nmake /f make.mak
	cd ..

dbg:
	cd common
	nmake /f make.mak DEBUG=1
	cd ..

	cd Database
	nmake /f make.mak DEBUG=1
	cd ..

	cd LuaLib
	nmake /f make.mak DEBUG=1
	cd ..

	cd Net
	nmake /f make.mak DEBUG=1
	cd ..

	cd UrlRequest
	nmake /f make.mak DEBUG=1
	cd ..

	cd RedisCon
	nmake /f make.mak DEBUG=1
	cd ..

	cd RedisTest
	nmake /f make.mak DEBUG=1
	cd ..

	cd TestProto
	nmake /f make.mak DEBUG=1
	cd ..

clean:
	cd common
	nmake /f make.mak clean
	cd ..

	cd Database
	nmake /f make.mak clean
	cd ..

	cd LuaLib
	nmake /f make.mak clean
	cd ..

	cd Net
	nmake /f make.mak clean
	cd ..

	cd UrlRequest
	nmake /f make.mak clean
	cd ..

	cd RedisCon
	nmake /f make.mak clean
	cd ..

	cd RedisTest
	nmake /f make.mak clean
	cd ..

	cd TestProto
	nmake /f make.mak clean
	cd ..





