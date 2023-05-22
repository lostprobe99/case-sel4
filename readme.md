# seL4

## 依赖

```sh
# base dependencies
sudo apt-get update
sudo apt-get install build-essential
sudo apt-get install cmake ccache ninja-build cmake-curses-gui
sudo apt-get install libxml2-utils ncurses-dev
sudo apt-get install curl git doxygen device-tree-compiler
sudo apt-get install u-boot-tools

# build dependencies
sudo apt-get install clang gdb
sudo apt-get install libssl-dev libclang-dev libcunit1-dev libsqlite3-dev
sudo apt-get install qemu-kvm

# for ubuntu 18.04
sudo apt-get install python-dev python-pip python3-dev python3-pip
sudo apt-get install protobuf-compiler python-protobuf
sudo apt-get install gcc-8 g++-8
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 800 --slave /usr/bin/g++ g++ /usr/bin/g++-8

# CMake require minimum version is 3.12.0
# for ubuntu 18.04, CMake can be downloaded from: https://cmake.org/download/

# for ubuntu 20.04 and 22.04
sudo apt-get install python3-dev python3-pip python-is-python3
sudo apt-get install protobuf-compiler python3-protobuf

# for manual
sudo apt-get install texlive texlive-latex-extra texlive-fonts-extra

# python
pip3 install --user setuptools
pip3 install --user sel4-deps
# or
pip install --user setuptools
pip install --user sel4-deps
```

## 拉取子模块

```sh
git submodule init
git submodule update
```

## 如何使用

1. 将你的项目放到`projects`目录下
2. 将`project-CMakeLists.txt.template`拷贝到你的项目目录下并重命名为`CMakeLists.txt`
3. 将上一步中的`CMakeLists.txt`中的`set(project_name hello)`中的`hello`改为你项目的名称

## 编译运行

```sh
cmake -G Ninja -C./settings.cmake ./projects/${project_name} -B ${build_directory}
cd ${build_directory}
ninja
./simulate
```

