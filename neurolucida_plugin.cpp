/*
 * Copyright (c) 2010-2015:  G-CSC, Goethe University Frankfurt
 * Author: Andreas Vogel
 *
 * This file is part of UG4.
 *
 * UG4 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License version 3 (as published by the
 * Free Software Foundation) with the following additional attribution
 * requirements (according to LGPL/GPL v3 §7):
 *
 * (1) The following notice must be displayed in the Appropriate Legal Notices
 * of covered and combined works: "Based on UG4 (www.ug4.org/license)".
 *
 * (2) The following notice must be displayed at a prominent place in the
 * terminal output of covered works: "Based on UG4 (www.ug4.org/license)".
 *
 * (3) The following bibliography is recommended for citation and must be
 * preserved in all covered files:
 * "Reiter, S., Vogel, A., Heppner, I., Rupp, M., and Wittum, G. A massively
 *   parallel geometric multigrid solver on hierarchically distributed grids.
 *   Computing and visualization in science 16, 4 (2013), 151-164"
 * "Vogel, A., Reiter, S., Rupp, M., Nägel, A., and Wittum, G. UG4 -- a novel
 *   flexible software system for simulating pde based models on high performance
 *   computers. Computing and visualization in science 16, 4 (2013), 165-179"
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 */

/*!
 * \file neurolucida_plugin.cpp
 * \brief plugin registry
 *
 *  Created on: Jan 6, 2016
 *      Author: Stephan Grein
 */

#include "bridge/util.h"
#include "bridge/util_domain_algebra_dependent.h"
#include "common/ug_config.h"
#include "common/error.h"
#include <string>
#include "neurolucida.h"

#include "lib_grid/attachments/attachment_info_traits.h"
#include "lib_grid/attachments/attachment_io_traits.h"
#include "lib_grid/global_attachments.h"

using namespace std;
using namespace ug::bridge;

#ifdef UG_PARALLEL
#include "pcl/pcl_util.h"
#endif

namespace ug {
	namespace neurolucida {
		struct Functionality {
			static void Common(Registry& reg, string grp) {
				typedef neurolucida::Neurolucida TNeurolucida;
				reg.add_class_<TNeurolucida>("Neurolucida", "Neuro/")
					.add_constructor<void (*)()>("")
					.add_method("convert", (void (TNeurolucida::*)(const std::string&))&TNeurolucida::convert)
					.add_method("convert", (void (TNeurolucida::*)(const std::string&, bool, bool))&TNeurolucida::convert)
					.add_method("set_separator", (void (TNeurolucida::*)(const std::string&))&TNeurolucida::set_separator)
					.add_method("set_scaling", (void (TNeurolucida::*)(number))&TNeurolucida::set_scaling)
					.add_method("set_VRLOutputNames", (void (TNeurolucida::*)(bool))&TNeurolucida::set_VRLOutputNames)
					.add_method("print_setup",  (void (TNeurolucida::*)())&TNeurolucida::print_setup)
					.add_method("set_obj_output", (void (TNeurolucida::*)(bool))&TNeuroLucida::set_convert_to_obj)
					.add_method("set_ugx_output", (void (TNeurolucida::*)(bool))&TNeuroLucida::set_convert_to_ugx);
			}
		};
	} /// \}

	extern "C" void InitUGPlugin_Neurolucida(Registry* reg, string grp) {
		grp.append("/Neurolucida");
		typedef neurolucida::Functionality Functionality;

		typedef ANumber ADiameter;
		GlobalAttachments::declare_attachment<ADiameter>("diameter");

		try {
			RegisterCommon<Functionality>(*reg, grp);
		}
		UG_REGISTRY_CATCH_THROW(grp);
	}
}
