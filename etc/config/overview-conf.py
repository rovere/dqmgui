import os.path

CONFIGDIR = os.path.normcase(os.path.abspath(__file__)).rsplit('/', 1)[0]
TOPDIR = CONFIGDIR.rsplit('/', 1)[0]
PORT = 9000

modules = ("GuiCompPhedex", "GuiFilelight", "GuiProdmon",
	   "GuiCache", "GuiCompCore", "GuiCompDDT", "GuiCompGrid",
	   "GuiCompTier0", "GuiCompWelcome", "GuiCompCAF")
envsetup = ("export MATPLOTLIBRC=%s\n"
            "export TNS_ADMIN=%s\n"
            "export HOME=%s\n"
	    % (CONFIGDIR, CONFIGDIR, CONFIGDIR))

#server.instrument  = 'valgrind --num-callers=999 `cmsvgsupp`'
#server.instrument  = 'igprof -t python -pp'
server.port        = PORT
server.serverDir   = '/build/lat/ovtest/gui'
server.baseUrl     = '/overview'
server.title       = 'CMS'
server.serviceName = 'Overview'
server.options     = { 'thread_pool': 10, 'stack_size': 512*1024 }

server.extend('OverviewCache')
server.source('Filelight')
server.source('Prodmon')
server.source('Phedex', CONFIGDIR + '/dbparam.py')
server.source('CAFPlot')
server.source('CAFDisk',
	      ('alca', r"alca"),
	      ('comm', r"^(/calo/|/minimumbias/|/monitor/|/testenables/"
			"|/hltdebug/|/hcalhpdnoise/|/randomtriggers/"
			"|/test/commissioning|/calprivate|/barrelmuon/"
			"|/endcapsmuon/|/minbias/|/[^/]*(cosmic|beamhalo)"
			"|/global[^/]*-A/)"),
	      ('phys', '^'))
server.source('CompDDT')
server.source('CompTier0')

server.workspace('CompWelcome',  10, 'Shift',     'Welcome')
server.workspace('CompCore',     11, 'Shift',     'CERN/Core')
server.workspace('CompTier0',    12, 'Shift',     'Tier-0')
server.workspace('CompCAF',      13, 'Shift',     'CAF')
server.workspace('CompDDT',      14, 'Shift',     'Transfers')
server.workspace('CompGrid',     15, 'Shift',     'Grid Services')
server.workspace('CompPhedex',   20, 'Activity',  'PhEDEx')
server.workspace('Filelight',    21, 'Activity',  'Transfer Summary')
server.workspace('Prodmon',      22, 'Activity',  'Prodmon')

server.service('Overview', 'http://cmsweb.cern.ch/overview')
