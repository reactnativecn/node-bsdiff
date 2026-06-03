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
        "BZ_NO_STDIO",
        "NAPI_VERSION=8"
      ],
      "include_dirs" : [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "conditions": [
        ["OS==\"mac\"", {
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "CLANG_CXX_LIBRARY": "libc++",
            "MACOSX_DEPLOYMENT_TARGET": "10.15"
          }
        }],
        ["OS==\"win\"", {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1
            }
          }
        }]
      ]
    }
  ]
}
