partial_interpreter_tester = executable(
    'partial-interpreter-tester',
    'main.cpp',
    dependencies: [
        analyze_dep,
        ast_dep,
        log_dep,
        objects_dep,
    ],
)

partial_interpreter_tests = [
    [
        'foreach',
        [
            'have_foo',
            'have_bar',
            'have_baz',
            'foohave_',
            'barhave_',
            'bazhave_',
        ],
    ],
    [
        'format',
        [
            'have_2',
            'have_3',
        ],
    ],
    [
        'underscorify',
        [
            'glib_genmarshal',
            'glib_mkenums',
        ],
    ],
    [
        'harfbuzz',
        [
            'hb_shape_fuzzer_exe',
            'hb_subset_fuzzer_exe',
            'hb_set_fuzzer_exe',
            'hb_draw_fuzzer_exe',
            'hb_repacker_fuzzer_exe',
        ],
    ],
    [
        'gdkpixbuf',
        ['gdk_pixbuf_csource', 'gdk_pixbuf_query_loaders', 'gdk_pixbuf_pixdata'],
    ],
    [
        'vpx8',
        [
            'have_vp8_encoder',
            'have_vp8_decoder',
            'have_vp9_encoder',
            'have_vp9_decoder',
        ],
    ],
    [
        'vpx2',
        [
            'have_vp6_encoder',
            'have_vp6_decoder',
            'have_vp7_encoder',
            'have_vp7_decoder',
        ],
    ],
    [
        'wayland',
        [
            'wayland_client_protocol_h',
            'wayland_server_protocol_h',
            'wayland_client_protocol_core_h',
            'wayland_server_protocol_core_h',
        ],
    ],
    [
        'grilo',
        [
            'bookmarks_enabled',
            'bookmarks_idx',
            'filesystem_enabled',
            'filesystem_idx',
            'lua_factory_enabled',
            'lua_factory_idx',
            'youtube_enabled',
            'youtube_idx',
            'bookmarks_enabledfoo',
            'filesystem_enabledfoo',
            'lua_factory_enabledfoo',
            'youtube_enabledfoo',
        ],
    ],
    ['dict+join_path', ['arch1/x86', 'a/b', 'a1/b', 'arch/x861']],
    [
        'goa',
        [
            'afc',
            'goa',
            'mtp',
        ],
    ],
    [
        'dict+iter',
        [
            'dict11111',
            'dict22222',
        ],
    ],
    ['imgui', ['python_binding', 'cpp_binding', 'human_binding', 'lua___shim'], 'imgui-options'],
]

foreach test_data : partial_interpreter_tests
    file = files('partial-interpreter-test' / (test_data[0] + '.meson'))[0]
    options_file = test_data.length() == 2 ? [] : (['--'] + files('partial-interpreter-test' / (test_data[2] + '.meson')))

    test(
        'pt-' + test_data[0],
        partial_interpreter_tester,
        args: [file] + test_data[1] + options_file,
    )
endforeach