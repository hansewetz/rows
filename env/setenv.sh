export STDFLAG="-std=c++2a"                                    
export GCC_ERROR_IGNORE="-Wno-deprecated -Wno-register"        # we get some warnings (which are turned into errors) with c++17
export GCC_USE_GLIBCXX11_ABI="-D_GLIBCXX_USE_CXX11_ABI=0"      # gcc5+ uses a different ABI - we must specify this to avoid link errors

export BOOST_INC=/usr/include                                  # location of boost header files
export BOOST_LIB=/usr/lib64                                    # location of boost libs

export OCCI_INC=/usr/include/oracle/11.2/client64
export OCCI_LIB=/usr/lib/oracle/11.2/client64/lib

export INSTALL_HOME=~/installs                                 # local installations


export PATH=${INSTALL_HOME}/bin:${PATH}
export LD_LIBRARY_PATH=${PROJECT_ROOT}/artefacts/lib:${INSTALL_HOME}/lib64:${INSTALL_HOME}/lib:${LD_LIBRARY_PATH}:${PROJECT_ROOT}/artefacts/lib

# build environment root
export ENV_ROOT=`pwd`
export PROJECT_ROOT=${ENV_ROOT}/..
