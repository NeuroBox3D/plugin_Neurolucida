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
					.add_method("print_setup",  (void (TNeurolucida::*)())&TNeurolucida::print_setup);
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
