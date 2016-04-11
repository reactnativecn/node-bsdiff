{
  "targets": [
    {
      "target_name": "bsdiff",
      "sources": [
        "src/main.cc",
        "src/bsdiff/bsdiff.c",
        "src/bzlib/bzlib.c",
        "src/bzlib/compress.c",
        "src/bzlib/crctable.c",
        "src/bzlib/randtable.c",
        "src/bzlib/blocksort.c",
        "src/bzlib/huffman.c",
        "src/bzlib/decompress.c"
      ],
      "defines": [
        "BZ_NO_STDIO"
      ]
    }
  ]
}
