
DIRS = Database LuaLib Net LuaTest TestClient TestServer

release:
	@for dir in ${DIRS}; do\
	   		($(MAKE) -C $$dir all) || exit 1; \
			done
debug:
	@for dir in ${DIRS}; do\
	   		($(MAKE) BUILD=DEBUG -C $$dir all) || exit 1; \
			done
clean:
	@for dir in ${DIRS}; do\
	   		($(MAKE) -C $$dir clean) || exit 1; \
			done

