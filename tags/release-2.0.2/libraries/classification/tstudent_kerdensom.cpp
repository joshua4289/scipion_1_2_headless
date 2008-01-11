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

//-----------------------------------------------------------------------------
// xmippTStudentKerDenSOM.cc
// Implements Smoothly Distributed Kernel Probability Density Estimator Self-Organizing Map
// Uses a t-student Kernel Function.
//-----------------------------------------------------------------------------

#include <fstream>
#include <ctime>

#include "tstudent_kerdensom.h"

//-----------------------------------------------------------------------------
/**
 * Trains the TStudentKerDenSOM
 * Parameter: _som  The KerDenSom to train
 * Parameter: _ts   The training set
 * Parameter: _update True if uses _som as starting point for training.
 * Parameter: _sigma If update = true, uses this sigma for the training.
 */
void xmippTStudentKerDenSOM::train(xmippFuzzyMap& _som, TS& _examples, FileName& _fn, bool _update, double _sigma)
{
    // Saves algorithm information in fn file
    FileName tmpN1 = _fn.c_str() + (string) ".LCurveInfo";
    FileName tmpN;
    ofstream algoStream(tmpN1.c_str());
    algoStream << "File, Regularization, Functional, Sigma" << endl;

    numNeurons = _som.size();
    numVectors = _examples.size();
    dim = _examples.theItems[0].size();
    tmpV.resize(dim, 0.);
    tmpDens.resize(numNeurons, 0.);
    tmpMap.resize(numNeurons);
    for (int i = 0; i < numNeurons; i++)
        tmpMap[i].resize(dim, 0.);
    tmpD.resize(numNeurons);
    tmpD1.resize(numNeurons);
    double stopError;

    int verbosity = listener->getVerbosity();
    if (verbosity)
        listener->OnReportOperation((string) "\nTraining KerDenSOM....\n");

    // Initialization
    if (verbosity)
        listener->OnReportOperation((string) "\nInitializing....\n");
    if (!_update)
    {
        initU(&_som);
        updateV1(&_som, &_examples);
        sigma = updateSigmaI(&_som, &_examples);
    }
    else
    {
        double alpha;
        sigma = _sigma;
        if (_sigma == 0.0)
        {
            updateU1(&_som, &_examples);
            sigma = updateSigmaI(&_som, &_examples);
        }
        updateU(&_som, &_examples, sigma, alpha);
    }


    double regtmp, tmpregMax, tmpregMin, pen, lkhood;
    tmpregMax = log(reg0);
    tmpregMin = log(reg1);

    if (annSteps == 0) annSteps = 1;
    for (int iter = 0; iter < annSteps; iter++)
    {

        if (verbosity)
        {
            if (annSteps > 1)
            {
                char s[100];
                sprintf(s, "\nTraining Deterministic Annealing step %d of %d....\n", iter + 1, annSteps);
                listener->OnReportOperation((string) s);
            }
            else listener->OnReportOperation((string) "Training ....\n");
        }

        if (annSteps == 1)
            regtmp = reg1;
        else
            regtmp = exp(tmpregMax - iter * (tmpregMax - tmpregMin) / (annSteps - 1));

        stopError = mainIterations(&_som, &_examples, sigma, regtmp);
        if (verbosity)
            listener->OnReportOperation((string) "Calculating cost function....\n");

        double funct = functional(&_examples, &_som, sigma, reg1, lkhood, pen);

        algoStream << iter << ",  " << regtmp << ",  " << funct << ",  " << sigma << endl;


        if (verbosity)
        {
            char s[100];
            sprintf(s, "Code vectors variation: %g\n", stopError);
            listener->OnReportOperation((string) s);
        }

        if (verbosity)
            listener->OnReportOperation((string) "Remaking the memberships....\n");
        reMakeU(&_som, &_examples, sigma);

        if (annSteps > 1)
        {
            // Classifying
            if (verbosity)
                listener->OnReportOperation((string) "Classifying....\n");
            _som.classify(&_examples);

            // Calibrating
            if (verbosity)
                listener->OnReportOperation((string) "Calibrating....\n");
            _som.calibrate(_examples);

            if (_examples.isNormalized())
            {
                if (verbosity)
                    listener->OnReportOperation((string) "Denormalizing code vectors....\n");
                _som.unNormalize(_examples.getNormalizationInfo()); // de-normalize codevectors
            }

            // Saves each codebook (for all iterations)
            tmpN = _fn.c_str() + (string) "_" + integerToString(iter) + (string) ".cod";
            if (verbosity)
                listener->OnReportOperation((string) "Saving code vectors....\n");
            ofstream codS(tmpN.c_str());
            codS << _som;
            codS.flush();

            // save .his file (Histogram)
            if (verbosity)
                listener->OnReportOperation((string) "Saving histogram file....\n");
            tmpN = _fn.c_str() + (string) "_" + integerToString(iter) + (string) ".his";
            ofstream hisStream(tmpN.c_str());
            _som.printHistogram(hisStream);
            hisStream.flush();

            // save .err file (Average Quantization Error)
            if (verbosity)
                listener->OnReportOperation((string) "Saving Average Quantization Error file....\n");
            tmpN = _fn.c_str() + (string) "_" + integerToString(iter) + (string) ".err";
            ofstream errStream(tmpN.c_str());
            _som.printQuantError(errStream);
            errStream.flush();

            // save .vs file to be compatible with SOM_PAK
            if (verbosity)
                listener->OnReportOperation((string) "Saving visual file....\n");
            tmpN = _fn.c_str() + (string) "_" + integerToString(iter) + (string) ".vs";
            ofstream vsStream(tmpN.c_str());
            vsStream << _examples.theItems[0].size() << " " << _som.layout() << " " << _som.width() << " " << _som.height() << " gaussian" << endl;
            for (int i = 0; i < _examples.size(); i++)
            {
                int j = _som.fuzzyWinner(i);
                vsStream << _som.indexToPos(j).first << " " << _som.indexToPos(j).second << " " << _som.memb[i][j] << " " << _examples.theTargets[i] << endl;
            }
            vsStream.flush();

            // save .inf file
            if (verbosity)
                listener->OnReportOperation((string) "Saving inf file....\n");
            tmpN = _fn.c_str() + (string) "_" + integerToString(iter) + (string) ".inf";
            ofstream infS(tmpN.c_str());
            infS << "Kernel Probability Density Estimator SOM algorithm" << endl << endl;
            infS << "Deterministic annealing step " << iter + 1 << " out of " << annSteps << endl;
            infS << "Number of feature vectors: " << _examples.size() << endl;
            infS << "Number of variables: " << _examples.theItems[0].size() << endl;
            infS << "Horizontal dimension (Xdim) = " << _som.width() << endl;
            infS << "Vertical dimension (Ydim) = " << _som.height() << endl;
            if (_examples.isNormalized())
                infS << "Input data normalized" << endl;
            else
                infS << "Input data not normalized" << endl;
            if (_som.layout() == "rect")
                infS << "Rectangular topology " << endl;
            else
                infS << "Hexagonal topology " << endl;
            infS << "t-Student Kernel function" << endl;
            infS << "Degrees of freedom (df) = " << df << endl;
            infS << "Total number of iterations = " << somNSteps << endl;
            infS << "Stopping criteria (eps) = " << epsilon << endl << endl ;

            infS << "Smoothness factor (regularization) = " << regtmp << endl;
            infS << "Functional value = " << funct << endl;
            infS << "Sigma (Kernel width) = " << sigma << endl;
            infS.flush();


            if (_examples.isNormalized())
            {
                if (verbosity)
                    listener->OnReportOperation((string) "Normalizing code vectors....\n");
                _som.Normalize(_examples.getNormalizationInfo());       // normalize code vectors
            }
        } // if annStep

    } // for

    algoStream.flush();

    tmpV.clear();
    tmpDens.clear();
    tmpMap.clear();
    tmpD.clear();
    tmpD1.clear();

};


//-----------------------------------------------------------------------------

/**
 * Remake U (Membership)
 */
void xmippTStudentKerDenSOM::reMakeU(xmippFuzzyMap* _som, const TS* _examples, const double& _sigma)
{

    // Create auxiliar stuff

    unsigned i, j, k;
    double var = 0;
    double auxDist, auxProd;
    double t1, t2, t3;

    t1 = df * _sigma;
    t2 = ((double)(df + dim)) / (-2.0);

    // Update Membership matrix

    for (k = 0; k < numVectors; k++)
    {
        auxProd = 0;
        for (i = 0; i < numNeurons; i ++)
        {
            auxDist = 0;
            for (j = 0; j < dim; j++)
                auxDist += ((double) _examples->theItems[k][j] - (double)_som->theItems[i][j]) * ((double)_examples->theItems[k][j] - (double)_som->theItems[i][j]);
            t3 = (double) pow((double)(1.0 + auxDist / t1), (double) t2);
            if (t3 < MAXZERO) t3 = MAXZERO;
            auxProd += t3;
            tmpD[i] = t3;
        }
        for (j = 0; j < numNeurons; j ++)
            _som->memb[k][j] = (xmippFeature)(tmpD[j] / auxProd);

    } // for k

}


//-----------------------------------------------------------------------------

/**
 * Update the U (Membership)
 */
double xmippTStudentKerDenSOM::updateU(xmippFuzzyMap* _som, const TS* _examples, const double& _sigma, double& _alpha)
{

    // Create auxiliar stuff

    unsigned i, j, k;
    double var = 0;
    double auxDist, auxProd;
    double t1, t2, t3;

    t1 = df * _sigma;
    t2 = ((double)(df + dim)) / (-2.0);
    _alpha = 0;


    // Update Membership matrix

    for (k = 0; k < numVectors; k++)
    {
        auxProd = 0;
        for (i = 0; i < numNeurons; i ++)
        {
            auxDist = 0;
            for (j = 0; j < dim; j++)
            {
                double tmp;
                tmp = ((double) _examples->theItems[k][j] - (double) _som->theItems[i][j]);
                auxDist += tmp * tmp;
            }
            tmpD1[i] = auxDist;
            t3 = (double) pow((double)(1.0 + auxDist / t1), (double) t2);
            if (t3 < MAXZERO) t3 = MAXZERO;
            auxProd += t3;
            tmpD[i] = t3;
        }
        for (j = 0; j < numNeurons; j ++)
        {
            t3 = 1 + tmpD1[j] / t1;
            double tmp =  tmpD[j] / (auxProd * t3);
            _som->memb[k][j] = (xmippFeature) tmp;
            _alpha += tmp * tmpD1[j];
        }

    } // for k


    return 0.0;

}


//-----------------------------------------------------------------------------

// Estimate Sigma part II
double xmippTStudentKerDenSOM::updateSigmaII(xmippFuzzyMap* _som, const TS* _examples, const double& _reg, const double& _alpha)
{
    int vv, cc, j;

    if (_reg == 0) return(double)(_alpha*(df + dim) / (double)(numVectors*dim*df));

    // Computing Sigma (Part II)
    double q = 0.;
    for (cc = 0; cc < numNeurons; cc++)
    {
        _som->localAve(_som->indexToPos(cc), tmpV);
        double t1 = _som->getLayout().numNeig(_som, (SomPos) _som->indexToPos(cc));
        for (j = 0; j < dim; j++)
            q += (t1 * (double)(_som->theItems[cc][j]) - (double)(tmpV[j])) * (double)(_som->theItems[cc][j]);
    }
    return (double)((_alpha + _reg*q)*(df + dim) / (double)(numVectors*dim*df));
}



//-----------------------------------------------------------------------------
/**************** Necessary stuff ***************************************/
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

/**
 * Estimate the PD (Method 1: Using the code vectors)
 */
double xmippTStudentKerDenSOM::codeDens(const xmippFuzzyMap* _som, const xmippVector* _example, double _sigma) const
{
    unsigned j, cc;
    double t = 0, s = 0, d1, e1;
    d1 = df * _sigma;
    e1 = ((double)(df + dim)) / (-2.0);
    for (cc = 0; cc < numNeurons; cc++)
    {
        t = 0;
        for (j = 0; j < dim; j++)
            t += ((*_example)[j] - _som->theItems[cc][j]) * ((*_example)[j] - _som->theItems[cc][j]);
        t = (double) pow((double)(1.0 + t / d1), (double) e1);
        if (t < MAXZERO) t = MAXZERO;
        s += t;
    }
    double tmp = (double) dim / 2.0;
    return (double)(pow((double) _sigma, (double) - tmp)*s / (double)numNeurons);
}


//-----------------------------------------------------------------------------

/**
 * Estimate the PD (Method 2: Using the data)
 */
double xmippTStudentKerDenSOM::dataDens(const TS* _examples, const xmippVector* _example, double _sigma) const
{
    unsigned j, vv;
    double t = 0, s = 0, d1, e1;
    d1 = df * _sigma;
    e1 = ((double)(df + dim)) / (-2.0);
    for (vv = 0; vv < numVectors; vv++)
    {
        t = 0;
        for (j = 0; j < dim; j++)
            t += ((double)((*_example)[j]) - (double)(_examples->theItems[vv][j])) * ((double)((*_example)[j]) - (double)(_examples->theItems[vv][j]));
        t = (double) pow((double)(1.0 + t / d1), (double) e1);
        if (t < MAXZERO) t = MAXZERO;
        s += t;
    }
    double tmp = (double) dim / 2.0;
    return (double)(pow((double) _sigma, (double) - tmp)*s / (double)numVectors);
}


//-----------------------------------------------------------------------------

/**
 * Determines the functional value
 * Returns the likelihood and penalty parts of the functional
 */
double xmippTStudentKerDenSOM::functional(const TS* _examples, const xmippFuzzyMap* _som, double _sigma, double _reg, double& _likelihood, double& _penalty)
{
    unsigned j, vv, cc;
    double t;
    _likelihood = 0;
    for (vv = 0; vv < numVectors; vv++)
    {
        t = codeDens(_som, &(_examples->theItems[vv]), _sigma);
        if (t <= MAXZERO)
            t = MAXZERO;
        _likelihood += log(t);
    }
    _likelihood = -_likelihood;
    _penalty = 0;

    if (_reg != 0)
    {
        for (cc = 0; cc < numNeurons; cc++)
        {
            _som->localAve(_som->indexToPos(cc), tmpV);
            t = _som->getLayout().numNeig(_som, (SomPos) _som->indexToPos(cc));
            for (j = 0; j < dim; j++)
            {
                _penalty += (t * (double)(_som->theItems[cc][j]) - (double)(tmpV[j])) * (double)(_som->theItems[cc][j]);
            }
        }
        _penalty = _reg * _penalty * (df + dim) / (2 * df * _sigma);
    }

    return (_likelihood + _penalty);
}

//-----------------------------------------------------------------------------
