/***************************************************************************
 *
 * Authors:     Alberto Pascual Montano (pascual@cnb.uam.es)
 *
 * Unidad de  Bioinformatica of Centro Nacional de Biotecnologia , CSIC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or   
 * (at your option) any later version.                                 
 *                                                                     
 * This program is distributed in the hope that it will be useful,     
 * but WITHOUT ANY WARRANTY; without even the implied warranty of      
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       
 * GNU General Public License for more details.                        
 *                                                                     
 * You should have received a copy of the GNU General Public License   
 * along with this program; if not, write to the Free Software         
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA            
 * 02111-1307  USA                                                     
 *                                                                     
 *  All comments concerning this program package may be sent to the    
 *  e-mail address 'xmipp@cnb.uam.es'                                  
 ***************************************************************************/

// To avoid problems with long template names
#pragma warning(disable:4786)

#include <fstream>
#include <Classification/xmippPC.hh>

/* Prototypes -============================================================= */

void Usage (char **argv);

/* Main function -============================================================= */

main(int argc, char** argv) {


/* Input Parameters ======================================================== */

FileName       fn_in;    	// input file
FileName       fn_out;   	// output file
FileName       fn_ein;    	// eigen vectors input file
FileName       fn_evin;    	// eigen values input file
FileName       cb_in = "";   	// Code vectors input file
FileName       tmpN;		// Temporary variable
unsigned       k;               // Dimension of projected subspace  
float          p;               // Percent of explained variance  
bool           met;             // project by k?

/* Parameters ============================================================== */
   try {

       fn_in = get_param(argc, argv, "-din");
       fn_ein = get_param(argc, argv, "-ein");
       fn_evin = get_param(argc, argv, "-evin");

       if (check_param(argc, argv, "-cout"))
          fn_out = get_param(argc, argv, "-cout");
       else {
         Usage(argv); 
	 exit(EXIT_FAILURE);
       } 

       if (check_param(argc, argv, "-k")) {
       		if (check_param(argc, argv, "-p")) {
     			cerr << argv[0] << ": Invalid option. You can not select number of dimensions and percent at the same time" << endl;
     			exit(EXIT_FAILURE);
		}
                k = AtoI(get_param(argc, argv, "-k"));
		met = true;
       }

       if (check_param(argc, argv, "-p")) {
       		if (check_param(argc, argv, "-k")) {
     			cerr << argv[0] << ": Invalid option. You can not select number of dimensions and percent at the same time" << endl;
     			exit(EXIT_FAILURE);
		}
                p = AtoF(get_param(argc, argv, "-p"));
		met = false;
       }
       
       
       
       if (argc == 1) {Usage(argv);}
       
   }
   catch (Xmipp_error XE) {cout << XE; Usage(argv);}


/* Some validations ===================================================== */
  

   if (!met && (p <= 0 || p > 100)) {
     cerr << argv[0] << ": invalid value for percent of explained variance (must be > 0 and <= 100): " << p << endl;
     exit(EXIT_FAILURE);
   }

/* Shows parameters ===================================================== */

   cout << endl << "Parameters used: " << endl;
   cout << "Input data file : " << fn_in << endl;
   cout << "Input eigen vectors file : " << fn_ein << endl;
   cout << "Input eigen values file : " << fn_evin << endl;
   cout << "Output file name : " << fn_out << endl;
   if (met)
   	cout << "Dimension of projected subspace : " << k << endl;
   else 
   	cout << "Percent of explained variance : " << p << endl;


/* Open training vector ================================================= */

  
  ifstream inStream(fn_in.c_str());
  if (!inStream) {
      cerr << argv[0] << ": can't open file " << fn_in << endl;
      exit(EXIT_FAILURE);
  }

  xmippCTVectors ts(0, false);

  cout << endl << "Reading data file " << fn_in << "....." << endl;     
  inStream >> ts;

  if (met) { 
   if (k > ts.theItems[0].size() || k <= 0) {
     cerr << argv[0] << ": invalid value for dimension of projected subspace (must be > 0 and <= dimension of the data): " << k << endl;
     exit(EXIT_FAILURE);
   }
  }

/* Open eigen vectors================================================= */

  
  ifstream einStream(fn_ein.c_str());
  if (!einStream) {
      cerr << argv[0] << ": can't open file " << fn_ein << endl;
      exit(EXIT_FAILURE);
  }

  xmippCTVectors ev(0, false);

  cout << "Reading eigen vectors file " << fn_ein << "....." << endl;     
  einStream >> ev;

/* Open eigen values================================================= */

  ifstream evinStream(fn_evin.c_str());
  if (!evinStream) {
      cerr << argv[0] << ": can't open file " << fn_evin << endl;
      exit(EXIT_FAILURE);
  }

  xmippCTVectors eval(0, false);

  cout << "Reading eigen values file " << fn_evin << "....." << endl;     
  evinStream >> eval;

/* Real stuff ============================================================== */

 if (!met) {
 // Calculate the number of components that satisfy p% of the variance
   double cum = 0; int bestIndex;
   for (int i = 0; i < eval.size(); i++) {
      cum += eval.theItems[i][2]*100.0;
      if (cum >= p) {
        bestIndex = i;
	p = cum; 
        break;
      }
   }
   k = bestIndex + 1;
   cout << "Explained " << p << "% of the variance with " << k << " components" << endl;
 } else {
 // Calculate the percent of variance explained by k components
   double cum = 0;
   for (int i = 0; i < k; i++)
      cum += eval.theItems[i][2]*100.0;
   p = cum;   
   cout << "The " << k << " components explain " << p << "% of the variance." << endl;
 }
 
 try {

   // Do the projection

   cout << "projecting into " << k << " dimensions..." << endl; 
  int size=ts.size();
  int dim = ts.itemAt(0).size();
   
  xmippCTVectors projectedTs(0, true);
  projectedTs.theItems.resize(size);
  projectedTs.theTargets.resize(size);
  for (int h = 0; h < size; h++) projectedTs.theItems[h].resize(k, 0); 

    for (int i = 0; i < k; i++) {
       for (int z = 0; z < size; z++) {
         double cum = 0;
         for (int j = 0; j < dim; j++) {
           cum += ev.theItems[i][j]*ts.theItems[z][j];  
         } // j
         projectedTs.theItems[z][i] = cum; 
       }  // z
    } // i
    
  /******************************************************* 
      Saving all kind of Information 
  *******************************************************/

   cout << "Saving projected vectors as " << fn_out << ".dat ....." << endl;  
   tmpN = fn_out.c_str() + (string) ".dat"; 
   ofstream projS(tmpN.c_str());
   projS << k << " " << size << endl;
   for (int j = 0; j < size ; j++) {
     for (int i = 0; i < k; i++)
       projS << " " << projectedTs.theItems[j][i];
     projS << " " << ts.theTargets[j] << endl;  
   }
   projS.flush();    

   cout << "Saving algorithm information as " << fn_out << ".inf ....." << endl;  
   tmpN = fn_out.c_str() + (string) ".inf"; 
   ofstream infS(tmpN.c_str());
   infS << "PCA Projection" << endl << endl;
   infS << "Input data file : " << fn_in << endl;
   infS << "Projected vectors output file : " << fn_out <<  ".dat" << endl;
   infS << "Algorithm information output file : " << fn_out <<  ".inf" << endl;
   infS << "Number of feature vectors: " << ts.size() << endl;
   infS << "Number of variables: " << ts.itemAt(0).size() << endl;
   if (met) {
   	infS << "Dimension of projected subspace : " << k << endl;
   	infS << "Percent of explained variance : " << p << endl;
   } else {
   	infS << "Percent of explained variance : " << p << endl;
   	infS << "Number of final components : " << k << endl;
   }
   infS.flush();    

   cout << endl;
   
 } catch ( Xmipp_error &e ) {
    cout << e << endl;
 }
  return 0;
}


/* ------------------------------------------------------------------------- */
/* Help Message for this Program                                             */
/* ------------------------------------------------------------------------- */
void Usage (char **argv) {
  printf (
     "\nUsage: %s [Purpose and Parameters]"
     "\nPurpose: Project data into the first k Principal Components (PCA projection)"
     "\n"           
     "\nParameter Values: (note space before value)"
     "\n"
     "\n    -din    file_in           Input data file (plain data)"
     "\n    -ein    file_in           Eigen vectors file"
     "\n    -evin   file_in           Eigen values file"
     "\n    -cout   file_out          Base name for output data files"
     "\n    -k      k                 Number of components"
     "\n    -p      p                 Percent of the explained variance"
     "\n			   \n"
     ,argv[0]);
     exit(0);
}

