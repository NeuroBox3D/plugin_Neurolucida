# Copyright (c) 2012-2013:  G-CSC, Goethe University Frankfurt
# Author: Andreas Vogel
#
# This file is part of UG4.
#
# UG4 is free software: you can redistribute it and/or modify it under the
# terms of the GNU Lesser General Public License version 3 (as published by the
# Free Software Foundation) with the following additional attribution
# requirements (according to LGPL/GPL v3 §7):
#
# (1) The following notice must be displayed in the Appropriate Legal Notices
# of covered and combined works: "Based on UG4 (www.ug4.org/license)".
#
# (2) The following notice must be displayed at a prominent place in the
# terminal output of covered works: "Based on UG4 (www.ug4.org/license)".
#
# (3) The following bibliography is recommended for citation and must be
# preserved in all covered files:
# "Reiter, S., Vogel, A., Heppner, I., Rupp, M., and Wittum, G. A massively
#   parallel geometric multigrid solver on hierarchically distributed grids.
#   Computing and visualization in science 16, 4 (2013), 151-164"
# "Vogel, A., Reiter, S., Rupp, M., Nägel, A., and Wittum, G. UG4 -- a novel
#   flexible software system for simulating pde based models on high performance
#   computers. Computing and visualization in science 16, 4 (2013), 165-179"
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.

/*!
 * \file neurolucida.cpp
 *
 *  Created on: Jan 6, 2016
 *      Author: Stephan Grein
 */

#include "neurolucida.h"

#include <fstream>

using namespace ug::neurolucida;
using namespace std;

number Neurolucida::REMOVE_DOUBLE_THRESHOLD = 1e-6;
std::string Neurolucida::OBJ_EXTENSION = ".obj";
std::string Neurolucida::UGX_EXTENSION = ".ugx";
int Neurolucida::DEFAULT_SUBSET_COLOR = 1; /// RED

void Neurolucida::parse_file(const std::string& filename) {
    ifstream in(filename.c_str(), ios::binary);
    if (!in) return;

    streampos posStart = in.tellg();
    in.seekg(0, ios_base::end);
    streampos posEnd = in.tellg();
    streamsize size = posEnd - posStart;
    in.seekg(posStart);

    char* fileContent = m_doc.allocate_string(0, size + 1);
    in.read(fileContent, size);
    fileContent[size] = 0;
    in.close();
    m_doc.parse<0>(fileContent);

    UG_LOGN("processed document!");
    process_document();
}
