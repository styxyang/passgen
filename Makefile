LUA_VER_FLAG ?= none
LUA_TARGET ?= none
LUA_VERSIONS = lua51 lua52

.PHONY: default lua51 lua52

default:
	$(MAKE) $(LUA_TARGET)

none :
	@echo "Please do 'make LUA_VER' where LUA_VER is one of these:"
	@echo "   $(LUA_VERSIONS)"

lua52: LUA_VER_FLAG = LUA_52

lua51: LUA_VER_FLAG = LUA_51

lua51 lua52: 
	$(MAKE) all LUA_VER_FLAG=$(LUA_VER_FLAG)

PASSGEN_CLIB = passgen

CLIB_PATH = lib

CFLAGS = -g -O2 -Wall --shared -fPIC -D$(LUA_VER_FLAG) -I$(LUA_INC)
LD_FLAGS = -lssl

all: $(foreach v, $(PASSGEN_CLIB), $(CLIB_PATH)/$(v).so)

$(CLIB_PATH):
	@[[ -d $(CLIB_PATH) ]] || mkdir $(CLIB_PATH)

$(CLIB_PATH)/passgen.so: src/passgen.c src/passgen_pack.c | $(CLIB_PATH)
	$(CC) $(CFLAGS) $(LD_FLAGS) $^ -o $@

clean:
	@[[ ! -d $(CLIB_PATH) ]] || rm -r $(CLIB_PATH)

