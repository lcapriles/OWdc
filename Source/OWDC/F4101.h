#ifndef __F4101__
#define __F4101__

#define DATA_VERSION_F4101  1
#define NID_F4101  _J("F4101")
#define STABLENAME_F4101  _J("F4101")

typedef struct
{
#define NID_ITM  _J("ITM")
   MATH_NUMERIC   imitm;               /* 0 to 48 */
#define NID_LITM  _J("LITM")
   JCHAR           imlitm[26];          /* 49 to 100 */
#define NID_AITM  _J("AITM")
   JCHAR           imaitm[26];          /* 101 to 152 */
#define NID_DSC1  _J("DSC1")
   JCHAR           imdsc1[31];          /* 153 to 214 */
#define NID_DSC2  _J("DSC2")
   JCHAR           imdsc2[31];          /* 215 to 276 */
#define NID_SRTX  _J("SRTX")
   JCHAR           imsrtx[31];          /* 277 to 338 */
#define NID_ALN  _J("ALN")
   JCHAR           imaln[31];           /* 339 to 400 */
#define NID_SRP1  _J("SRP1")
   JCHAR           imsrp1[4];           /* 401 to 408 */
#define NID_SRP2  _J("SRP2")
   JCHAR           imsrp2[4];           /* 409 to 416 */
#define NID_SRP3  _J("SRP3")
   JCHAR           imsrp3[4];           /* 417 to 424 */
#define NID_SRP4  _J("SRP4")
   JCHAR           imsrp4[4];           /* 425 to 432 */
#define NID_SRP5  _J("SRP5")
   JCHAR           imsrp5[4];           /* 433 to 440 */
#define NID_SRP6  _J("SRP6")
   JCHAR           imsrp6[7];           /* 441 to 454 */
#define NID_SRP7  _J("SRP7")
   JCHAR           imsrp7[7];           /* 455 to 468 */
#define NID_SRP8  _J("SRP8")
   JCHAR           imsrp8[7];           /* 469 to 482 */
#define NID_SRP9  _J("SRP9")
   JCHAR           imsrp9[7];           /* 483 to 496 */
#define NID_SRP0  _J("SRP0")
   JCHAR           imsrp0[7];           /* 497 to 510 */
#define NID_PRP1  _J("PRP1")
   JCHAR           imprp1[4];           /* 511 to 518 */
#define NID_PRP2  _J("PRP2")
   JCHAR           imprp2[4];           /* 519 to 526 */
#define NID_PRP3  _J("PRP3")
   JCHAR           imprp3[4];           /* 527 to 534 */
#define NID_PRP4  _J("PRP4")
   JCHAR           imprp4[4];           /* 535 to 542 */
#define NID_PRP5  _J("PRP5")
   JCHAR           imprp5[4];           /* 543 to 550 */
#define NID_PRP6  _J("PRP6")
   JCHAR           imprp6[7];           /* 551 to 564 */
#define NID_PRP7  _J("PRP7")
   JCHAR           imprp7[7];           /* 565 to 578 */
#define NID_PRP8  _J("PRP8")
   JCHAR           imprp8[7];           /* 579 to 592 */
#define NID_PRP9  _J("PRP9")
   JCHAR           imprp9[7];           /* 593 to 606 */
#define NID_PRP0  _J("PRP0")
   JCHAR           imprp0[7];           /* 607 to 620 */
#define NID_CDCD  _J("CDCD")
   JCHAR           imcdcd[16];          /* 621 to 652 */
#define NID_PDGR  _J("PDGR")
   JCHAR           impdgr[4];           /* 653 to 660 */
#define NID_DSGP  _J("DSGP")
   JCHAR           imdsgp[4];           /* 661 to 668 */
#define NID_PRGR  _J("PRGR")
   JCHAR           imprgr[9];           /* 669 to 686 */
#define NID_RPRC  _J("RPRC")
   JCHAR           imrprc[9];           /* 687 to 704 */
#define NID_ORPR  _J("ORPR")
   JCHAR           imorpr[9];           /* 705 to 722 */
#define NID_BUYR  _J("BUYR")
   MATH_NUMERIC   imbuyr;              /* 723 to 771 */
#define NID_DRAW  _J("DRAW")
   JCHAR           imdraw[21];          /* 772 to 813 */
#define NID_RVNO  _J("RVNO")
   JCHAR           imrvno[3];           /* 814 to 819 */
#define NID_DSZE  _J("DSZE")
   JCHAR           imdsze;              /* 820 to 821 */
#define NID_VCUD  _J("VCUD")
   MATH_NUMERIC   imvcud;              /* 822 to 870 */
#define NID_CARS  _J("CARS")
   MATH_NUMERIC   imcars;              /* 871 to 919 */
#define NID_CARP  _J("CARP")
   MATH_NUMERIC   imcarp;              /* 920 to 968 */
#define NID_SHCN  _J("SHCN")
   JCHAR           imshcn[4];           /* 969 to 976 */
#define NID_SHCM  _J("SHCM")
   JCHAR           imshcm[4];           /* 977 to 984 */
#define NID_UOM1  _J("UOM1")
   JCHAR           imuom1[3];           /* 985 to 990 */
#define NID_UOM2  _J("UOM2")
   JCHAR           imuom2[3];           /* 991 to 996 */
#define NID_UOM3  _J("UOM3")
   JCHAR           imuom3[3];           /* 997 to 1002 */
#define NID_UOM4  _J("UOM4")
   JCHAR           imuom4[3];           /* 1003 to 1008 */
#define NID_UOM6  _J("UOM6")
   JCHAR           imuom6[3];           /* 1009 to 1014 */
#define NID_UOM8  _J("UOM8")
   JCHAR           imuom8[3];           /* 1015 to 1020 */
#define NID_UOM9  _J("UOM9")
   JCHAR           imuom9[3];           /* 1021 to 1026 */
#define NID_UWUM  _J("UWUM")
   JCHAR           imuwum[3];           /* 1027 to 1032 */
#define NID_UVM1  _J("UVM1")
   JCHAR           imuvm1[3];           /* 1033 to 1038 */
#define NID_SUTM  _J("SUTM")
   JCHAR           imsutm[3];           /* 1039 to 1044 */
#define NID_UMVW  _J("UMVW")
   JCHAR           imumvw;              /* 1045 to 1046 */
#define NID_CYCL  _J("CYCL")
   JCHAR           imcycl[4];           /* 1047 to 1054 */
#define NID_GLPT  _J("GLPT")
   JCHAR           imglpt[5];           /* 1055 to 1064 */
#define NID_PLEV  _J("PLEV")
   JCHAR           implev;              /* 1065 to 1066 */
#define NID_PPLV  _J("PPLV")
   JCHAR           impplv;              /* 1067 to 1068 */
#define NID_CLEV  _J("CLEV")
   JCHAR           imclev;              /* 1069 to 1070 */
#define NID_PRPO  _J("PRPO")
   JCHAR           imprpo;              /* 1071 to 1072 */
#define NID_CKAV  _J("CKAV")
   JCHAR           imckav;              /* 1073 to 1074 */
#define NID_BPFG  _J("BPFG")
   JCHAR           imbpfg;              /* 1075 to 1076 */
#define NID_SRCE  _J("SRCE")
   JCHAR           imsrce;              /* 1077 to 1078 */
#define NID_OT1Y  _J("OT1Y")
   JCHAR           imot1y;              /* 1079 to 1080 */
#define NID_OT2Y  _J("OT2Y")
   JCHAR           imot2y;              /* 1081 to 1082 */
#define NID_STDP  _J("STDP")
   MATH_NUMERIC   imstdp;              /* 1083 to 1131 */
#define NID_FRMP  _J("FRMP")
   MATH_NUMERIC   imfrmp;              /* 1132 to 1180 */
#define NID_THRP  _J("THRP")
   MATH_NUMERIC   imthrp;              /* 1181 to 1229 */
#define NID_STDG  _J("STDG")
   JCHAR           imstdg[4];           /* 1230 to 1237 */
#define NID_FRGD  _J("FRGD")
   JCHAR           imfrgd[4];           /* 1238 to 1245 */
#define NID_THGD  _J("THGD")
   JCHAR           imthgd[4];           /* 1246 to 1253 */
#define NID_COTY  _J("COTY")
   JCHAR           imcoty;              /* 1254 to 1255 */
#define NID_STKT  _J("STKT")
   JCHAR           imstkt;              /* 1256 to 1257 */
#define NID_LNTY  _J("LNTY")
   JCHAR           imlnty[3];           /* 1258 to 1263 */
#define NID_CONT  _J("CONT")
   JCHAR           imcont;              /* 1264 to 1265 */
#define NID_BACK  _J("BACK")
   JCHAR           imback;              /* 1266 to 1267 */
#define NID_IFLA  _J("IFLA")
   JCHAR           imifla[3];           /* 1268 to 1273 */
#define NID_TFLA  _J("TFLA")
   JCHAR           imtfla[3];           /* 1274 to 1279 */
#define NID_INMG  _J("INMG")
   JCHAR           iminmg[11];          /* 1280 to 1301 */
#define NID_ABCS  _J("ABCS")
   JCHAR           imabcs;              /* 1302 to 1303 */
#define NID_ABCM  _J("ABCM")
   JCHAR           imabcm;              /* 1304 to 1305 */
#define NID_ABCI  _J("ABCI")
   JCHAR           imabci;              /* 1306 to 1307 */
#define NID_OVR  _J("OVR")
   JCHAR           imovr;               /* 1308 to 1309 */
#define NID_WARR  _J("WARR")
   JCHAR           imwarr[9];           /* 1310 to 1327 */
#define NID_CMCG  _J("CMCG")
   JCHAR           imcmcg[9];           /* 1328 to 1345 */
#define NID_SRNR  _J("SRNR")
   JCHAR           imsrnr;              /* 1346 to 1347 */
#define NID_PMTH  _J("PMTH")
   JCHAR           impmth;              /* 1348 to 1349 */
#define NID_FIFO  _J("FIFO")
   JCHAR           imfifo;              /* 1350 to 1351 */
#define NID_LOTS  _J("LOTS")
   JCHAR           imlots;              /* 1352 to 1353 */
#define NID_SLD  _J("SLD")
   MATH_NUMERIC   imsld;               /* 1354 to 1402 */
#define NID_ANPL  _J("ANPL")
   MATH_NUMERIC   imanpl;              /* 1403 to 1451 */
#define NID_MPST  _J("MPST")
   JCHAR           immpst;              /* 1452 to 1453 */
#define NID_PCTM  _J("PCTM")
   MATH_NUMERIC   impctm;              /* 1454 to 1502 */
#define NID_MMPC  _J("MMPC")
   MATH_NUMERIC   immmpc;              /* 1503 to 1551 */
#define NID_PTSC  _J("PTSC")
   JCHAR           imptsc[3];           /* 1552 to 1557 */
#define NID_SNS  _J("SNS")
   JCHAR           imsns;               /* 1558 to 1559 */
#define NID_LTLV  _J("LTLV")
   MATH_NUMERIC   imltlv;              /* 1560 to 1608 */
#define NID_LTMF  _J("LTMF")
   MATH_NUMERIC   imltmf;              /* 1609 to 1657 */
#define NID_LTCM  _J("LTCM")
   MATH_NUMERIC   imltcm;              /* 1658 to 1706 */
#define NID_OPC  _J("OPC")
   JCHAR           imopc;               /* 1707 to 1708 */
#define NID_OPV  _J("OPV")
   MATH_NUMERIC   imopv;               /* 1709 to 1757 */
#define NID_ACQ  _J("ACQ")
   MATH_NUMERIC   imacq;               /* 1758 to 1806 */
#define NID_MLQ  _J("MLQ")
   MATH_NUMERIC   immlq;               /* 1807 to 1855 */
#define NID_LTPU  _J("LTPU")
   MATH_NUMERIC   imltpu;              /* 1856 to 1904 */
#define NID_MPSP  _J("MPSP")
   JCHAR           immpsp;              /* 1905 to 1906 */
#define NID_MRPP  _J("MRPP")
   JCHAR           immrpp;              /* 1907 to 1908 */
#define NID_ITC  _J("ITC")
   JCHAR           imitc;               /* 1909 to 1910 */
#define NID_ORDW  _J("ORDW")
   JCHAR           imordw;              /* 1911 to 1912 */
#define NID_MTF1  _J("MTF1")
   MATH_NUMERIC   immtf1;              /* 1913 to 1961 */
#define NID_MTF2  _J("MTF2")
   MATH_NUMERIC   immtf2;              /* 1962 to 2010 */
#define NID_MTF3  _J("MTF3")
   MATH_NUMERIC   immtf3;              /* 2011 to 2059 */
#define NID_MTF4  _J("MTF4")
   MATH_NUMERIC   immtf4;              /* 2060 to 2108 */
#define NID_MTF5  _J("MTF5")
   MATH_NUMERIC   immtf5;              /* 2109 to 2157 */
#define NID_EXPD  _J("EXPD")
   MATH_NUMERIC   imexpd;              /* 2158 to 2206 */
#define NID_DEFD  _J("DEFD")
   MATH_NUMERIC   imdefd;              /* 2207 to 2255 */
#define NID_SFLT  _J("SFLT")
   MATH_NUMERIC   imsflt;              /* 2256 to 2304 */
#define NID_MAKE  _J("MAKE")
   JCHAR           immake;              /* 2305 to 2306 */
#define NID_COBY  _J("COBY")
   JCHAR           imcoby;              /* 2307 to 2308 */
#define NID_LLX  _J("LLX")
   MATH_NUMERIC   imllx;               /* 2309 to 2357 */
#define NID_CMGL  _J("CMGL")
   JCHAR           imcmgl;              /* 2358 to 2359 */
#define NID_COMH  _J("COMH")
   MATH_NUMERIC   imcomh;              /* 2360 to 2408 */
#define NID_URCD  _J("URCD")
   JCHAR           imurcd[3];           /* 2409 to 2414 */
#define NID_URDT  _J("URDT")
   JDEDATE        imurdt;              /* 2415 to 2420 */
#define NID_URAT  _J("URAT")
   MATH_NUMERIC   imurat;              /* 2421 to 2469 */
#define NID_URAB  _J("URAB")
   MATH_NUMERIC   imurab;              /* 2470 to 2518 */
#define NID_URRF  _J("URRF")
   JCHAR           imurrf[16];          /* 2519 to 2550 */
#define NID_USER  _J("USER")
   JCHAR           imuser[11];          /* 2551 to 2572 */
#define NID_PID  _J("PID")
   JCHAR           impid[11];           /* 2573 to 2594 */
#define NID_JOBN  _J("JOBN")
   JCHAR           imjobn[11];          /* 2595 to 2616 */
#define NID_UPMJ  _J("UPMJ")
   JDEDATE        imupmj;              /* 2617 to 2622 */
#define NID_TDAY  _J("TDAY")
   MATH_NUMERIC   imtday;              /* 2623 to 2671 */
#define NID_UPCN  _J("UPCN")
   JCHAR           imupcn[14];          /* 2672 to 2699 */
#define NID_SCC0  _J("SCC0")
   JCHAR           imscc0[15];          /* 2700 to 2729 */
#define NID_UMUP  _J("UMUP")
   JCHAR           imumup[3];           /* 2730 to 2735 */
#define NID_UMDF  _J("UMDF")
   JCHAR           imumdf[3];           /* 2736 to 2741 */
#define NID_UMS0  _J("UMS0")
   JCHAR           imums0[3];           /* 2742 to 2747 */
#define NID_UMS1  _J("UMS1")
   JCHAR           imums1[3];           /* 2748 to 2753 */
#define NID_UMS2  _J("UMS2")
   JCHAR           imums2[3];           /* 2754 to 2759 */
#define NID_UMS3  _J("UMS3")
   JCHAR           imums3[3];           /* 2760 to 2765 */
#define NID_UMS4  _J("UMS4")
   JCHAR           imums4[3];           /* 2766 to 2771 */
#define NID_UMS5  _J("UMS5")
   JCHAR           imums5[3];           /* 2772 to 2777 */
#define NID_UMS6  _J("UMS6")
   JCHAR           imums6[3];           /* 2778 to 2783 */
#define NID_UMS7  _J("UMS7")
   JCHAR           imums7[3];           /* 2784 to 2789 */
#define NID_UMS8  _J("UMS8")
   JCHAR           imums8[3];           /* 2790 to 2795 */
#define NID_POC  _J("POC")
   JCHAR           impoc;               /* 2796 to 2797 */
#define NID_AVRT  _J("AVRT")
   MATH_NUMERIC   imavrt;              /* 2798 to 2846 */
#define NID_EQTY  _J("EQTY")
   JCHAR           imeqty[6];           /* 2847 to 2858 */
#define NID_WTRQ  _J("WTRQ")
   JCHAR           imwtrq;              /* 2859 to 2860 */
#define NID_TMPL  _J("TMPL")
   JCHAR           imtmpl[21];          /* 2861 to 2902 */
#define NID_SEG1  _J("SEG1")
   JCHAR           imseg1[11];          /* 2903 to 2924 */
#define NID_SEG2  _J("SEG2")
   JCHAR           imseg2[11];          /* 2925 to 2946 */
#define NID_SEG3  _J("SEG3")
   JCHAR           imseg3[11];          /* 2947 to 2968 */
#define NID_SEG4  _J("SEG4")
   JCHAR           imseg4[11];          /* 2969 to 2990 */
#define NID_SEG5  _J("SEG5")
   JCHAR           imseg5[11];          /* 2991 to 3012 */
#define NID_SEG6  _J("SEG6")
   JCHAR           imseg6[11];          /* 3013 to 3034 */
#define NID_SEG7  _J("SEG7")
   JCHAR           imseg7[11];          /* 3035 to 3056 */
#define NID_SEG8  _J("SEG8")
   JCHAR           imseg8[11];          /* 3057 to 3078 */
#define NID_SEG9  _J("SEG9")
   JCHAR           imseg9[11];          /* 3079 to 3100 */
#define NID_SEG0  _J("SEG0")
   JCHAR           imseg0[11];          /* 3101 to 3122 */
#define NID_MIC  _J("MIC")
   JCHAR           immic;               /* 3123 to 3124 */
#define NID_AING  _J("AING")
   JCHAR           imaing;              /* 3125 to 3126 */
#define NID_BBDD  _J("BBDD")
   MATH_NUMERIC   imbbdd;              /* 3127 to 3175 */
#define NID_CMDM  _J("CMDM")
   JCHAR           imcmdm;              /* 3176 to 3177 */
#define NID_LECM  _J("LECM")
   JCHAR           imlecm;              /* 3178 to 3179 */
#define NID_LEDD  _J("LEDD")
   MATH_NUMERIC   imledd;              /* 3180 to 3228 */
#define NID_PEFD  _J("PEFD")
   MATH_NUMERIC   impefd;              /* 3229 to 3277 */
#define NID_SBDD  _J("SBDD")
   MATH_NUMERIC   imsbdd;              /* 3278 to 3326 */
#define NID_U1DD  _J("U1DD")
   MATH_NUMERIC   imu1dd;              /* 3327 to 3375 */
#define NID_U2DD  _J("U2DD")
   MATH_NUMERIC   imu2dd;              /* 3376 to 3424 */
#define NID_U3DD  _J("U3DD")
   MATH_NUMERIC   imu3dd;              /* 3425 to 3473 */
#define NID_U4DD  _J("U4DD")
   MATH_NUMERIC   imu4dd;              /* 3474 to 3522 */
#define NID_U5DD  _J("U5DD")
   MATH_NUMERIC   imu5dd;              /* 3523 to 3571 */
#define NID_DLTL  _J("DLTL")
   MATH_NUMERIC   imdltl;              /* 3572 to 3620 */
#define NID_DPPO  _J("DPPO")
   JCHAR           imdppo;              /* 3621 to 3622 */
#define NID_DUAL  _J("DUAL")
   JCHAR           imdual;              /* 3623 to 3624 */
#define NID_XDCK  _J("XDCK")
   JCHAR           imxdck;              /* 3625 to 3626 */
#define NID_LAF  _J("LAF")
   JCHAR           imlaf;               /* 3627 to 3628 */
#define NID_LTFM  _J("LTFM")
   JCHAR           imltfm;              /* 3629 to 3630 */
#define NID_RWLA  _J("RWLA")
   JCHAR           imrwla;              /* 3631 to 3632 */
#define NID_LNPA  _J("LNPA")
   JCHAR           imlnpa;              /* 3633 to 3634 */
#define NID_LOTC  _J("LOTC")
   JCHAR           imlotc[4];           /* 3635 to 3642 */
#define NID_APSC  _J("APSC")
   JCHAR           imapsc;              /* 3643 to 3644 */
#define NID_AUOM  _J("AUOM")
   JCHAR           imauom[10];          /* 3645 to 3664 */
#define NID_CONB  _J("CONB")
   JCHAR           imconb;              /* 3665 to 3666 */
#define NID_GCMP  _J("GCMP")
   JCHAR           imgcmp;              /* 3667 to 3668 */
#define NID_PRI1  _J("PRI1")
   MATH_NUMERIC   impri1;              /* 3669 to 3717 */
#define NID_PRI2  _J("PRI2")
   MATH_NUMERIC   impri2;              /* 3718 to 3766 */
#define NID_ASHL  _J("ASHL")
   JCHAR           imashl;              /* 3767 to 3768 */
#define NID_VMINV  _J("VMINV")
   JCHAR           imvminv;             /* 3769 to 3770 */
#define NID_CMETH  _J("CMETH")
   JCHAR           imcmeth;             /* 3771 to 3772 */
#define NID_EXPI  _J("EXPI")
   JCHAR           imexpi;              /* 3773 to 3774 */
#define NID_OPTH  _J("OPTH")
   MATH_NUMERIC   imopth;              /* 3775 to 3823 */
#define NID_CUTH  _J("CUTH")
   MATH_NUMERIC   imcuth;              /* 3824 to 3872 */
#define NID_UMTH  _J("UMTH")
   JCHAR           imumth[4];           /* 3873 to 3880 */
#define NID_LMFG  _J("LMFG")
   JCHAR           imlmfg;              /* 3881 to 3882 */
#define NID_LINE  _J("LINE")
   JCHAR           imline[13];          /* 3883 to 3908 */
#define NID_DFTPCT  _J("DFTPCT")
   MATH_NUMERIC   imdftpct;            /* 3909 to 3957 */
#define NID_KBIT  _J("KBIT")
   JCHAR           imkbit;              /* 3958 to 3959 */
#define NID_DFENDITM  _J("DFENDITM")
   JCHAR           imdfenditm;          /* 3960 to 3961 */
#define NID_KANEXLL  _J("KANEXLL")
   JCHAR           imkanexll;           /* 3962 to 3963 */
#define NID_SCPSELL  _J("SCPSELL")
   JCHAR           imscpsell;           /* 3964 to 3965 */
} F4101, FAR *LPF4101;

/* PRIMARY INDEX */
#define ID_F4101_ITEM_MASTER  1L

typedef struct
{
   MATH_NUMERIC   imitm;               /* 0 to 48 */
} KEY1_F4101, FAR *LPKEY1_F4101;

#define ID_F4101_2ND_ITEM_NUMBER  2L

typedef struct
{
   JCHAR           imlitm[26];          /* 49 to 100 */
} KEY2_F4101, FAR *LPKEY2_F4101;

#define ID_F4101_3RD_ITEM_NUMBER  3L

typedef struct
{
   JCHAR           imaitm[26];          /* 101 to 152 */
} KEY3_F4101, FAR *LPKEY3_F4101;

#define ID_F4101_SEARCH_TEXT_COMPRESSED  4L

typedef struct
{
   JCHAR           imaln[31];           /* 153 to 214 */
} KEY4_F4101, FAR *LPKEY4_F4101;

#define ID_F4101_LOW_LEVEL_CODE  5L

typedef struct
{
   MATH_NUMERIC   imllx;               /* 215 to 263 */
} KEY5_F4101, FAR *LPKEY5_F4101;

#define ID_F4101_SEGMENT1__SEG_2  6L

typedef struct
{
   JCHAR           imseg1[11];          /* 264 to 285 */
   JCHAR           imseg2[11];          /* 286 to 307 */
   JCHAR           imseg3[11];          /* 308 to 329 */
   JCHAR           imseg4[11];          /* 330 to 351 */
   JCHAR           imseg5[11];          /* 352 to 373 */
   JCHAR           imseg6[11];          /* 374 to 395 */
   JCHAR           imseg7[11];          /* 396 to 417 */
   JCHAR           imseg8[11];          /* 418 to 439 */
   JCHAR           imseg9[11];          /* 440 to 461 */
   JCHAR           imseg0[11];          /* 462 to 483 */
} KEY6_F4101, FAR *LPKEY6_F4101;

#define ID_F4101_TEMPLATE  7L

typedef struct
{
   JCHAR           imtmpl[21];          /* 484 to 525 */
} KEY7_F4101, FAR *LPKEY7_F4101;

/* UPCN Number  */
#define ID_F4101_UPC_NUMBER  8L

typedef struct
{
   JCHAR           imupcn[14];          /* 526 to 553 */
} KEY8_F4101, FAR *LPKEY8_F4101;


#endif
