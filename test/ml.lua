#!/usr/bin/env lua

local ml = require("mylib")
print(string.format("digest: %s", string.sub(ml.passgen("salt", "password", "google.com", 1, 8), 0, 16)))
