# vim: fileencoding=utf-8
import platform
import sys
import os
from os.path import join, dirname, abspath
root_dir = dirname(File('SConstruct').rfile().abspath)
sys.path.append(join(root_dir, 'tools'))

def GetVariables():
  return Variables()

def Az(context):
  return context.SConscript(
    'src/SConscript',
    variant_dir=join(root_dir, 'obj', 'az'),
    src=join(root_dir, 'src'),
    duplicate=False,
    exports="context root_dir"
  )

def Build():
  options = {}
  var = GetVariables()
  var.AddVariables(
    BoolVariable('debug', '', 0),
    BoolVariable('gprof', '', 0),
    BoolVariable('gcov', '', 0),
    BoolVariable('clang', '', 0),
    BoolVariable('release', '', 0)
  )
  env = Environment(options=var)
  env.VariantDir(join(root_dir, 'obj'), join(root_dir, 'src'), 0)

  env.PrependENVPath('PATH', os.environ['PATH']) #especially MacPorts's /opt/local/bin

  if os.path.exists(join(root_dir, '.config')):
    env.SConscript(
      join(root_dir, '.config'),
      duplicate=False,
      exports='root_dir env options')

  if options.get('cache'):
    env.CacheDir('cache')

  if env['gprof']:
    env.Append(
      CCFLAGS=["-pg"],
      LINKFLAGS=["-pg"]
    )

  if env['gcov']:
    env.Append(
      CCFLAGS=["-coverage"],
      LINKFLAGS=["-coverage"]
    )

  if env['clang']:
    env.Replace(CXX='clang++', CC='clang')

  if env['debug']:
    env.Append(CCFLAGS=["-g"])
  else:
    env.Append(
        CCFLAGS=["-O3"],
        CPPDEFINES=["NDEBUG"])

  env.Append(
    CCFLAGS=[
      "-pedantic",
      "-Wall", "-Wextra", "-Werror", '-pipe',
      "-Wno-unused-parameter", "-Wwrite-strings", "-Wreturn-type", "-Wpointer-arith",
      "-Wwrite-strings", "-Wno-long-long", "-Wno-missing-field-initializers"],
    CPPPATH=[join(root_dir, 'src'), join(root_dir, 'iv')],
    CPPDEFINES=[
      "_GNU_SOURCE",
      "__STDC_LIMIT_MACROS",
      "__STDC_CONSTANT_MACROS"],
    LIBPATH=["/usr/lib"])
  env['ENV']['GTEST_COLOR'] = os.environ.get('GTEST_COLOR')
  env['ENV']['HOME'] = os.environ.get('HOME')
  env.Replace(YACC='bison', YACCFLAGS='--name-prefix=yy_loaddict_')
  Help(var.GenerateHelpText(env))

  env.Alias('az', Az(env))
  env.Default('az')

Build()
