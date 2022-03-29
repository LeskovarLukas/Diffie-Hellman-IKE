project('leskovar_project', 'cpp',
        license : ['Boost'],
        version : '0.1',
        default_options : ['cpp_std=c++17', 'warning_level=3', 'werror=true'])

thread_dep = dependency('threads')


# # begin protobuf: https://developers.google.com/protocol-buffers/
# # protobuf: must be installed independently and has to be found...
# protoc = find_program('protoc', required : true)
# protobuf_dep = dependency('protobuf', required : true)

# gen = generator(protoc, \
#   output    : ['@BASENAME@.pb.cc', '@BASENAME@.pb.h'],
#   arguments : ['--proto_path=@CURRENT_SOURCE_DIR@/src', '--cpp_out=@BUILD_DIR@', '@PLAINNAME@'])

# # change proto file accordingly
# generated = gen.process('src/Message.proto')
# # end protobuf


inc_dir = include_directories('include')
client_src = ['src/client.cpp', 'src/tls_client.cpp']
server_src = ['src/server.cpp', 'src/tls_server.cpp']


subdir('tls_util')


executable('tls_client',
           sources: client_src,
           include_directories: [inc_dir, tls_util_inc_dir],
           dependencies: [thread_dep],
           link_with: tls_util_lib
          )   

executable('tls_server',
           sources: server_src,
           include_directories: [inc_dir, tls_util_inc_dir],
           dependencies: [thread_dep],
           link_with: tls_util_lib
          )    
