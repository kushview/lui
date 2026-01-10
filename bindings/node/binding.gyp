{
  "targets": [
    {
      "target_name": "lui_node",
      "sources": [
        "lui_node.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "../../include",
        "../../src",
        "/opt/homebrew/include"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
      "conditions": [
        ["OS=='mac'", {
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "CLANG_CXX_LIBRARY": "libc++",
            "MACOSX_DEPLOYMENT_TARGET": "13.3",
            "OTHER_CFLAGS": [
              "-std=c++20"
            ],
            "OTHER_CPLUSPLUSFLAGS": [
              "-std=c++20"
            ]
          },
          "libraries": [
            "<(module_root_dir)/../../build-node/lib/liblui-0.0.a",
            "-framework Cocoa",
            "-framework OpenGL"
          ]
        }],
        ["OS=='linux'", {
          "libraries": [
            "<(module_root_dir)/../../build-node/lib/liblui-0.0.a",
            "-lGL",
            "-lX11"
          ],
          "cflags_cc": [
            "-std=c++20"
          ]
        }],
        ["OS=='win'", {
          "libraries": [
            "<(module_root_dir)/../../build-node/lib/lui-0.0.lib",
            "opengl32.lib"
          ],
          "msvs_settings": {
            "VCCLCompilerTool": {
              "AdditionalOptions": [ "/std:c++20" ]
            }
          }
        }]
      ]
    }
  ]
}
