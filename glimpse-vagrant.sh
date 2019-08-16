#!/bin/sh

export INSTALL_PREFIX=/vagrant/glimpse-prefix
export SRC_PREFIX=/vagrant
export PATH=$HOME/.local/bin:$INSTALL_PREFIX/bin:$PATH
export PKG_CONFIG_PATH=$INSTALL_PREFIX/lib/pkgconfig:$INSTALL_PREFIX/lib/x86_64-linux-gnu/pkgconfig:$INSTALL_PREFIX/share/pkgconfig:$PKG_CONFIG_PATH
export LD_LIBRARY_PATH=$INSTALL_PREFIX/lib:$INSTALL_PREFIX/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH
export ACLOCAL_FLAGS="-I $INSTALL_PREFIX/share/aclocal $ACLOCAL_FLAGS"

mkdir -p $INSTALL_PREFIX
mkdir -p $INSTALL_PREFIX/share/aclocal

echo "Cloning submodules in $SRC_PREFIX..."
git submodule update --init

echo "Building BABL in $SRC_PREFIX..."
cd $SRC_PREFIX/babl
./autogen.sh --prefix=$INSTALL_PREFIX
make
make install

echo "Building mypaint-brushes in $SRC_PREFIX..."
cd $SRC_PREFIX/mypaint-brushes
./autogen.sh --prefix=$INSTALL_PREFIX
./configure --prefix=$INSTALL_PREFIX
make
make install

echo "Building libmypaint in $SRC_PREFIX..."
cd $SRC_PREFIX/libmypaint
./autogen.sh --prefix=$INSTALL_PREFIX
./configure --prefix=$INSTALL_PREFIX
make
make install

echo "Building GEGL in $SRC_PREFIX..."
cd $SRC_PREFIX/gegl
./autogen.sh --prefix=$INSTALL_PREFIX
make
make install

echo "Building Glimpse in $SRC_PREFIX..."
cd $SRC_PREFIX
./autogen.sh --prefix=$INSTALL_PREFIX
make
make install
