/*****************************************************************************
 *    Header File:  B0000065.h
 *
 *    Description:  Clear Variable Numerator Work File Header File
 *
 *        History:
 *          Date        Programmer  SAR# - Description
 *          ----------  ----------  -------------------------------------------
 *   Author 07/02/2008  J.Tomioka                       - Created  
 *
 *
 * Copyright (c) 2008  J.D. Edwards & Company
 *
 * This unpublished material is proprietary to J.D. Edwards & Company.
 * All rights reserved.  The methods and techniques described herein are
 * considered trade secrets and/or confidential.  Reproduction or
 * distribution, in whole or in part, is forbidden except by express
 * written permission of J.D. Edwards & Company.
 ****************************************************************************/

#ifndef __b0000065_H
#define __b0000065_H

/*****************************************
 * TYPEDEF for Data Structure
 *    Template Name: Get Next Number
 *    Template ID:   D0000065
 *    Generated:     Wed Jul 02 11:55:20 2008
 *
 * DO NOT EDIT THE FOLLOWING TYPEDEF
 *    To make modifications, use the OneWorld Data Structure
 *    Tool to Generate a revised version, and paste from
 *    the clipboard.
 *
 **************************************/

#ifndef DATASTRUCTURE_D0000065
#define DATASTRUCTURE_D0000065

typedef struct tagDSD0000065A
{
  JCHAR             szSystemCode[5];                     
  MATH_NUMERIC      mnNextNumberingIndexNo;              
  JCHAR             szCompanyKey[6];                     
  JCHAR             szDocumentType[3];                   
  MATH_NUMERIC      mnCentury;                           
  MATH_NUMERIC      mnFiscalYear1;                       
  JCHAR             szSameAsDocumentType[3];             
  MATH_NUMERIC      mnNextNumber001;                     
} DSD0000065A, *LPDSD0000065A;
 
#define IDERRszSystemCode_1                       1L
#define IDERRmnNextNumberingIndexNo_2             2L
#define IDERRszCompanyKey_3                       3L
#define IDERRszDocumentType_4                     4L
#define IDERRmnCentury_5                          5L
#define IDERRmnFiscalYear1_6                      6L
#define IDERRszSameAsDocumentType_7               7L
#define IDERRmnNextNumber001_8                    8L

#endif

#endif    /* __B0000065_H */
