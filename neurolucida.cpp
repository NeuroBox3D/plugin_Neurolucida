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
