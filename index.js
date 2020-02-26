/**
 * Created by tdzl2003 on 2/28/16.
 */

var native;
try {
  native = require("./build/Release/bsdiff");
} catch (e) {
  console.error(e);
  native = require("./build/Debug/bsdiff");
}
exports.native = native;

var compressjs = require("compressjs");
var algorithm = compressjs.Bzip2;

exports.diff = function(oldBuf, newBuf) {
  var buffers = [];
  native.diff(oldBuf, newBuf, function(output) {
    buffers.push(output);
  });
  var full = Buffer.concat(buffers);

  // Generate bzip2 package with header.
  var header = Buffer.alloc(32);
  header.fill("ENDSLEY/BSDIFF43", 0, 16);

  var buffers1 = [header];
  var compressed = algorithm.compressFile(full);
  buffers1.push(compressed);
  // console.log({compressed})
  // native.compress(full, function(output){
  //   buffers1.push(output);
  //   console.log({output:output.buffer})
  // });

  return Buffer.concat(buffers1);
};
