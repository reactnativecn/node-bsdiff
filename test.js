/**
 * Created by tdzl2003 on 2/28/16.
 */

var bsdiff = require('./index');
var crypto = require('crypto');
var fs = require('fs');

var cur = crypto.randomBytes(40960);

var ref = Buffer.concat([new Buffer('fdsa'), cur.slice(0, 4096), new Buffer('asdf'), cur.slice(4096), new Buffer('asdf')]);

fs.writeFileSync('old', cur);
fs.writeFileSync('new', ref);
fs.writeFileSync('patch', bsdiff.diff(cur, ref));
//console.log(bsdiff.diff(cur, ref).length);