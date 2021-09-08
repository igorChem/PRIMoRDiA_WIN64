//test_p.cpp

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

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstring>
#include <omp.h>


#include "../include/test_p.h"
#include "../include/autoprimordia.h"

//#include <gtest/gtest.h>

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::move;

/************************************************************************************/
test_p::test_p(){
}
/************************************************************************************/
test_p::~test_p(){}
/************************************************************************************/
void test_p::init_general_test(){
	//AutoPrimordia run_test("/home/igorchem/primordia-code/PRIMoRDiA1.0v/data_test/primordia.input");
	AutoPrimordia run_test("/media/igorchem/CCDIR/ccdir/primordia_testes/data_test_1_2/primordia.input");
	run_test.init();
}
/************************************************************************************/
void test_p::unit_test(){
	
}
/************************************************************************************/
void test_p::test_reaction_analysis(){
	AutoPrimordia run_test1d("/media/igorchem/CCDIR/ccdir/primordia_testes/enzyme_tuto/primordia.input");
	//AutoPrimordia run_test1d("/media/igorchem/CCDIR/ccdir/primordia_testes/deHalo_big335_PM7/primordia.input");
	run_test1d.init();
	//AutoPrimordia run_test2d("/home/igorchem/Documents/PM6_16x16/primordia.input");
	//run_test2d.init();
}
/************************************************************************************/
void test_p::test_traj_analysis(){
	
}
//================================================================================
//END OF FILE
//================================================================================