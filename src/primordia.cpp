//include c++ headers 
#include <iostream>
#include <string>
#include <fstream>
//include primordia headers
#include "../include/common.h"
#include "../include/Iatom.h"
#include "../include/Imolecule.h"
#include "../include/Icube.h"
#include "../include/global_rd.h"
#include "../include/local_rd.h"
#include "../include/local_rd_cnd.h"
#include "../include/gridgen.h"
#include "../include/QMparser.h"
#include "../include/primordia.h"
#include "../include/Iprotein.h"
#include "../include/scripts.h"
#include "../include/residue_lrd.h"

using std::string;
using std::cout;
using std::endl;
using std::move;
using std::to_string;
/*************************************************************************************/
primordia::primordia()			:
	name("nonamed")				{
}
/*************************************************************************************/
primordia::primordia(const primordia& pr_rhs)	:
	name(pr_rhs.name)							,
	mol_info(pr_rhs.mol_info)					,
	grd( pr_rhs.grd )							,
	lrdVol( pr_rhs.lrdVol )						,
	lrdCnd( pr_rhs.lrdCnd )						,
	ch_rd( pr_rhs.ch_rd )		 				,
	bio_rd( pr_rhs.bio_rd )						{
}
/***************************************************************************************/
primordia& primordia::operator=(const primordia& pr_rhs){
	if ( this != &pr_rhs ) {
		name	= pr_rhs.name;
		mol_info= pr_rhs.mol_info;
		grd		= pr_rhs.grd;
		lrdVol	= pr_rhs.lrdVol;
		lrdCnd	= pr_rhs.lrdCnd;
		ch_rd	= pr_rhs.ch_rd;
		bio_rd	= pr_rhs.bio_rd;
	}
	return *this;
}
/***************************************************************************************/
primordia::primordia(primordia&& pr_rhs) noexcept:
	name( move(pr_rhs.name) )					,
	mol_info( move(pr_rhs.mol_info) )			,
	grd( move(pr_rhs.grd) )						,
	lrdVol( move(pr_rhs.lrdVol) )				,
	lrdCnd( move(pr_rhs.lrdCnd) )				,
	ch_rd( move(pr_rhs.ch_rd) ) 				,
	bio_rd( move(pr_rhs.bio_rd) )				{
}
/***************************************************************************************/
primordia& primordia::operator=(primordia&& pr_rhs) noexcept{
	if ( this != &pr_rhs ) {
		name	= move(pr_rhs.name);
		mol_info= move(pr_rhs.mol_info);
		grd		= move(pr_rhs.grd);
		lrdVol	= move(pr_rhs.lrdVol);
		lrdCnd	= move(pr_rhs.lrdCnd);
		ch_rd	= move(pr_rhs.ch_rd);
		bio_rd	= move(pr_rhs.bio_rd);
	}
	return *this;
}
/***************************************************************************************/
primordia operator-(const primordia& pr_lhs, const primordia& pr_rhs){
	primordia Result(pr_lhs);
	Result.grd		= pr_lhs.grd	- pr_rhs.grd;
	Result.lrdVol	= pr_lhs.lrdVol	- pr_rhs.lrdVol;
	Result.lrdCnd	= pr_lhs.lrdCnd	- pr_rhs.lrdCnd;
	return Result;
}
/*************************************************************************************/
void primordia::init_FOA(const char* file_neutro,
						int grdN				,
						string loc_hard			,
						bool mep				, 
						string Program			,
						double den)				{

	name = remove_extension(file_neutro);
	//-------------------------------------------------------------------
	// log messages
	m_log->inp_delim(1);
	m_log->input_message("molecule name "+name);
	m_log->input_message("\nParameters for calculating the Reactivity descriptors:\n\t");
	m_log->input_message("Approximation: Frozen Orbital\n\t");
	m_log->input_message("Grid size: "+to_string(grdN) );
	m_log->input_message("\n\tProgram QM output: "+Program);
	m_log->input_message("\n\tlocal hardness method: "+loc_hard+"\n");
	m_log->inp_delim(2);
	//-------------------------------------------------------------------
	
	QMparser qmfile(file_neutro,Program); 
	Imolecule molecule( move ( qmfile.get_molecule() ) ); 
	if ( molecule.name == "empty"){
		m_log->write_warning("Molecular information not used for calculations! Entry "+name);
		m_log->input_message("Skipping reactivity descriptors calculations for: ");
		m_log->input_message(name);
		m_log->input_message("\n");
	}else{
		if ( dos ){
			scripts dos( molecule.name.c_str(), "DOS");
			dos.write_r_dos( molecule.orb_energies );
		}		
		molecule.light_copy(mol_info);
		molecule.mol_density = den;
		grd = global_rd(molecule) ;
		grd.calculate_rd();
		grd.write_rd();
		lrdCnd = local_rd_cnd( move(molecule) );
		lrdCnd.calculate_Fukui();
		lrdCnd.calculate_RD(grd);
		lrdCnd.calculate_Hardness(grd);
		lrdCnd.write_LRD();
		if ( comp_H ){
			ch_rd = comp_hard(grd,lrdCnd,den);
			ch_rd.write_comp_hardness( name.c_str() );
		}
		if ( grdN  > 0 ){
			gridgen grid1( grdN, move(lrdCnd.molecule) );
			Icube homo_cub	= grid1.calc_HOMO_density();
			Icube lumo_cub	= grid1.calc_LUMO_density();
			if ( loc_hard == "mepEE" || loc_hard == "LCP" ) {
				if ( Program == "orca" ) {
					grid1.calculate_density_orca();
				}
				else { 
					grid1.calculate_density();
				}
				Icube dens	= grid1.density;
				lrdVol		= local_rd( dens,homo_cub,lumo_cub );
			}else { 
				lrdVol = local_rd( homo_cub,lumo_cub );
			}		
			lrdVol.calculate_fukui();
			lrdVol.calculate_RD(grd);
			if ( loc_hard !="not" ){ 
				lrdVol.calculate_hardness(grd,loc_hard);
			}
			lrdVol.write_LRD();
			if ( mep ){
				grid1.calculate_mep_from_charges();
				grid1.density.write_cube(grid1.name+"_mep.cube");
				grid1.density.write_cube(grid1.name+"_mep2.cube");
				m_log->input_message("MEP grid calculations required \n");
			}
			if ( pymol_script ) {
				mol_info.write_pdb();
				scripts pymol_s( name,"pymols" );
				pymol_s.write_pymol_cube(lrdVol, true);
			}					
		}		
	}
}
/*************************************************************************************/
void primordia::init_FD(const char* file_neutro			,
							  const char* file_cation	,
							  const char* file_anion	, 
							  const int grdN			, 
							  int charge				,
							  bool mep					,
							  string loc_hard			, 
							  string Program			,
							  double den)				{

	name	= remove_extension(file_neutro);
	
	//-------------------------------------------------------------------
	// log messages
	m_log->input_message("molecule name "+name);
	m_log->input_message("\n\tParameters for calculating the Reactivity descriptors:\n\t");
	m_log->input_message("Approximation: Finite Differences\n\t");
	m_log->input_message("Grid size: "+ to_string(grdN) );
	m_log->input_message("\n\tProgram QM output: "+Program);
	m_log->input_message("\n\tlocal hardness method "+loc_hard);
	m_log->input_message("\n");
	//----------------------------------------------------------------------
	
	QMparser qmfile1 (file_neutro,Program);
	Imolecule molecule_a ( move(qmfile1.get_molecule() ) );
	QMparser qmfile2 (file_cation,Program);
	Imolecule molecule_b ( move(qmfile2.get_molecule() ) );
	QMparser qmfile3 (file_anion,Program);
	Imolecule molecule_c ( move(qmfile3.get_molecule() ) );

	if ( molecule_a.name == "empty"){
		m_log->write_warning("Molecular information not used for calculations! Entry "+name);
		m_log->input_message("Skipping reactivity descriptors calculations for:");
		m_log->input_message(file_neutro);
		m_log->input_message("\n");
		m_log->inp_delim(2);
	}else if ( molecule_b.name == "empty" ) {
		m_log->write_warning("Molecular information not used for calculations! Entry "+name);
		m_log->input_message("Skipping reactivity descriptors calculations for:");
		m_log->input_message(file_cation);
		m_log->input_message("\n");
		m_log->inp_delim(2);
	}else if ( molecule_c.name =="empty" ){
		m_log->write_warning("Molecular information not used for calculations! Entry "+name);
		m_log->input_message("Skipping reactivity descriptors calculations for:");
		m_log->input_message(file_anion);
		m_log->input_message("\n");
		m_log->inp_delim(2);
	}else{
		molecule_a.light_copy(mol_info);
		molecule_a.mol_density = den;
		grd = global_rd(molecule_a,molecule_b,molecule_c);
		grd.calculate_rd();
		grd.write_rd();			
		lrdCnd = local_rd_cnd(molecule_a, molecule_b, molecule_c);	
		lrdCnd.calculate_RD(grd);
		lrdCnd.calculate_Hardness(grd);
		lrdCnd.write_LRD();
		if ( comp_H ){
			ch_rd = comp_hard(grd,lrdCnd,den);
			ch_rd.write_comp_hardness( name.c_str() );
		}
		if ( grdN > 0 ){
			gridgen grid1 ( grdN,move(molecule_a) );
			if ( Program == "orca" ) {
				grid1.calculate_density_orca();
			}
			else {
				grid1.calculate_density();
			}	 
			gridgen grid2 ( grdN,move(molecule_b) );
			if ( Program == "orca" ) {
				grid2.calculate_density_orca();
			}
			else { 
				grid2.calculate_density();
			}	
			gridgen grid3 ( grdN, move(molecule_c) );
			if ( Program == "orca" ) {
				grid3.calculate_density_orca();
			}
			else { 
				grid3.calculate_density();
			}		
			lrdVol = local_rd(grid1.density,grid2.density,grid3.density,charge);
			lrdVol.calculate_fukui();
			lrdVol.calculate_RD(grd);
			lrdVol.calculate_hardness(grd,loc_hard);
			lrdVol.write_LRD();
			if ( mep ){
				grid1.calculate_mep_from_charges();
				grid1.density.write_cube(name+"_mep.cube");
				grid1.density.write_cube(name+"_mep2.cube");
				m_log->input_message("MEP grid calculations required\n");
			}
			if ( pymol_script ) {
				mol_info.write_pdb();
				scripts pymol_s( name,"pymols" );
				pymol_s.write_pymol_cube(lrdVol, true);
			}			
		}
	}
}
/*************************************************************************************/
void primordia::init_protein_RD(const char* file_name	,
								string locHardness		,
								int gridN				,
								int bandgap				,
								double* ref_atom		,
								int size				,
								const char* _pdb		,
								bool mep				,
								string bt				,
								string Program			){
	band		= bandgap;
	int band2	= bandgap;
	name = remove_extension(file_name);
	string band_m = "Energy Weighted";
	if ( bt == "BD") {
		band_m = "Band Density";
	}
	//-------------------------------------------------------------------
	// log messages
	m_log->input_message("molecule name "+name);
	m_log->input_message("\nParameters for calculating the band Reactivity descriptors:");
	m_log->input_message("\n\tApproximation: Frozen Orbital");
	m_log->input_message("\n\tGrid size: "+to_string(gridN) );
	m_log->input_message("\n\tProgram QM output: "+Program);
	m_log->input_message("\n\tlocal hardness method "+locHardness);
	m_log->input_message("\n\tBand Reactivity Descriptors method ");
	m_log->input_message(band_m);
	m_log->input_message("\n\tEnergy criteria "+std::to_string(energy_crit)+" (eV)\n\n" );
	//---------------------------------------------------------------------
	
	QMparser fileQM ( file_name,Program );
	Imolecule molecule( fileQM.get_molecule() );			
	if ( molecule.name == "empty"){
		m_log->write_warning("Molecular information not used for calculations! Entry "+name);
		m_log->input_message("Skipping reactivity descriptors calculations for:");
		m_log->input_message(file_name);
		m_log->inp_delim(1);
	}else{
		molecule.light_copy(mol_info);
		name = remove_extension(file_name);
		Iprotein pdbfile(_pdb);
		if ( dos ){
			scripts dos( molecule.name.c_str(), "DOS" );
			dos.write_r_dos(molecule.orb_energies);
		}
	
		grd		= global_rd( molecule );
		lrdCnd	= local_rd_cnd( move(molecule) );
		lrdCnd.mep	= mep;
		grd.calculate_rd();
	
		if ( band > 0 ) {
			if 	( bt == "EW" )	lrdCnd.calculate_Fukui_EW(band);
			else{ 				lrdCnd.calculate_Fukui_band(band); }
			lrdCnd.calculate_RD(grd);
			lrdCnd.calculate_Hardness(grd);
			bio_rd = lrdCnd.rd_protein(pdbfile);
			lrdCnd.write_rd_protein_pdb(pdbfile);
			lrdCnd.write_LRD();
		}else{
			lrdCnd.calculate_Fukui();
			lrdCnd.calculate_RD(grd);
			lrdCnd.calculate_Hardness(grd);
			bio_rd = lrdCnd.rd_protein(pdbfile);
			lrdCnd.write_rd_protein_pdb(pdbfile);
			lrdCnd.write_LRD();
		}
		if ( comp_H ){
			double dens_tmp = 0;
			ch_rd = comp_hard(grd,lrdCnd,dens_tmp);
			ch_rd.calculate_protein(bio_rd,pdbfile);
			ch_rd.write_comp_hardness( name.c_str() );
		}
		if ( gridN > 0 ){
			gridgen grid ( gridN,move(lrdCnd.molecule) );
			if ( size > 0 ) { 
				grid.redefine_lim(ref_atom[0],ref_atom[1],ref_atom[2],size);
			}
			Icube EAS;
			Icube NAS;
			if ( band  > 0 ) {
				if ( bt == "BD" ){
					EAS  = grid.calc_band_EAS(bandgap);
					NAS  = grid.calc_band_NAS(bandgap);
				}else if ( bt == "EW" ){
					EAS = grid.calc_EBLC_EAS();
					NAS = grid.calc_EBLC_NAS();
				}
				if ( locHardness == "LCP" || locHardness == "mepEE" ){
					grid.calculate_density();
					lrdVol = local_rd(grid.density,EAS,NAS);
				}else{ 
					lrdVol = local_rd(EAS,NAS);
				}
				lrdVol.name = name;
				lrdVol.calculate_fukui();
				lrdVol.calculate_RD(grd);
				lrdVol.calculate_hardness(grd,locHardness);
				lrdVol.write_LRD();
			}else{
				EAS = grid.calc_HOMO_density() ;
				NAS = grid.calc_LUMO_density() ;
				if ( locHardness == "LCP" || locHardness == "mepEE" ){ 
					grid.calculate_density();
					lrdVol = local_rd(grid.density,EAS,NAS);
				}else{ 
					lrdVol = local_rd(EAS,NAS);
				}
				lrdVol.name = name;
				lrdVol.calculate_fukui();
				lrdVol.calculate_RD(grd);
				lrdVol.calculate_hardness(grd,locHardness);
				lrdVol.write_LRD();
			}
			if ( mep ){
				grid.calculate_mep_from_charges();
				grid.density.write_cube(name+"_mep.cube");
				grid.density.write_cube(name+"_mep2.cube");
			}
			if ( pymol_script ){
				scripts pymol_s( name, "pymols" );
				pymol_s.write_pymol_cube(lrdVol, true);
			}
		}else{
			lrdCnd.molecule.clear();
		}
		if ( pymol_script ) { 
			scripts pymol_pdb( name, "pymols_pdb" );
			pymol_pdb.write_pymol_pdb();
		}
	}
}
/***************************************************************************************/
primordia::~primordia(){};
//================================================================================
//END OF FILE
//================================================================================
