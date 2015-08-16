
/***************************************************************************** 
 *    Header File:  B0100095.h 
 * 
 *    Description:  Secure Address Book Personal Data Header File 
 * 
 *        History: 
 *          Date        Programmer  SAR# - Description 
 *          ----------  ----------  ------------------------------------------- 
 *   Author 06/16/2004  NR627       Unknown  - Created  
 * 
 * 
 * Copyright (c) J.D. Edwards World Source Company, 1996 
 * 
 * This unpublished material is proprietary to J.D. Edwards World Source  
 * Company.  All rights reserved.  The methods and techniques described  
 * herein are considered trade secrets and/or confidential.  Reproduction 
 * or distribution, in whole or in part, is forbidden except by express 
 * written permission of J.D. Edwards World Source Company. 
 ****************************************************************************/ 
 
#ifndef __B0100095_H
#define __B0100095_H 

 
/***************************************************************************** 
 * Table Header Inclusions 
 ****************************************************************************/ 
#include <f01138.h>
#include <f0101.h>

/***************************************************************************** 
 * External Business Function Header Inclusions 
 ****************************************************************************/ 
#include <b0100018.h>

/***************************************************************************** 
 * Global Definitions 
 ****************************************************************************/ 
  
/***************************************************************************** 
 * Structure Definitions 
 ****************************************************************************/ 

 typedef struct tagF0101ST
 { 
	MATH_NUMERIC  aban8;                     
    JCHAR          abat1[4];   
 }	F0101ST /*,	FAR *tagF0101ST
			Modificado:	Luis Capriles,		Fecha:	18/10/2010 - Upgrade 810-900.
			*/;
 

 typedef struct tagF0092AN
 { 
	JCHAR           uluser[11];                    
	MATH_NUMERIC   ulan8;  
 }	F0092AN /*,	FAR *tagF0092AN
			Modificado:	Luis Capriles,		Fecha:	18/10/2010 - Upgrade 810-900.
			*/;
 
 
/***************************************************************************** 
 * DS Template Type Definitions 
 ****************************************************************************/ 
/*****************************************
 * TYPEDEF for Data Structure
 *    Template Name: Secure Address Book Personal Data
 *    Template ID:   D0100095
 *    Generated:     Wed Jul 07 12:29:48 2004
 *
 * DO NOT EDIT THE FOLLOWING TYPEDEF
 *    To make modifications, use the OneWorld Data Structure
 *    Tool to Generate a revised version, and paste from
 *    the clipboard.
 *
 **************************************/

#ifndef DATASTRUCTURE_D0100095
#define DATASTRUCTURE_D0100095

typedef struct tagDSD0100095
{
  JCHAR             szUserId[11];                        
  MATH_NUMERIC      mnAddressNumber;                     
  JCHAR             szSearchType[4];                     
  MATH_NUMERIC      mnUsersAddressNumber;                
  JCHAR             cSecureABPersonalDataFlag;           
  JCHAR             szTaxId[21];                         
  JCHAR             szTaxIdAdditional[21];               
  JCHAR             szAddressLine1[41];                  
  JCHAR             szAddressLine2[41];                  
  JCHAR             szAddressLine3[41];                  
  JCHAR             szAddressLine4[41];                  
  JCHAR             szAddressLine5[41];                  
  JCHAR             szAddressLine6[41];                  
  JCHAR             szAddressLine7[41];                  
  JCHAR             szCity[26];                          
  JCHAR             szState[4];                          
  JCHAR             szZipCodePostal[13];                 
  JCHAR             szCountry[4];                        
  JCHAR             szCountyAddress[26];                 
  JCHAR             szPhoneAreaCode1[7];                 
  JCHAR             szPhoneNumber[21];                   
  JCHAR             szEmailAddress[257];                 
  MATH_NUMERIC      mnDayDateOfBirth;                    
  MATH_NUMERIC      mnMonthDateOfBirth;                  
  MATH_NUMERIC      mnYearDateOfBirth;                   
  JCHAR             cGender;                             
  JCHAR             szUserDefinedString1[31];            
  JCHAR             szUserDefinedString2[31];            
  JCHAR             szUserDefinedString3[31];            
  JCHAR             szUserDefinedString4[31];            
  JCHAR             szUserDefinedString5[31];            
  JDEDATE           jdUserDefinedDate;                   
  MATH_NUMERIC      mnUserDefinedMath;                   
  JCHAR             cUserDerfinedChar;                   
  JCHAR             cSecureTaxIDFlag;                    
  JCHAR             cSecureAdditionalTaxIDFlag;          
  JCHAR             cSecureAddressFlag;                  
  JCHAR             cSecurePhoneFlag;                    
  JCHAR             cSecureE_MailAddressFlag;            
  JCHAR             cSecureDateofBirthFlag;              
  JCHAR             cSecureGenderFlag;                   
  JCHAR             cSecureUserDefStringFlag1;           
  JCHAR             cSecureUserDefStringFlag2;           
  JCHAR             cSecureUserDefStringFlag3;           
  JCHAR             cSecureUserDefStringFlag4;           
  JCHAR             cSecureUserDefStringFlag5;           
  JCHAR             cSecureUserDefDateFlag;              
  JCHAR             cSecureUserDefMathNumericF;          
  JCHAR             cSecureUserDefCharacterFlag;         
} DSD0100095, *LPDSD0100095;
 
#define IDERRszUserId_1                           1L
#define IDERRmnAddressNumber_2                    2L
#define IDERRszSearchType_3                       3L
#define IDERRmnUsersAddressNumber_4               4L
#define IDERRcSecureABPersonalDataFlag_5          5L
#define IDERRszTaxId_6                            6L
#define IDERRszTaxIdAdditional_7                  7L
#define IDERRszAddressLine1_8                     8L
#define IDERRszAddressLine2_9                     9L
#define IDERRszAddressLine3_10                    10L
#define IDERRszAddressLine4_11                    11L
#define IDERRszAddressLine5_12                    12L
#define IDERRszAddressLine6_13                    13L
#define IDERRszAddressLine7_14                    14L
#define IDERRszCity_15                            15L
#define IDERRszState_16                           16L
#define IDERRszZipCodePostal_17                   17L
#define IDERRszCountry_18                         18L
#define IDERRszCountyAddress_19                   19L
#define IDERRszPhoneAreaCode1_20                  20L
#define IDERRszPhoneNumber_21                     21L
#define IDERRszEmailAddress_22                    22L
#define IDERRmnDayDateOfBirth_23                  23L
#define IDERRmnMonthDateOfBirth_24                24L
#define IDERRmnYearDateOfBirth_25                 25L
#define IDERRcGender_26                           26L
#define IDERRszUserDefinedString1_27              27L
#define IDERRszUserDefinedString2_28              28L
#define IDERRszUserDefinedString3_29              29L
#define IDERRszUserDefinedString4_30              30L
#define IDERRszUserDefinedString5_31              31L
#define IDERRjdUserDefinedDate_32                 32L
#define IDERRmnUserDefinedMath_33                 33L
#define IDERRcUserDerfinedChar_34                 34L
#define IDERRcSecureTaxIDFlag_35                  35L
#define IDERRcSecureAdditionalTaxIDFlag_36        36L
#define IDERRcSecureAddressFlag_37                37L
#define IDERRcSecurePhoneFlag_38                  38L
#define IDERRcSecureE_MailAddressFlag_39          39L
#define IDERRcSecureDateofBirthFlag_40            40L
#define IDERRcSecureGenderFlag_41                 41L
#define IDERRcSecureUserDefStringFlag1_42         42L
#define IDERRcSecureUserDefStringFlag2_43         43L
#define IDERRcSecureUserDefStringFlag3_44         44L
#define IDERRcSecureUserDefStringFlag4_45         45L
#define IDERRcSecureUserDefStringFlag5_46         46L
#define IDERRcSecureUserDefDateFlag_47            47L
#define IDERRcSecureUserDefMathNumericF_48        48L
#define IDERRcSecureUserDefCharacterFlag_49       49L

#endif
 /***************************************************************************** 
 * Source Preprocessor Definitions 
 ****************************************************************************/ 
#if defined (JDEBFRTN) 
	#undef JDEBFRTN 
#endif 
 
#if defined (WIN32) 
	#if defined (WIN32) 
		#define JDEBFRTN(r) __declspec(dllexport) r 
	#else 
		#define JDEBFRTN(r) __declspec(dllimport) r 
	#endif 
#else 
	#define JDEBFRTN(r) r 
#endif 
 
/***************************************************************************** 
 * Business Function Prototypes 
 ****************************************************************************/ 
 JDEBFRTN (ID) JDEBFWINAPI SecureAddressBookPersonalData    (LPBHVRCOM lpBhvrCom, LPVOID lpVoid, LPDSD0100095 lpDS); 
 
 
/***************************************************************************** 
 * Internal Function Prototypes 
 ****************************************************************************/ 
 void I0100095_SetDataPrivate(LPBHVRCOM lpBhvrCom, LPVOID lpVoid, JCHAR cSecureFlag, ID idMemberToSecure);
          
#endif    /* __B0100095_H */ 
 
