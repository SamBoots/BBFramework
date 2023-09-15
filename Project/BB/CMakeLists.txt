########################################################################
#  This cmakelist handles the base setup for the BB library framework  #
########################################################################
cmake_minimum_required (VERSION 3.8)

# Include sub-projects.
add_subdirectory ("Framework")
add_subdirectory ("UnitTests")