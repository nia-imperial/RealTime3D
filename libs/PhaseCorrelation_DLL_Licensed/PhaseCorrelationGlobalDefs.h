/*******************************************************************************
*
* $Workfile:   Phase_Registration_Global_Defs.h$
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
* Header file used to contain definitions that the PhaseCorrelation DLL
* can reference so that the number of files #included by the file 
* PhaseCorrelation.h can be minimised.
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


#ifndef _PHASE_REGISTRATION_GLOBAL_DEFS_H_
#define _PHASE_REGISTRATION_GLOBAL_DEFS_H_


/*********************************************************/
/* INCLUDE FILES */
/*********************************************************/


/*********************************************************/
/* DEFINITIONS */
/*********************************************************/


/*********************************************************/
/* TYPE DEFINITIONS */
/*********************************************************/

typedef enum {
	CurveFit1,
	CurveFit2,		
	Robust2DFit1,	
	Robust2DFit2		
} PHASE_CORRELATION_METHOD;


/*********************************************************/
/* VARIABLES */
/*********************************************************/


/*********************************************************/
/* FUNCTION PROTOTYPES */
/*********************************************************/


/*********************************************************/
/* CODE */
/*********************************************************/


#endif // #ifndef _PHASE_REGISTRATION_GLOBAL_DEFS_H_

/*********************************************************/
/* END-OF-FILE */