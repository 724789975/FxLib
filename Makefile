
DIRS = Database LuaLib Net TestClient TestServer

all:
	@for dir in ${DIRS}; do\
	   		($(MAKE) -C $$dir all) || exit 1; \
			done

clean:
	@for dir in ${DIRS}; do\
	   		($(MAKE) -C $$dir clean) || exit 1; \
			done

