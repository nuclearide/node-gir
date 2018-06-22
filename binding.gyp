{
    'targets': [
        {
            'target_name': 'girepository',
            'sources': [
                'src/main.cpp',
                'src/util.cpp',
                'src/namespace_loader.cpp',
                'src/arguments.cpp',
                'src/values.cpp',
                'src/types/object.cpp',
                'src/types/struct.cpp',
                'src/types/function.cpp',
                'src/types/enum.cpp',
                'src/loop.cpp',
                'src/closure.cpp'
            ],
            'include_dirs': [
                '<!(node -e "require(\'nan\')")',
                '<!@(PKG_CONFIG_PATH=/usr/local/opt/libffi/lib/pkgconfig pkg-config glib-2.0 gobject-introspection-1.0 --cflags-only-I | sed s/-I//g)',
                'src'
            ],
            'libraries': [
                '<!@(pkg-config --libs glib-2.0 gobject-introspection-1.0)'
            ],
            'cflags': [
                '-std=c++11',
                '-fexceptions',
                '-g'
            ],
            'cflags!': [
                '-fno-exceptions'
            ],
            'cflags_cc!': [
                '-fno-exceptions'
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
                    }
                }]
            ]
        }
    ]
}
