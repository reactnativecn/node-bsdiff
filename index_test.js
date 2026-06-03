/**
 * Created by tdzl2003 on 2/28/16.
 */

var native = require('node-gyp-build')(__dirname);
exports.native = native;

function byteLength(value) {
  if (Buffer.isBuffer(value)) {
    return value.length;
  }
  if (ArrayBuffer.isView(value)) {
    return value.byteLength;
  }
  if (value instanceof ArrayBuffer) {
    return value.byteLength;
  }
  throw new TypeError('Expected Buffer, TypedArray, or ArrayBuffer.');
}

exports.diff = function(oldBuf, newBuf) {
  var buffers = [];
  native.diff(oldBuf, newBuf, function(output){
    buffers.push(output);
  });
  var full = Buffer.concat(buffers);

  // Generate bzip2 package with header.
  var header = Buffer.alloc(32);
  Buffer.from('ENDSLEY/BSDIFF43').copy(header, 0);
  header.writeUInt32LE(byteLength(newBuf), 16);

  var buffers1 = [header];
  native.compress(full, function(output){
    buffers1.push(output);
  });

  return Buffer.concat(buffers1);
}
