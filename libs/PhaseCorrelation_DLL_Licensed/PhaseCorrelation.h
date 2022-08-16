/*******************************************************************************
*
* $Workfile:   PhaseCorrelation.h$
* $Revision:   $
*
* Title                 : Phase Correlation
* Author(s)             : Hongshi Yan
* Project               : For DSTL
* Classification        : Unclassified
* Reference document    :
*
* Copyright(s)          :
* Copyright(s)
*
*
********************************************************************************
*
* Related Documents
*
* PAF                              N/A
* S/W design specification         N/A
* HW/SW I/F specification          N/A
* S/W test specification           N/A
*
********************************************************************************
* Description
* -----------
*
* Header file for the Phase Correlation DLL
* 
********************************************************************************
* Change Record
* -------------
* Date          Name            Comments
* 25-08-2011    hongshi.yan     Initial version for ADROIT
* 01-12-2013    hongshi.yan     Initial version for DSTL
********************************************************************************
* Version History
* ---------------
* $Modtime:   $
* $Log:   $
*
*******************************************************************************/


#ifndef _PHASECORRELATION_H_
#define _PHASECORRELATION_H_

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PHASECORRELATION_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PHASECORRELATION_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef PHASECORRELATION_EXPORTS
#define PHASECORRELATION_API __declspec(dllexport)
#else
#define PHASECORRELATION_API __declspec(dllimport)
#endif

#define TIGER_INTERFACE_ONLY

/*********************************************************/
/* INCLUDE FILES */
/*********************************************************/

#include "PhaseCorrelationGlobalDefs.h"   // for definition of ROBUST_METHOD


/*********************************************************/
/* DEFINITIONS */
/*********************************************************/


/*********************************************************/
/* TYPE DEFINITIONS */
/*********************************************************/


/*********************************************************/
/* VARIABLES */
/*********************************************************/


/*********************************************************/
/* FUNCTION PROTOTYPES */
/*********************************************************/


PHASECORRELATION_API int Disparity_Map_Generation_PROC(
        float *image_ref,
        float *image_tgt,
        int image_width,
        int image_height,
        int pyramid_level,//pyramid_level=1 to 20, if pyramid_level too large, then the algorithm can automatically choose a valid pyramid_level based on the image size
        PHASE_CORRELATION_METHOD PhaseCorrelationMethod, // Phase_Correlation_Method=CurveFit1, CurveFit2, Robust2DFit1, Robust2DFit2, default parameter is CurveFit1
        int winSize,//local phase correlation scanning window size, the default setting is 16, if the result not good, try winSize=32.
        int step,//phase correlation scanning jump step for speed up caculation. it can be set as large as 5 for lower accuracy Of DEM display
        int filter1_size,//inter-pyramid-level filter size, it can be set from 0 to 15, the default value is 7.
        int filter2_size,//final refined filter size, it can be set from 0 to 15, the default value is 2.
        /* outputs */
        double *v_x, // shift in x direction
        double *v_y  // shift in y direction
);


PHASECORRELATION_API int DEM_Map_Generation_PROC(
        double *disparityMapData, // in case passing v_y instead?
        int sizeR,          // height of disparity map
        int sizeC,          // width of disparity map
        double flightAltitude,      // flight Altitude above the ground in metres
        double cameraBaseline, // difference in position between the two cameras in metres
        double stereoImageResolution, // how many metres a single pixel represents
        double *demMapData      // output
);


/*********************************************************/
/* CODE */
/*********************************************************/


#endif // #ifndef _PHASECORRELATION_H_

/*********************************************************/
