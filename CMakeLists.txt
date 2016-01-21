################################################################################
# NEUROLUCIDA
#
# created by Stephan Grein
#
################################################################################

set(pluginName	Neurolucida)
set(SOURCES		neurolucida.cpp neurolucida_plugin.cpp)

cmake_minimum_required(VERSION 2.6)
project(UG_PLUGIN_${pluginName})

# include the definitions and dependencies for ug-plugins.
include(${CMAKE_SOURCE_DIR}/cmake/ug_plugin_includes.cmake)

if(buildEmbeddedPlugins)
	EXPORTSOURCES(${CMAKE_CURRENT_SOURCE_DIR} ${SOURCES})
else(buildEmbeddedPlugins)
	add_library(${pluginName} SHARED ${SOURCES})
	target_link_libraries (${pluginName} ug4)
endif(buildEmbeddedPlugins)