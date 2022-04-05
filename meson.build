project('leskovar_project', 'cpp',
        license : ['Boost'],
        version : '0.1',
        default_options : ['cpp_std=c++17', 'warning_level=3', 'werror=true'])

thread_dep = dependency('threads')


subdir('tls_util')

inc_dir = include_directories('include')
client_src = ['src/client.cpp', 'src/tls_client.cpp', generated]
server_src = ['src/server.cpp', 'src/tls_server.cpp', generated]


executable('tls_client',
           sources: client_src,
           include_directories: [inc_dir, tls_util_inc_dir],
           dependencies: [thread_dep],
           link_with: tls_util_lib
          )   

executable('tls_server',
           sources: server_src,
           include_directories: [inc_dir, tls_util_inc_dir],
           dependencies: [thread_dep, protobuf_dep],
           link_with: tls_util_lib
          )    
