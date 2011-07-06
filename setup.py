import sys, os, os.path, re, shutil, string
from distutils.core import setup, Command
from distutils.command.build import build
from distutils.command.install import install
from distutils.spawn import spawn
from glob import glob

systems = \
{
  'DQM':
  {
    'h':      ['DQM/*.h', 'DQM/DQMCollector.cc', 'DQM/MonitorElement.cc',
               'DQM/DQMNet.cc', 'DQM/DQMError.cc', 'DQM/DQMStore.cc',
               'DQM/QTest.cc', 'DQM/QReport.cc', 'DQM/QStatisticalTests.cc',
               'boost/*/*/*/*.hpp', 'rtgu/i*/*.hpp', 'rtgu/i*/*/*.hpp'],
    'cpp':    [('bin', 'visDQM*'),
	       ('bin', 'DQMCollector'),
	       ('lib', 'libDQMGUI.so'),
	       ('pylib', 'Accelerator.so', 'Monitoring/DQM')],
    'etc':    ['makefile*'],
    'bin':    ['mon*', 'visDQM*', 'dqm-*'],
    'python': ['Monitoring.Core', 'Monitoring.DQM'],
    'data':   ['javascript/Core/*.js', 'javascript/DQM/*.js',
               'css/Core/*.css', 'css/DQM/*.css',
               'images/*.png', 'images/*.gif',
               'root/*.C', 'templates/*.tmpl']
  },
  'Overview':
  {
    'h':      [],
    'cpp':    [],
    'etc':    [],
    'bin':    ['mon*'],
    'python': ['Monitoring.Core', 'Monitoring.Overview'],
    'data':   ['javascript/Core/*.js', 'javascript/Overview/*.js',
               'css/Core/*.css', 'css/Overview/*.css',
               'images/*.png', 'images/*.gif',
               'templates/*.tmpl']
  }
}

def get_relative_path():
  return os.path.dirname(os.path.abspath(os.path.join(os.getcwd(), sys.argv[0])))

def define_the_build(self, dist, system_name, run_make = True, patch_x = ''):
  # Expand various sources.
  system = systems[system_name]
  datasrc = sum((glob("src/%s" % x) for x in system['data']), [])
  etcsrc = sum((glob("etc/%s" % x) for x in system['etc']), [])
  binsrc = sum((glob("bin/%s" % x) for x in system['bin']), [])
  cpp = { 'bin': [], 'lib': [], 'pylib': [], 'h': [] }

  # If we have c++ sources, build them now.
  if system['cpp']:
    pwd = os.getcwd()
    try:
      os.chdir("%s/src/cpp" % get_relative_path())
      if run_make:
        spawn(['make', '-r', '-k', '-j', '8', '-f', '../../etc/makefile'])
      cpp['h'] = sum([glob(x) for x in system['h']], [])
      for p in system['cpp']:
        if p[0] == 'bin':
          cpp['bin'] += ["src/cpp/%s" % x for x in glob(p[1])]
        elif p[0] == 'lib':
          cpp['lib'] += ["src/cpp/%s" % x for x in glob(p[1])]
        elif p[0] == 'pylib':
          cpp['pylib'].append((p[2], ["src/cpp/%s" % x for x in glob(p[1])]))
        else:
          assert False, "unsupported cpp product type '%s'" % p[0]
    finally:
      os.chdir(pwd)

  # Specify what to install. We don't use build_ext to build extension,
  # as we have a makefile to build all the rest above anyway. Any c++
  # products we built go into data_files as python just needs to copy them.
  # Headers get special handling, we map them automatically to directories.
  py_version = (string.split(sys.version))[0]
  pylibdir = '%slib/python%s/site-packages' % (patch_x, py_version[0:3])
  dist.py_modules = ['Monitoring.__init__']
  dist.packages = system['python']
  dist.data_files = [('etc', etcsrc),
                     ('%sbin' % patch_x, binsrc + cpp['bin']),
                     ('%slib' % patch_x, cpp['lib'])]
  for module, libs in cpp['pylib']:
    dist.data_files.append(('%s/%s' % (pylibdir, module.replace('.', '/')), libs))
  for dir in set(x.rsplit('/', 1)[0] for x in cpp['h']):
    hdrs = ['src/cpp/%s' % x for x in cpp['h'] if x.startswith(dir + '/')]
    dist.data_files.append(('%sinclude/%s' % (patch_x, dir), hdrs))
  for dir in set(x[4:].rsplit('/', 1)[0] for x in datasrc):
    files = [x for x in datasrc if x.startswith('src/%s/' % dir)]
    dist.data_files.append(('%sdata/%s' % (patch_x, dir), files))

class BuildCommand(Command):
  """Build python and c++ modules for a specific system."""
  description = \
    "Build python and c++ modules for the specified system. Possible\n" + \
    "\t\t   systems are 'DQM' and 'Overview'. The former has python and c++\n" + \
    "\t\t   code, the latter has only python sources. Use with --force to\n" + \
    "\t\t   ensure a clean build of only the requested parts.\n"
  user_options = build.user_options
  user_options.append(('system=', 's', 'build the specified system'))

  def initialize_options(self):
    self.system = None

  def finalize_options(self):
    # Check options.
    if self.system == None:
      print "System not specified, please use '-s DQM' or '-s Overview'"
      sys.exit(1)
    elif self.system not in systems:
      print "System %s unrecognised, please use '-s DQM' or '-s Overview'" % self.system
      sys.exit(1)

    # Expand various sources and maybe do the c++ build.
    define_the_build(self, self.distribution, self.system, True, '')

    # Force rebuild.
    shutil.rmtree("%s/build" % get_relative_path(), True)

  def run(self):
    command = 'build'
    if self.distribution.have_run.get(command): return
    cmd = self.distribution.get_command_obj(command)
    cmd.force = self.force
    cmd.ensure_finalized()
    cmd.run()
    self.distribution.have_run[command] = 1

class InstallCommand(install):
  """Install a specific system."""
  description = \
    "Install a specific system, either 'DQM' or 'Overview'. You can\n" + \
    "\t\t   patch an existing installation instead of normal full installation\n" + \
    "\t\t   using the '-p' option.\n"
  user_options = install.user_options
  user_options.append(('system=', 's', 'install the specified system'))
  user_options.append(('patch', None, 'patch an existing installation'))

  def initialize_options(self):
    install.initialize_options(self)
    self.system = None
    self.patch = None

  def finalize_options(self):
    # Check options.
    if self.system == None:
      print "System not specified, please use '-s DQM' or '-s Overview'"
      sys.exit(1)
    elif self.system not in systems:
      print "System %s unrecognised, please use '-s DQM' or '-s Overview'" % self.system
      sys.exit(1)
    if self.patch and not os.path.isdir("%s/xbin" % self.prefix):
      print "Patch destination %s does not look like a valid location." % self.prefix
      sys.exit(1)

    # Expand various sources, but don't build anything from c++ now.
    define_the_build(self, self.distribution, self.system,
		     False, (self.patch and 'x') or '')

    # Whack the metadata name.
    self.distribution.metadata.name = self.system
    assert self.distribution.get_name() == self.system

    # Pass to base class.
    install.finalize_options(self)

    # Mangle paths if we are patching. Most of the mangling occurs
    # already in define_the_build(), but we need to fix up others.
    if self.patch:
      self.install_lib = re.sub(r'(.*)/lib/python(.*)', r'\1/xlib/python\2', self.install_lib)
      self.install_scripts = re.sub(r'(.*)/bin$', r'\1/xbin', self.install_scripts)

  def run(self):
    for cmd_name in self.get_sub_commands():
      cmd = self.distribution.get_command_obj(cmd_name)
      cmd.distribution = self.distribution
      if cmd_name == 'install_data':
        cmd.install_dir = self.prefix
      else:
        cmd.install_dir = self.install_lib
      cmd.ensure_finalized()
      self.run_command(cmd_name)
      self.distribution.have_run[cmd_name] = 1

setup(name = 'dqmgui',
      version = '1.0',
      maintainer_email = 'hn-cms-dqmDevel@cern.ch',
      cmdclass = { 'build_system': BuildCommand,
                   'install_system': InstallCommand },
      package_dir = { 'Monitoring': 'src/python' })
