/**
 * Created by tdzl2003 on 2/28/16.
 */

function loadNative() {
  var platform = process.platform;
  var arch = process.arch;

  if (platform === 'darwin' && arch === 'arm64') {
    return require('./prebuilds/darwin-arm64/node-bsdiff.node');
  }
  if (platform === 'linux' && arch === 'x64') {
    return require('./prebuilds/linux-x64/node-bsdiff.node');
  }
  if (platform === 'linux' && arch === 'arm64') {
    return require('./prebuilds/linux-arm64/node-bsdiff.node');
  }

  return require('node-gyp-build')(__dirname);
}

var native = loadNative();
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
  var full = native.diff(oldBuf, newBuf);

  // Generate bzip2 package with header.
  var header = Buffer.alloc(32);
  Buffer.from('ENDSLEY/BSDIFF43').copy(header, 0);
  header.writeUInt32LE(byteLength(newBuf), 16);

  return Buffer.concat([header, native.compress(full)]);
}
