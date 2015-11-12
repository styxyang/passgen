PASSGEN_CLIB = passgen

LUA_VER = LUA_52

CFLAGS = -g -O2 -Wall --shared -fPIC
LD_FLAGS = -lssl

$(CLIB_PATH):
	[[ -d $(CLIB_PATH) ]] || mkdir $(CLIB_PATH)

all: $(foreach v, $(PASSGEN_CLIB), $(CLIB_PATH)/$(v).so)

$(CLIB_PATH)/passgen.so: src/passgen.c src/passgen_pack.c | $(CLIB_PATH)
	$(CC) $(CFLAGS) $(LD_FLAGS) $^ -o $@
