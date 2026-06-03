var bsdiff = require("../index");
var test_bsdiff = require("../index_test");
var crypto = require("crypto");
var assert = require("assert").strict;

var cur = crypto.randomBytes(40960);

var ref = Buffer.concat([
  Buffer.from("fdsa"),
  cur.slice(0, 4096),
  Buffer.from("asdf"),
  cur.slice(4096),
  Buffer.from("asdf")
]);

var directDiff = bsdiff.diff(cur, ref);
var callbackDiff = test_bsdiff.diff(cur, ref);
assert.deepStrictEqual(directDiff, callbackDiff);

var typedArrayDiff = bsdiff.diff(new Uint8Array(cur), new Uint8Array(ref));
assert.deepStrictEqual(typedArrayDiff, directDiff);

assert.strictEqual(directDiff.subarray(0, 16).toString(), "ENDSLEY/BSDIFF43");
assert.strictEqual(directDiff.readUInt32LE(16), ref.byteLength);
console.log("pass");
