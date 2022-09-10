current_directory=${PWD}
#install core4.5

sudo apt-get install bash bridge-utils ebtables iproute libev-dev python \
    tcl8.5 tk8.5 libtk-img \
    autoconf automake gcc libev-dev make python-dev libreadline-dev pkg-config imagemagick help2man

wget http://downloads.pf.itd.nrl.navy.mil/core/source/archive/core-4.5.tar.gz
mv core-4.5.tar.gz ~/Downloads/
cd ~/Downloads    
tar xzvf core-4.5.tar.gz    
cd core-4.5
./bootstrap.sh
./configure
make
sudo make install
#install standard quagga
sudo apt-get install quagga
#install GNU library
sudo apt-get install libgsl0-dev
sudo apt-get install libgsl0ldbl


#install boost library

wget http://downloads.sourceforge.net/project/boost/boost/1.56.0/boost_1_56_0.tar.gz
mv boost_1_56_0.tar.gz ~/Downloads

cd ~/Downloads

tar xvzf boost_1_56_0.tar.gz

cd boost_1_56_0

./bootstrap.sh --help2man

./bootstrap.sh

sudo ./b2 install

LD_LIBRARY_PATH=/usr/local/lib
export LD_LIBRARY_PATH
cd $current_directory

