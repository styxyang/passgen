default:
	gcc --shared -fPIC -DLUA_51 -lssl mylib.c passgen_pack.c -o mylib.so
