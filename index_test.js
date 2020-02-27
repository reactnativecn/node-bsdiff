/**
 * Created by tdzl2003 on 2/28/16.
 */

var native;
try {
  native = require('./build/Release/bsdiff');
} catch(e) {
  console.error(e);
  native = require('./build/Debug/bsdiff');
}
exports.native = native;

var compressjs = require("compressjs");
var algorithm = compressjs.Bzip2;

exports.diff = function(oldBuf, newBuf) {
  var buffers = [];
  native.diff(oldBuf, newBuf, function(output){
    buffers.push(output);
  });
  var full = Buffer.concat(buffers);

  // Generate bzip2 package with header.
  var header = Buffer.alloc(32);
  Buffer.from('ENDSLEY/BSDIFF43').copy(header, 0);
  header.writeUInt32LE(newBuf.length, 16);

  var buffers1 = [header];
  // var compressed = algorithm.compressFile(full, null, 9);
  // buffers1.push(compressed);
  native.compress(full, function(output){
    buffers1.push(output);
  });

  return Buffer.concat(buffers1);
}


function makeInStream(in_fd) {
  var stream = new compressjs.Stream()
  var stat = fs.fstatSync(in_fd)
  if (stat.size) {
    stream.size = stat.size
  }
  stream.buffer = Buffer.alloc(4096)
  stream.filePos = null
  stream.pos = 0
  stream.end = 0
  stream._fillBuffer = function() {
    this.end = fs.readSync(
      in_fd,
      this.buffer,
      0,
      this.buffer.length,
      this.filePos
    )
    this.pos = 0
    if (this.filePos !== null && this.end > 0) {
      this.filePos += this.end
    }
  }
  stream.readByte = function() {
    if (this.pos >= this.end) {
      this._fillBuffer()
    }
    if (this.pos < this.end) {
      return this.buffer[this.pos++]
    }
    return -1
  }
  stream.read = function(
    buffer,
    bufOffset,
    length
  ) {
    if (this.pos >= this.end) {
      this._fillBuffer()
    }
    var bytesRead = 0
    while (bytesRead < length && this.pos < this.end) {
      buffer[bufOffset++] = this.buffer[this.pos++]
      bytesRead++
    }
    return bytesRead
  }
  stream.seek = function(seek_pos) {
    this.filePos = seek_pos
    this.pos = this.end = 0
  }
  stream.eof = function() {
    if (this.pos >= this.end) {
      this._fillBuffer()
    }
    return this.pos >= this.end
  }
  return stream
}