def set_options(opt):
  opt.tool_options('compiler_cxx')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')
  if not conf.check_cfg(package='libcurl', args='--cflags --libs', uselib_store='CURL'):
    if not conf.check(lib="curl", uselib_store="CURL"):
      conf.fatal('Missing libcurl');

def build(bld):
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  obj.cxxflags = [
    '-Wall',
    '-D_FILE_OFFSET_BITS=64',
    '-D_LARGEFILE_SOURCE',
    '-std=c++0x',
    '-O3',
    '-DNDEBUG'
  ]
  obj.target = 'addon'
  obj.source = [
    'src/addon.cc',
    'src/curl_easy_wrapper.cc',
    'src/curl_multi_wrapper.cc',
    'src/helpers.cc'
  ]
  obj.env.append_value('LINKFLAGS','-lcurl')
