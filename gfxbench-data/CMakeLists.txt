cmake_minimum_required(VERSION 3.5)
project(gfxbench-data)

if (NOT OPT_COMMUNITY_BUILD OR BUNDLE_DATA)
  add_subdirectory(../gfxbench-data40 gfxbench40_datadir)
  add_subdirectory(../gfxbench-data50 gfxbench50_datadir)
endif()
