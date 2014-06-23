AR = '/usr/bin/ar'
ARFLAGS = 'rcs'
BINDIR = '/usr/local/bin'
BOOST_VERSION = '1_48'
CC_VERSION = ('4', '6', '3')
CFLAGS_NS3_APPLICATIONS = ['-pthread']
CFLAGS_NS3_CORE = ['-pthread']
CFLAGS_NS3_INTERNET = ['-pthread']
CFLAGS_NS3_MOBILITY = ['-pthread']
CFLAGS_NS3_NDNSIM = ['-pthread']
CFLAGS_NS3_NETWORK = ['-pthread']
CFLAGS_NS3_POINT-TO-POINT = ['-pthread']
CFLAGS_NS3_TOPOLOGY-READ = ['-pthread']
CFLAGS_NS3_VISUALIZER = ['-pthread']
COMPILER_CXX = 'g++'
CPPPATH_ST = '-I%s'
CXX = ['/usr/bin/g++']
CXXFLAGS = ['-O0', '-g3']
CXXFLAGS_MACBUNDLE = ['-fPIC']
CXXFLAGS_NS3_APPLICATIONS = ['-pthread']
CXXFLAGS_NS3_CORE = ['-pthread']
CXXFLAGS_NS3_INTERNET = ['-pthread']
CXXFLAGS_NS3_MOBILITY = ['-pthread']
CXXFLAGS_NS3_NDNSIM = ['-pthread']
CXXFLAGS_NS3_NETWORK = ['-pthread']
CXXFLAGS_NS3_POINT-TO-POINT = ['-pthread']
CXXFLAGS_NS3_TOPOLOGY-READ = ['-pthread']
CXXFLAGS_NS3_VISUALIZER = ['-pthread']
CXXFLAGS_cxxshlib = ['-fPIC']
CXXLNK_SRC_F = []
CXXLNK_TGT_F = ['-o']
CXX_NAME = 'gcc'
CXX_SRC_F = []
CXX_TGT_F = ['-c', '-o']
DEFINES = ['NS3_LOG_ENABLE', 'HAVE_NS3_NDNSIM=1', 'HAVE_NS3_CORE=1', 'HAVE_NS3_NETWORK=1', 'HAVE_NS3_INTERNET=1', 'HAVE_NS3_POINT_TO_POINT=1', 'HAVE_NS3_TOPOLOGY_READ=1', 'HAVE_NS3_APPLICATIONS=1', 'HAVE_NS3_MOBILITY=1', 'HAVE_NS3_VISUALIZER=1', 'NS3_LOG_ENABLE=1', 'NS3_ASSERT_ENABLE=1', '_DEBUG=1']
DEFINES_ST = '-D%s'
DEST_BINFMT = 'elf'
DEST_CPU = 'x86_64'
DEST_OS = 'linux'
INCLUDES_BOOST = '/usr/include'
INCLUDES_NS3_APPLICATIONS = ['/usr/local/include/ns3-dev']
INCLUDES_NS3_CORE = ['/usr/local/include/ns3-dev']
INCLUDES_NS3_INTERNET = ['/usr/local/include/ns3-dev']
INCLUDES_NS3_MOBILITY = ['/usr/local/include/ns3-dev']
INCLUDES_NS3_NDNSIM = ['/usr/local/include/ns3-dev']
INCLUDES_NS3_NETWORK = ['/usr/local/include/ns3-dev']
INCLUDES_NS3_POINT-TO-POINT = ['/usr/local/include/ns3-dev']
INCLUDES_NS3_TOPOLOGY-READ = ['/usr/local/include/ns3-dev']
INCLUDES_NS3_VISUALIZER = ['/usr/local/include/ns3-dev']
LIBDIR = '/usr/local/lib'
LIBPATH_BOOST = ['/usr/lib']
LIBPATH_NS3_APPLICATIONS = ['/usr/local/lib']
LIBPATH_NS3_CORE = ['/usr/local/lib']
LIBPATH_NS3_INTERNET = ['/usr/local/lib']
LIBPATH_NS3_MOBILITY = ['/usr/local/lib']
LIBPATH_NS3_NDNSIM = ['/usr/local/lib']
LIBPATH_NS3_NETWORK = ['/usr/local/lib']
LIBPATH_NS3_POINT-TO-POINT = ['/usr/local/lib']
LIBPATH_NS3_TOPOLOGY-READ = ['/usr/local/lib']
LIBPATH_NS3_VISUALIZER = ['/usr/local/lib']
LIBPATH_ST = '-L%s'
LIB_BOOST = ['boost_system-mt', 'boost_iostreams-mt']
LIB_NS3_APPLICATIONS = ['ns3-dev-applications-debug', 'ns3-dev-internet-debug', 'ns3-dev-config-store-debug', 'ns3-dev-tools-debug', 'ns3-dev-bridge-debug', 'ns3-dev-mpi-debug', 'ns3-dev-stats-debug', 'ns3-dev-network-debug', 'ns3-dev-core-debug', 'rt']
LIB_NS3_CORE = ['ns3-dev-core-debug', 'rt']
LIB_NS3_INTERNET = ['ns3-dev-internet-debug', 'ns3-dev-bridge-debug', 'ns3-dev-mpi-debug', 'ns3-dev-network-debug', 'ns3-dev-core-debug', 'rt']
LIB_NS3_MOBILITY = ['ns3-dev-mobility-debug', 'ns3-dev-network-debug', 'ns3-dev-core-debug', 'rt']
LIB_NS3_NDNSIM = ['ns3-dev-ndnSIM-debug', 'ns3-dev-visualizer-debug', 'ns3-dev-topology-read-debug', 'ns3-dev-internet-debug', 'ns3-dev-wifi-debug', 'ns3-dev-point-to-point-debug', 'ns3-dev-bridge-debug', 'ns3-dev-propagation-debug', 'ns3-dev-mobility-debug', 'ns3-dev-mpi-debug', 'ns3-dev-network-debug', 'ns3-dev-core-debug', 'rt']
LIB_NS3_NETWORK = ['ns3-dev-network-debug', 'ns3-dev-core-debug', 'rt']
LIB_NS3_POINT-TO-POINT = ['ns3-dev-point-to-point-debug', 'ns3-dev-mpi-debug', 'ns3-dev-network-debug', 'ns3-dev-core-debug', 'rt']
LIB_NS3_TOPOLOGY-READ = ['ns3-dev-topology-read-debug', 'ns3-dev-network-debug', 'ns3-dev-core-debug', 'rt']
LIB_NS3_VISUALIZER = ['ns3-dev-visualizer-debug', 'ns3-dev-internet-debug', 'ns3-dev-wifi-debug', 'ns3-dev-point-to-point-debug', 'ns3-dev-bridge-debug', 'ns3-dev-propagation-debug', 'ns3-dev-mobility-debug', 'ns3-dev-mpi-debug', 'ns3-dev-network-debug', 'ns3-dev-core-debug', 'rt']
LIB_ST = '-l%s'
LINKFLAGS_MACBUNDLE = ['-bundle', '-undefined', 'dynamic_lookup']
LINKFLAGS_NS3_APPLICATIONS = ['-pthread', '-Wl,-Bdynamic']
LINKFLAGS_NS3_CORE = ['-pthread', '-Wl,-Bdynamic']
LINKFLAGS_NS3_INTERNET = ['-pthread', '-Wl,-Bdynamic']
LINKFLAGS_NS3_MOBILITY = ['-pthread', '-Wl,-Bdynamic']
LINKFLAGS_NS3_NDNSIM = ['-pthread', '-Wl,-Bdynamic']
LINKFLAGS_NS3_NETWORK = ['-pthread', '-Wl,-Bdynamic']
LINKFLAGS_NS3_POINT-TO-POINT = ['-pthread', '-Wl,-Bdynamic']
LINKFLAGS_NS3_TOPOLOGY-READ = ['-pthread', '-Wl,-Bdynamic']
LINKFLAGS_NS3_VISUALIZER = ['-pthread', '-Wl,-Bdynamic']
LINKFLAGS_cxxshlib = ['-shared']
LINKFLAGS_cxxstlib = ['-Wl,-Bstatic']
LINK_CXX = ['/usr/bin/g++']
NS3_CHECK_MODULE_ONCE = ''
NS3_MODULES_FOUND = ['ndnSIM', 'core', 'network', 'internet', 'point-to-point', 'topology-read', 'applications', 'mobility', 'visualizer']
PKGCONFIG = '/usr/bin/pkg-config'
PREFIX = '/usr/local'
RPATH_ST = '-Wl,-rpath,%s'
SHLIB_MARKER = ['-Wl,-Bdynamic', '-Wl,--no-as-needed']
SONAME_ST = '-Wl,-h,%s'
STLIBPATH_ST = '-L%s'
STLIB_MARKER = '-Wl,-Bstatic'
STLIB_ST = '-l%s'
cxxprogram_PATTERN = '%s'
cxxshlib_PATTERN = 'lib%s.so'
cxxstlib_PATTERN = 'lib%s.a'
define_key = ['HAVE_NS3_NDNSIM', 'HAVE_NS3_CORE', 'HAVE_NS3_NETWORK', 'HAVE_NS3_INTERNET', 'HAVE_NS3_POINT_TO_POINT', 'HAVE_NS3_TOPOLOGY_READ', 'HAVE_NS3_APPLICATIONS', 'HAVE_NS3_MOBILITY', 'HAVE_NS3_VISUALIZER', 'NS3_LOG_ENABLE', 'NS3_ASSERT_ENABLE', '_DEBUG']
macbundle_PATTERN = '%s.bundle'