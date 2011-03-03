import os.path, glob

CONFIGDIR = os.path.normcase(os.path.abspath(__file__)).rsplit('/', 1)[0]
TOPDIR = CONFIGDIR.rsplit('/', 1)[0]
PORT = 8030

modules = ("GuiDQM",)
envsetup = "export QUIET_ASSERT=a"
#server.instrument  = 'valgrind --num-callers=999 `cmsvgsupp`'
#server.instrument  = 'igprof -t python -pp'
server.port        = PORT
server.serverDir   = '/build/lat/dqmtest/gui'
server.baseUrl     = '/dqm/online'
server.title       = 'CMS'
server.serviceName = 'Online'
server.options     = { 'python_profile': True, 'thread_pool': 5 }

server.plugin('render', "%s/test/TestDQMRenderPlugin.cc" % TOPDIR)
server.extend('DQMRenderLink', server.pathOfPlugin('render'))
server.source('DQMUnknown')
server.source('DQMOverlay')
server.source('DQMStripChart')
server.source('DQMLive', 'localhost:9090')
server.source('DQMArchive', '/build/lat/dqmgui/idx', '^/Global/')
server.source('DQMLayout', *glob.glob("%s/*-layouts.py" % CONFIGDIR))
#server.extend('DQMFileAccess', '/dev/null', '/build/lat/dqmtest/data')

execfile(CONFIGDIR + "/dqm-services.py", globals(), locals())
execfile(CONFIGDIR + "/online-workspaces.py", globals(), locals())
