//autoprimordia.h

/*********************************************************************/
/* This source code file is part of PRIMoRDiA software project created 
 * by Igor Barden Grillo at Federal University of Paraíba. 
 * barden.igor@gmail.com ( Personal e-mail ) 
 * igor.grillo@acad.pucrs.br ( Academic e-mail )
 * quantum-chem.pro.br ( group site )
 * IgorChem ( Git Hub account )
 */ 

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
 
/*********************************************************************/

#ifndef AUTO_PRIMORDIA 
#define AUTO_PRIMORDIA

#include <vector>
#include "../include/ReactionAnalysis.h"

class primordia;
//==========================================
/**
 * Class to automate the use of primordia class to calculate the RD from a collection of files in a folder.
 * The Files to be used in the calculations is indicated in a input-like file with structure 
 * @class AutoPrimordia
 * @author Igor Barden Grillo
 * @date 05/05/18
 * @file primordia.h
 * @brief Automate the reactivity descriptor calculations for a given list of converged QM calculations.
 */
class AutoPrimordia{
	public:
		//member variables
		const char* m_file_list;
		std::string run_type;		
		std::vector<primordia> RDs;	
		ReactionAnalysis trj_info;
		//constructors/destructors
		AutoPrimordia();
		AutoPrimordia(const char* file_list);
		AutoPrimordia(const AutoPrimordia& Apr_rhs) = delete;
		AutoPrimordia& operator=(const AutoPrimordia& Apr_rhs) = delete;
		~AutoPrimordia();
		//member functions
		void init();
		void calculate_rd();
		void calculate_rd_from_traj();
		void reaction_analysis();
		void md_trajectory_analysis();
		void lig_biding();
		void write_global();
};
#endif
//================================================================================
//END OF FILE
//================================================================================
