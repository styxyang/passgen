# passgen
A password generator in C and originally for nginx-lua-module. The design
follows [oneshallpass](https://github.com/maxtaco/oneshallpass) and
[mypass](https://github.com/cyfdecyf/mypass)

# Install & Configure
Compile:
```
git clone https://github.com/styxyang/passgen.git
cd passgen
make lua51
```

Nginx configuration:
```conf
lua_package_cpath '/path/of/passgen/lib/?.so;;';

server {
...
    location /hellolua {
        default_type 'text/plain';
        content_by_lua '
            local ml = require("passgen")
            local salt = ngx.var.arg_salt or "Anonymous"
            local passphrase = ngx.var.arg_passphrase or "Anonymous"
            local site = ngx.var.arg_site or "Anonymous"
            local gen = ngx.var.arg_gen or "Anonymous"
            local iter = ngx.var.arg_iter or "Anonymous"
            ngx.say(ml.generate(salt, passphrase, site, gen, iter))
        ';
    }
}
```

# Suggestions
1. To ensure security, make sure the server is using HTTPS so the requests and responses are encrypted
