{
  "targets": [
    {
      "target_name": "bsdiff",
      "sources": [
        "src/main.cc",
        "src/bsdiff/bsdiff.c"
      ],
      "defines": [
        "BZ_NO_STDIO"
      ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}
