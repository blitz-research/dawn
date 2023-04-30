
PACKAGE="dawn"

CONFIG_COMMON="-DDAWN_BUILD_SAMPLES=0 -DTINT_BUILD_SAMPLES=0 -DTINT_BUILD_TESTS=0 -DTINT_BUILD_DOCS=0 -DGLFW_INSTALL=0"
CONFIG_WINDOWS=""
CONFIG_LINUX=""

TARGET="bundling_target"

cd dawn
if [ ! -f .gclient ]; then
    cp scripts/standalone.gclient .gclient
    gclient sync
fi
cd ..
