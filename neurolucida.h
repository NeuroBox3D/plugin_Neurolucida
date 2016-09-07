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
 * \file neurolucida.h
 *
 * TODO:
 * 			   1. dont duplicate vertices... can be done without duplicating them
 * 			   2. dont store edges maybe? as we dont need them really we can omit storing them
 * 			   3. document and cleanup this code -> move to NETI finally if new UG available
 *
 *  Created on: Jan 6, 2016
 *      Author: Stephan Grein
 */

#ifndef __H__UG__NEUEROLUCIDA__NEUROLUCIDA__
#define __H__UG__NEUEROLUCIDA__NEUROLUCIDA__

#include <string>

#include <common/parser/rapidxml/rapidxml.hpp>
#include <common/math/ugmath.h>

#include "lib_grid/lib_grid.h"
#include "lib_grid/algorithms/remove_duplicates_util.h"
#include "lib_grid/attachments/attachment_info_traits.h"
#include "lib_grid/attachments/attachment_io_traits.h"
#include "lib_grid/global_attachments.h"

namespace ug {
	namespace neurolucida {
		class Neurolucida {
		private:
			rapidxml::xml_document<> m_doc;
			size_t m_subsetCount;
			size_t m_somaIndex;

			ug::Grid* m_g;
			ug::SubsetHandler* m_s;
			ug::Grid::VertexAttachmentAccessor<ug::APosition3> m_aaPos;
			ug::Grid::VertexAttachmentAccessor<ANumber> m_aaDiameter;
			Attachment<number> m_aDiameter;

			static number REMOVE_DOUBLE_THRESHOLD;
			static std::string UGX_EXTENSION;
			static std::string OBJ_EXTENSION;
			static int DEFAULT_SUBSET_COLOR;

			bool m_bConvertToUGX;
			bool m_bConvertToOBJ;
			bool m_bSomaAvailable;
			bool m_bVRLOutputNames;

			ug::MathVector<4> m_defaultSubsetColor;
			std::string m_outputName;
			std::string m_separator; ///<! output separator for subset names
			number m_scaling; ///<! assumes micrometer and scales to meter

			/*!
			 * \brief NL contour
			 */
			struct Contour {
				std::vector<MathVector<4> > points;
				std::string name;
				std::string color;
				bool closed;

				Contour() : name("N/A"),
							color(""),
							closed(false) {
				}
			};

			/*!
			 * \brief NL edge
			 */
			struct CEdge {
				ug::MathVector<4> from;
				ug::MathVector<4> to;
			};

			/*!
			 * \brief NL tree
			 */
			struct Tree {
				std::vector<MathVector<4> > points;
				std::string type;
				std::string leaf;
				std::string color;
				std::vector<CEdge> edges;

				Tree() : type("N/A"),
						 leaf("N/A"),
						 color("") {
				}
			};

		public:
			/*!
			 * \brief default ctor
			 */
			Neurolucida() : m_subsetCount(0),
							m_somaIndex(0),
							m_g(new ug::Grid()),
							m_s(new SubsetHandler(*m_g)),
							m_bConvertToUGX(false),
							m_bConvertToOBJ(false),
							m_bSomaAvailable(false),
							m_bVRLOutputNames(true),
							m_scaling(1e-6) {

					if (!m_g->has_vertex_attachment(ug::aPosition)) {
						m_g->attach_to_vertices(ug::aPosition);
					}

					m_aaPos = ug::Grid::VertexAttachmentAccessor<ug::APosition3>(*m_g, ug::aPosition);

					if (!m_g->has_vertex_attachment(m_aDiameter)) {
						m_aDiameter = GlobalAttachments::attachment<ANumber>("diameter");
						m_g->attach_to_vertices(m_aDiameter);
					}

					m_aaDiameter = ug::Grid::VertexAttachmentAccessor<ANumber>(*m_g, m_aDiameter);

					for (size_t i = 0; i < 3; i++) {
						m_defaultSubsetColor.coord(i) = GetColorFromStandardPalette(DEFAULT_SUBSET_COLOR).coord(i);
					}

					m_defaultSubsetColor.coord(3) = 1.f;

					m_separator = "";

			}

			/*!
			 * \brief default dtor
			 */
			~Neurolucida() {
				delete m_s;
				delete m_g;
			}

		protected:
			void process_trees() {
				rapidxml::xml_node<>* rootNode = m_doc.first_node();
				std::vector<Tree> trees;

				if (rootNode) {
					if (strcmp(rootNode->name(), "mbf") != 0) {
						UG_LOG("XML file in wrong format, or no Neurolucida XML file provided!");
					} else {
						rapidxml::xml_node<>* treeData = rootNode->first_node("tree");
						while (treeData) {
							Tree t;
							rapidxml::xml_node<>* pointData = treeData->first_node("point");
							while (pointData) {
								number x = atof(pointData->first_attribute("x")->value());
								number y = atof(pointData->first_attribute("y")->value());
								number z = atof(pointData->first_attribute("z")->value());
								number d = atof(pointData->first_attribute("d")->value());
								MathVector<4> point(x, y, z, d);
								t.points.push_back(point);
								pointData = pointData->next_sibling("point");
								UG_LOGN("x: " << x << ", " << "y: " << y << ", z: " << z << ", d:" << d);
							}

							for (size_t i = 0; i < t.points.size()-1; i++) {
								CEdge e;
								e.from = t.points[i];
								e.to = t.points[i+1];
								t.edges.push_back(e);
							}

							rapidxml::xml_node<>* branchData = treeData->first_node("branch");
							MathVector<4> point_before_branch = t.points[t.points.size()-1];

							if (branchData) {
								/// process branches of given tree
								UG_LOGN("tree with type: '" << treeData->first_attribute("type")->value() << "' has branches!");
								t.color = treeData->first_attribute("color")->value();
								t.leaf = treeData->first_attribute("leaf")->value();
								t.type = treeData->first_attribute("type")->value();
								process_branch(t, branchData, point_before_branch);
							} else {
								/// process branches in case we have no branches
								UG_LOGN("tree with type: '" << treeData->first_attribute("type")->value() << "' has no branches!");
								t.color = treeData->first_attribute("color")->value();
								t.leaf = treeData->first_attribute("leaf")->value();
								t.type = treeData->first_attribute("type")->value();
								process_branch(t, branchData, point_before_branch);
							}

							/// store the tree information
							trees.push_back(t);

							/// and next tree
							treeData = treeData->next_sibling("tree");
						}
					}
				} else {
					UG_LOGN("Error during parsing XML document.")
				}

				size_t treeIndex = 1;
				UG_LOGN("#trees: " << trees.size());
				std::vector<Tree>::const_iterator it = trees.begin();

				/// create vertices
				for (; it != trees.end(); ++it) {
					std::vector<MathVector<4> >::const_iterator it2 = it->points.begin();
					UG_LOGN("#points of tree: " << it->points.size());
					for (;it2 != it->points.end();) {
						ug::RegularVertex* vtx =  *(m_g->create<ug::RegularVertex>());
						m_aaPos[vtx] = ug::vector3(it2->coord(0) * m_scaling, it2->coord(1) * m_scaling, it2->coord(2) * m_scaling);
						m_aaDiameter[vtx] = it2->coord(3) * m_scaling;
						m_s->assign_subset(vtx, treeIndex+m_subsetCount);
						++it2;
					}
					std::stringstream ss;
					if (!m_bVRLOutputNames) {
						ss << "Tree" << m_separator << treeIndex << ":" << m_separator << "'" << it->type << "'" << m_separator << "(" << "Leaf:" << m_separator << "'" << it->leaf << "')";
					} else {
						std::string str = it->type;
						str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
						ss << "Tree" << "_" << treeIndex << "_" << str << "_" << it->leaf;
					}
					m_s->subset_info(treeIndex+m_subsetCount).name = ss.str();
					treeIndex++;
				}

				/// create edges
				treeIndex = 1;
				it = trees.begin();
				for (; it != trees.end(); ++it) {
					typedef std::vector<CEdge>::const_iterator IT2;
					IT2 it2;
					it2 = it->edges.begin();
					UG_LOGN("#edges of tree:" << it->edges.size());
					for (; it2 != it->edges.end(); ++it2) {
						ug::RegularVertex* vtx =  *(m_g->create<ug::RegularVertex>());
						ug::RegularVertex* vtx2 =  *(m_g->create<ug::RegularVertex>());
						const CEdge* e = &(*it2);
						m_aaPos[vtx] = ug::vector3(m_scaling * e->from.coord(0), m_scaling * e->from.coord(1), m_scaling * e->from.coord(2));
						m_aaPos[vtx2] = ug::vector3(m_scaling * e->to.coord(0), m_scaling * e->to.coord(1), m_scaling * e->to.coord(2));
						ug::RegularEdge* edge = (*m_g->create<ug::RegularEdge>(EdgeDescriptor(vtx, vtx2)));
						m_s->assign_subset(edge, treeIndex+m_subsetCount);
					}

					ug::MathVector<4> color;
					get_subset_color(it->color, color);
					UG_LOGN("it->color: " << it->color);
					m_s->subset_info(treeIndex+m_subsetCount).color = color;
					treeIndex++;
				}

				m_subsetCount = treeIndex + m_subsetCount;
				m_subsetCount--;

				RemoveDoubles<3>(*m_g, m_g->vertices_begin(), m_g->vertices_end(), ug::aPosition, REMOVE_DOUBLE_THRESHOLD);
				EraseEmptySubsets(*m_s);

				if (m_bSomaAvailable) connect_to_soma(trees);
			}

		private:
			void process_branches(Tree& t, std::vector<rapidxml::xml_node<>*> branchData, const MathVector<4>& point_before_branch) {
				std::vector<rapidxml::xml_node<>*>::const_iterator it = branchData.begin();
				std::vector<CEdge> edges;
				for (; it != branchData.end(); ++it) {
					std::vector<MathVector<4> > local_points;
					rapidxml::xml_node<>* pointData = (*it)->first_node("point");
					while (pointData) {
						number x = atof(pointData->first_attribute("x")->value());
						number y = atof(pointData->first_attribute("y")->value());
						number z = atof(pointData->first_attribute("z")->value());
						number d = atof(pointData->first_attribute("d")->value());
						MathVector<4> point(x, y, z, d);
						UG_LOGN("x: " << x << ", " << "y: " << y << ", z: " << z << ", d:" << d);
						t.points.push_back(point);
						local_points.push_back(point);
						pointData = pointData->next_sibling("point");
					}

					if (local_points.size() >= 1) {
						CEdge e;
						e.from = point_before_branch;
						e.to = local_points[0];
						edges.push_back(e);
					}

					for (size_t i = 0; i < local_points.size()-1; i++) {
						CEdge e;
						e.from = local_points[i];
						e.to = local_points[i+1];
						edges.push_back(e);
					}

					// append edges
					std::vector<CEdge> edges_new;
 					edges_new.insert(edges_new.end(), t.edges.begin(), t.edges.end());
					edges_new.insert(edges_new.end(), edges.begin(), edges.end());
					t.edges.clear();
					t.edges.insert(t.edges.end(), edges_new.begin(), edges_new.end());

					ug::MathVector<4> point_before_branch_new;
					if (local_points.size() == 0) {
						point_before_branch_new = point_before_branch;
					} else if (local_points.size() == 1) {
						point_before_branch_new = local_points[0];
					} else {
						point_before_branch_new = edges[edges.size()-1].to;
					}

					if ((*it)->first_node("branch")) {
						UG_LOGN("Branch does contain a branch!");
						rapidxml::xml_node<>* branchLocal = (*it)->first_node("branch");
						std::vector<rapidxml::xml_node<>*> branches;
						while (branchLocal) {
							branches.push_back(branchLocal);
							branchLocal = branchLocal->next_sibling("branch");
						}
						process_branches(t, branches, point_before_branch_new);
					} else {
						UG_LOGN("Branch does not contain a branch!");
					}
				}
			}

			void process_branch(Tree& t, rapidxml::xml_node<>* branchData, const MathVector<4>& point_before_branch) {
				std::vector<rapidxml::xml_node<>*> branches;
				if (!branchData) {
					UG_LOGN("No branches, only points (or recursion end)!");
					/// TODO this could be wrong! process_branches(t, branches, point_before_branch); assure correctness
					return;
				} else {
					UG_LOGN("Tree has branches!");
					while (branchData) {
						branches.push_back(branchData);
						branchData = branchData->next_sibling("branch");
					}
					process_branches(t, branches, point_before_branch);
				}
			}


		protected:
			void process_contours() {
				std::vector<Contour> contours;
				rapidxml::xml_node<>* rootNode = m_doc.first_node();

				if (rootNode) {
					if (strcmp(rootNode->name(), "mbf") != 0) {
						UG_LOG("XML file in wrong format, or no Neurolucida XML file provided!");
					} else {
						rapidxml::xml_node<>* contourData = rootNode->first_node("contour");
						while (contourData) {
							rapidxml::xml_node<>* pointData = contourData->first_node("point");

							Contour contour;
							contour.name = contourData->first_attribute("name")->value();
							contour.closed = strcmp(contourData->first_attribute("closed")->value(), "false") != 0;
							contour.color = contourData->first_attribute("color")->value();
							UG_LOGN("Contour name: '" << contour.name << "'");

							while (pointData) {
						        number x = atof(pointData->first_attribute("x")->value());
						        number y = atof(pointData->first_attribute("y")->value());
						        number z = atof(pointData->first_attribute("z")->value());
						        number d = atof(pointData->first_attribute("d")->value());
						        MathVector<4> point(x, y, z, d);
						        UG_LOGN("x: " << x << ", " << "y: " << y << ", z: " << z << ", d: " << d);
						        contour.points.push_back(point);
						        pointData = pointData->next_sibling("point");

							}
							contours.push_back(contour);
							contourData = contourData->next_sibling("contour");
						}
					}
				} else {
					UG_LOGN("Error during parsing XML document.")
				}

				std::vector<Contour>::const_iterator it = contours.begin();
				size_t contourIndex = 1;
				for (; it != contours.end(); ++it) {
					if (strcmp(it->name.c_str(), "Cell Body") == 0) {
						m_somaIndex = contourIndex-1; // subsets start counting at 0
						m_bSomaAvailable = true;
					}

					std::stringstream ss;
					if (!m_bVRLOutputNames) {
						ss << "Contour" << m_separator << contourIndex << ":" << m_separator << "'" << it->name << "'" << m_separator << "(Closed:" << m_separator << "'" << std::boolalpha << it->closed << "')";
					} else {
						std::string str = it->name;
						str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
						ss << "Contour" << "_" << contourIndex << "_" << str << "_" << std::boolalpha << it->closed;
					}

					m_s->subset_info(contourIndex).name = ss.str();
					ug::MathVector<4> color;
					get_subset_color(it->color, color);
					m_s->subset_info(contourIndex).color = color;

					/// if scaled, it may happen that the first point equals the last point
					/// in this case if the contour should be non-closed will be closed
					/// because the next to last point gets connected with the first
					/// resulting in connectivity
					size_t no_points = it->points.size();
					if (!it->closed)
						no_points--;

					for (size_t i = 0; i < no_points-1; i++) {
						ug::RegularVertex* vtx =  *(m_g->create<ug::RegularVertex>());
						m_aaPos[vtx] = ug::vector3(m_scaling * it->points[i].coord(0), m_scaling * it->points[i].coord(1), m_scaling * it->points[i].coord(2));
						m_aaDiameter[vtx] = it->points[i].coord(3) * m_scaling;
						m_s->assign_subset(vtx, contourIndex);

						ug::RegularVertex* vtx2 =  *(m_g->create<ug::RegularVertex>());
						m_aaPos[vtx2] = ug::vector3(m_scaling * it->points[i+1].coord(0), m_scaling * it->points[i+1].coord(1), m_scaling * it->points[i+1].coord(2));
						m_aaDiameter[vtx2] = it->points[i].coord(3) * m_scaling;

						m_s->assign_subset(vtx, contourIndex);
						m_s->assign_subset(vtx2, contourIndex);

						ug::RegularEdge* edge = (*m_g->create<ug::RegularEdge>(EdgeDescriptor(vtx, vtx2)));
						m_s->assign_subset(edge, contourIndex);

					}
					contourIndex++;
				}

				m_subsetCount = contourIndex;
				m_subsetCount--;

				RemoveDoubles<3>(*m_g, m_g->vertices_begin(), m_g->vertices_end(), ug::aPosition, REMOVE_DOUBLE_THRESHOLD);
				EraseEmptySubsets(*m_s);
			}

		private:
			void connect_to_soma(const std::vector<Tree>& trees) {
				size_t treeIndex = 1;
				std::vector<Tree>::const_iterator it = trees.begin();
				for (; it != trees.end(); ++it) {
					/// find vertices of edge
					ug::vector3 temp (m_scaling * it->edges[0].from.coord(0), m_scaling * it->edges[0].from.coord(1), m_scaling * it->edges[0].from.coord(2));
					ug::Vertex* closest = FindClosestByCoordinate<ug::Vertex>(temp, m_s->begin<ug::Vertex>(m_somaIndex), m_s->end<ug::Vertex>(m_somaIndex), m_aaPos);
					ug::RegularVertex* vtx =  *(m_g->create<ug::RegularVertex>()); /// RegularVertex* needs to be used in every case, Vertex* cannot be used and will result in faulty code
					m_aaDiameter[vtx] = it->edges[0].from.coord(3) * m_scaling;
					m_aaPos[vtx] = temp;

					/// create edge
					ug::RegularEdge* edge = (*m_g->create<ug::RegularEdge>(EdgeDescriptor(vtx, closest)));
					UG_LOGN("closest (to soma): " << m_aaPos[closest]);
					UG_LOGN("temp (from): " << temp);
					UG_LOGN("subset count: " << m_subsetCount);
					UG_LOGN("#trees: " << trees.size());
					m_s->assign_subset(edge, m_subsetCount - trees.size() - 1 + treeIndex);
					treeIndex++;
				}

				RemoveDoubles<3>(*m_g, m_g->vertices_begin(), m_g->vertices_end(), ug::aPosition, REMOVE_DOUBLE_THRESHOLD);
				EraseEmptySubsets(*m_s);
			}

		public:
			void print_setup() const {
				std::cout << "Neurolucida conversion settings:" << std::endl;
				std::cout << "\tScaling: '" << m_scaling << "'" << std::endl;
				std::cout << "\tSeparator: '" << m_separator << "'" << std::endl;
				std::cout << "\tVRL Output Names: '" << std::boolalpha << m_bVRLOutputNames << "'" << std::endl;
				std::cout << "\tREMOVE_DOUBLES_TRESHOLD: '" << REMOVE_DOUBLE_THRESHOLD << "'" << std::endl;
				std::cout << std::endl;
			}

			inline void set_VRLOutputNames(bool VRLOutputNames) {
				m_bVRLOutputNames = VRLOutputNames;
			}

			inline bool get_VRLOuputNames() const {
				return m_bVRLOutputNames;
			}

			inline void set_scaling(number scaling) {
				m_scaling = scaling;
			}

			inline number get_scaling() const {
				return m_scaling;
			}

			inline void set_separator(const std::string& separator) {
				m_separator = separator;
			}

			inline std::string get_separator() const {
				return m_separator;
			}

			inline void set_convert_to_obj(bool objOutput) {
				m_bConvertToOBJ = objOutput;

			}

			inline void set_convert_to_ugx(bool ugxOutput) {
				m_bConvertToUGX = ugxOutput;
			}

			inline bool get_convert_to_obj() const {
				return m_bConvertToOBJ;
			}

			inline bool get_convert_to_ugx() const {
				return m_bConvertToUGX;
			}

			inline void convert(const std::string& filename, bool outputUGX, bool outputOBJ) {
				set_output_name(filename);
				set_convert_to_ugx(outputUGX);
				set_convert_to_obj(outputOBJ);
				parse_file(filename);
			}

			inline void convert(const std::string& filename) {
				set_output_name(filename);
				set_convert_to_ugx(true);
				set_convert_to_obj(false);
				parse_file(filename);
			}

		private:
			/*
			 * \brief returns RGBA color for ProMesh
			 *  Note: A (opacity) not used for now,
			 * 		  also, the color needs to be converted
			 * 		  into the range [0,1] from a RGBA color
			 * 		  in hex or dec in the range of [0, FF] respectively [0,255]
			 */
			void get_subset_color(const std::string& colorString, ug::MathVector<4>& color) {
				 size_t firstChar = colorString.find_first_of("#");
				 /*
				  * if color information from Neurolucida is erroneous:
				  * should be mixed upper/lower case hex value
				  */
				 if (firstChar == std::string::npos) {
					 color = m_defaultSubsetColor;
				 } else {
					 /// use color information from Neurolucida
					 for (size_t i = 0; i < 3; i++) {
						 std::stringstream ss;
						 ss << std::hex << colorString[i*2+1] << colorString[(i+1)*2];
						 int temp;
						 ss >> temp;
						 color.coord(i) = temp / 255.0f;
					 }
				 }
				 color.coord(3) = 1.f;
			}

			/*!
			 * sets the outputname based on the input file name
			 */
			void set_output_name(const std::string& inputFileName) {
				 size_t lastdot = inputFileName.find_last_of(".");
	  		     std::string name;
			     std::string ext;
			     std::stringstream ss;
			     if (lastdot == std::string::npos) name = inputFileName;
			     name = inputFileName.substr(0, lastdot);
			     m_outputName = name;
			}

			inline std::string get_output_name() const {
				return m_outputName;
			}

			void parse_file(const std::string& filename);

			void process_document() {
				/// process contours and trees
				process_contours();
				process_trees();

				/// save grid to obj if demanded
				if (m_bConvertToOBJ) {
					std::stringstream ss;
					ss << m_outputName << OBJ_EXTENSION;
					SaveGridToFile(*m_g, *m_s, ss.str().c_str());
				}

				/// save grid to ugx if demanded
				if (m_bConvertToUGX) {
					std::stringstream ss;
					ss << m_outputName << UGX_EXTENSION;
					SaveGridToFile(*m_g, *m_s, ss.str().c_str());
				}
			}
		};
	}
}

#endif /// __H__UG__NEUEROLUCIDA__NEUROLUCIDA__
