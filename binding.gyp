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
            'conditions': [
                ['OS=="linux"',
                    {
                        'libraries': [
                            '<!@(pkg-config --libs glib-2.0 gobject-introspection-1.0)'
                        ],
                        'cflags': [
                            '<!@(pkg-config --cflags glib-2.0 gobject-introspection-1.0)',
                            '-D_FILE_OFFSET_BITS=64',
                            '-D_LARGEFILE_SOURCE'
                        ],
                        'ldflags': [
                            '<!@(pkg-config --libs glib-2.0 gobject-introspection-1.0)'
                        ]
                    }
                ]
            ],
            'include_dirs': [
                '<!(node -e "require(\'nan\')")',
                'src'
            ],
            'cflags': [
                '-std=c++11',
                '-g'
            ],
            'cflags!': [
                '-fno-exceptions'
            ],
            'cflags_cc!': [
                '-fno-exceptions'
            ]
        }
    ]
}
