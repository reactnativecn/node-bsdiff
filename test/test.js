var bsdiff = require("../index");
var test_bsdiff = require("../index_test");
var crypto = require("crypto");
var md5 = require("md5");
var assert = require("assert").strict;

var cur = crypto.randomBytes(40960);

var ref = Buffer.concat([
  Buffer.from("fdsa"),
  cur.slice(0, 4096),
  Buffer.from("asdf"),
  cur.slice(4096),
  Buffer.from("asdf")
]);

assert.deepStrictEqual(
  md5(bsdiff.diff(cur, ref)),
  md5(test_bsdiff.diff(cur, ref))
);
console.log("pass");
