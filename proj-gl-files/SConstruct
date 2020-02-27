import os
env = Environment(ENV = os.environ)

env.Append(LIBS=["png"])
env.Append(CXXFLAGS=["-std=c++11","-g","-Wall","-O3"])
env.Append(LINKFLAGS=[])

env.Program("driver",["main.cpp","parse.cpp","dump_png.cpp","driver_state.cpp","shaders.cpp"])
