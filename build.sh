sudo apt-get install -y cmake
# opengl, glew, glfw, glu, glut, gtk2
sudo apt-get install -y libgl1-mesa-dev mesa-common-dev libglu1-mesa libglu1-mesa-dev libegl1-mesa libegl1-mesa-dev libgles2-mesa-dev libglew-dev libglfw3-dev

mkdir build
cmake -B build -S .
cd build
make
