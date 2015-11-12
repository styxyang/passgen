LUA_VER_FLAG ?= LUA_52
LUA_TARGET ?= lua52

.PHONY: default lua52 lua51

default:
	$(MAKE) $(LUA_TARGET)

lua52: LUA_VER_FLAG = LUA_52

lua51: LUA_VER_FLAG = LUA_51

lua51 lua52:
	$(MAKE) all

PASSGEN_CLIB = passgen

CLIB_PATH = lib

CFLAGS = -g -O2 -Wall --shared -fPIC -D$(LUA_VER_FLAG)
LD_FLAGS = -lssl

all: $(foreach v, $(PASSGEN_CLIB), $(CLIB_PATH)/$(v).so)

$(CLIB_PATH):
	@[[ -d $(CLIB_PATH) ]] || mkdir $(CLIB_PATH)

$(CLIB_PATH)/passgen.so: src/passgen.c src/passgen_pack.c | $(CLIB_PATH)
	@$(CC) $(CFLAGS) $(LD_FLAGS) $^ -o $@

clean:
	@rm -r lib

