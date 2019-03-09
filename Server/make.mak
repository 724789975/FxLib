
all:
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
	cd Database
	nmake /f make.mak clean
	nmake /f make.mak DEBUG=1 clean
	cd ..

	cd LuaLib
	nmake /f make.mak clean
	nmake /f make.mak DEBUG=1 clean
	cd ..

	cd Net
	nmake /f make.mak clean
	nmake /f make.mak DEBUG=1 clean
	cd ..

	cd UrlRequest
	nmake /f make.mak clean
	nmake /f make.mak DEBUG=1 clean
	cd ..

	cd RedisCon
	nmake /f make.mak clean
	nmake /f make.mak DEBUG=1 clean
	cd ..

	cd RedisTest
	nmake /f make.mak clean
	nmake /f make.mak DEBUG=1 clean
	cd ..

	cd TestProto
	nmake /f make.mak clean
	nmake /f make.mak DEBUG=1 clean
	cd ..





