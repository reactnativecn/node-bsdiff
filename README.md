# node-bsdiff

Create patch buffer with origin buffer with [bsdiff](http://www.daemonology.net/bsdiff/)

Patch compatible with bsdiff v4.3

## Installation

```bash
npm install --save node-bsdiff
```

## Usage

### diff(originBuf, newBuf)

Compare two buffers and return a new bsdiff patch as return value.
