project('mesonlsp', ['c', 'vala'],
	version: '0.1.0',
	meson_version: '>= 0.59.0',
	default_options: ['warning_level=2', 'werror=false'],
)
vapi_dir = meson.current_source_dir() / 'vapi'
add_project_arguments(['--vapidir', vapi_dir], language: 'vala')
subdir('docs')
subdir('parser')
subdir('src')
