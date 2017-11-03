
#ifndef FAPI_INTERFACE_H
#define FAPI_INTERFACE_H

#include "UeBaseType.h"

/* Set alignment to 32 bit boundary for this file */
/*SPR_10158_FIX_START*/
/* + TM7_8_changes Start*/
/* - TM7_8_changes End*/

#pragma pack(4)
/*SPR_10158_FIX_END*/
typedef unsigned int    uint32_t;
typedef unsigned short  uint16_t;
typedef unsigned char   uint8_t;

/* +DYNAMIC_ICIC */
typedef signed int      int32_t;
/* -DYNAMIC_ICIC */

/* + TM7_8_changes Start*/
typedef signed char      int8_t;
/* - TM7_8_changes End*/

/**
 * FAPI_msgIdType_t definition
 */
typedef uint8_t FAPI_msgIdType_t;
/**
 * Possible hashdefs of FAPI_msgIdType_t
 */

#define PHY_PARAM_REQUEST               0x00
#define PHY_PARAM_RESPONSE              0x01
#define PHY_CELL_CONFIG_REQUEST         0x02
#define PHY_CELL_CONFIG_RESPONSE        0x03
#define PHY_START_REQUEST               0x04
#define PHY_STOP_REQUEST                0x05
#define PHY_STOP_INDICATION             0x06
#define PHY_UE_CONFIG_REQUEST           0x07
#define PHY_UE_CONFIG_RESPONSE          0x08
#define PHY_ERROR_INDICATION            0x09

#define PHY_DL_CONFIG_REQUEST          0x80
#define PHY_UL_CONFIG_REQUEST          0x81
#define PHY_UL_SUBFRAME_INDICATION     0x82
#define PHY_DL_HI_DCI0_REQUEST         0x83
#define PHY_DL_TX_REQUEST              0x84
#define PHY_UL_HARQ_INDICATION         0x85
#define PHY_UL_CRC_INDICATION          0x86
#define PHY_UL_RX_ULSCH_INDICATION     0x87
#define PHY_UL_RACH_INDICATION         0x88
#define PHY_UL_SRS_INDICATION          0x89
#define PHY_UL_RX_SR_INDICATION        0x8a
#define PHY_UL_RX_CQI_INDICATION       0x8b
#define PHY_DELETE_UE_REQUEST          0xf0

/* +DYNAMIC_ICIC */
#define PHY_MEASUREMENT_REPORT_INDICATION 0x96
/* -DYNAMIC_ICIC */


/* This macro is used for declaring array of variable length */
#define FAPI_VAR_SIZE(x) 1


/**
* Data : MACRO
* Name : RECONFIG_TLV_NUM
* Description : This MACRO describes the number of TLV changes request during
*               PHY Reconfig currently value is 2, later may be increased as
*               number of runtime reconfig parameters supported increases
**/
#define RECONFIG_TLV_NUM 2


/*  SPR 8128 change Start */
/* L1 Error Indication */
#define      FAPI_MSG_OK                                0
#define      FAPI_MSG_INVALID_STATE                     1
#define      FAPI_MSG_INVALID_CONFIG                    2
#define      FAPI_SFN_OUT_OF_SYNC                       3 
#define      FAPI_MSG_SUBFRAME_ERR                      4
#define      FAPI_MSG_BCH_MISSING                       5
#define      FAPI_MSG_INVALID_SFN                       6 
#define      FAPI_MSG_HI_ERR                            7  
#define      FAPI_MSG_TX_ERR                            8
#define      FAPI_MSG_SYNTAX_ERR                            9
/*+ SPR 9285*/
typedef      uint8_t  FAPI_l1ErrorCodes_en;
/*- SPR 9285*/

/* PHY CELL CONFIG Possible Tags */
#define                FAPI_DUPLEXING_MODE               1 
#define                FAPI_PCFICH_POWER_OFFSET          2
#define                FAPI_P_B                          3
#define                FAPI_DL_CYCLIC_PREFIX_TYPE        4
#define                FAPI_UL_CYCLIC_PREFIX_TYPE        5
/* RF Config */
#define                FAPI_DL_CHANNEL_BANDWIDTH         6   
#define                FAPI_UL_CHANNEL_BANDWIDTH         7
#define                FAPI_REFERENCE_SIGNAL_POWER       8
#define                FAPI_TX_ANTENNA_PORTS             9
#define                FAPI_RX_ANTENNA_PORTS             10
/* RF CONFIG TAGS ENDS */
/* PHICH CONFIG */
#define                FAPI_PHICH_RESOURCE               11
#define                FAPI_PHICH_DURATION               12
#define                FAPI_PHICH_POWER_OFFSET           13
/* PHICH CONFIG ENDS */
/* SCH COnfig */
#define                FAPI_PRIMARY_SYNC_SIGNAL          14
#define                FAPI_SECONDARY_SYNC_SIGNAL        15
#define                FAPI_PHYSICAL_CELL_ID             16
/* SCH COnfig  Ends */
/* PRACH Config */
#define             FAPI_CONFIGURATION_INDEX                     17
#define             FAPI_ROOT_SEQUENCE_INDEX                     18
#define             FAPI_ZERO_CORRELATION_ZONE_CONFIGURATION     19
#define             FAPI_HIGH_SPEED_FLAG                         20
 #define             FAPI_FREQUENCY_OFFSET                       21
/* PRACH Config  Ends */
/* PUSCH Config */
 #define             FAPI_HOPPING_MODE                           22
#define             FAPI_HOPPIG_OFFSET                           23
#define             FAPI_NUM_OF_SUB_BANDS                        24
/* PUSCH Config  Ends */
/* PUCCH Config */
#define            FAPI_DELTA_PUCCH_SHIFT                        25
#define            FAPI_N_CQI_RB                                 26
#define            FAPI_N_AN_CS                                  27
#define            FAPI_N_1_PUCCH_AN                             28
/* PUCCH Config Ends */
/* SRS Config */
#define             FAPI_BANDWIDTH_CONFIGURATION                 29
#define             FAPI_MAX_UP_PTS                              30
#define             FAPI_SRS_SUB_FRAME_CONFIGURATION             31
#define             FAPI_SRS_ACK_NACK_SRS_SIMULTANEOUS_TRANSMISSION   32
/* SRS Config Ends */
/* Uplink reference signal Config */
#define             FAPI_UPLINK_RS_HOPPING                       33
#define             FAPI_GROUP_ASSIGNMENT                        34
 #define             FAPI_CYCLIC_SHIFT_1_FOR_DMRS                35
/* Uplink reference signal Config Ends */
/* TDD Frame Structure Config */
#define             FAPI_SUB_FRAME_ASSIGNMENT                    36
#define             FAPI_SPECIAL_SUB_FRAME_PATTERNS              37
#ifndef ASTRI_INTF
              /* + PRS_CHANGES */
#define             FAPI_PRS_BANDWIDTH                           38
#define             FAPI_PRS_CYCLIC_PREFIX                       39
              /* - PRS_CHANGES */
#endif
/* TDD Frame Structure Config Ends */
/* below tags are used by L1 to reports its physical capabilities to L2/L3 software */
#define             FAPI_DL_BANDWIDTH_SUPPORT                    40 
#define             FAPI_UL_BANDWIDTH_SUPPORT                    41
#define             FAPI_DL_MODULATION_SUPPORT                   42
#define             FAPI_UL_MODULATION_SUPPORT                   43
 #define             FAPI_PHY_ANTENNA_CAPABILITY                 44
/* below tags are used by L2/L3 software to configure the interaction between L2/L3 and L1 */
           
#define             FAPI_DATA_REPORT_MODE                        50 
#define             FAPI_SFN_SF                                  51
           
/* the below tag is used by L1 to report its current status */
#define              FAPI_PHY_STATE                              60


/* ASTRI Variables */
#ifdef ASTRI_INTF 
#define   FAPI_DCI_POWER_OFFSET  38
#define   FAPI_RACH_AND_SR_REPORT_MODE  53
#define   FAPI_PHY_SYNC_MODE  52
#define   FAPI_SRS_CONFIG_SUPPORT  61
#define   FAPI_PHY_VERSION_0  62
#define   FAPI_PHY_VERSION_1  63
#define   FAPI_EXTRACTION_WINDOW_MARGIN  80
#define   FAPI_PUCCH_NOISE_ESTIMATION_GAMMA  81
#define   FAPI_PRACH_FORMAT_4_PEAK_RATIO  82
#define   FAPI_PRACH_FORMAT_0_PEAK_RATIO  83
#define   FAPI_DOPPLER_ESTIMATION_COMPENSATION_FACTOR  84
#define   FAPI_PROBABILITY_DTX_ACK_PUSCH  85
#define   FAPI_PROBABILITY_DTX_ACK_PUCCH_FORMAT_1  86
#define   FAPI_UPLINK_PAYLOAD_ALLOCATION_METHOD  90
#define   FAPI_RUN_TIME_SYNCHRONIZATION_MODE 92
#define   FAPI_UL_EARFCN 96
#endif

/* --------------------------------------------- */

/* PHY states */
#define   FAPI_IDLE                                               0
#define   FAPI_CONFIGURED                                         1
#define   FAPI_RUNNING                                            2

/* Types of Duplexing Mode */
#define  FAPI_TDD                                                 0
#define  FAPI_FDD                                                 1
#define  FAPI_HD_FDD                                              2

/* Types of Cylic Prefix */
#define  FAPI_CP_NORMAL                                           0
#define  FAPI_CP_EXTENDED                                         1

/* UL/DL Channel Bandwidth */
#define   FAPI_CHANNEL_BW_6RB                                     6
#define   FAPI_CHANNEL_BW_15RB                                    15
#define   FAPI_CHANNEL_BW_25RB                                    25
#define   FAPI_CHANNEL_BW_50RB                                    50
#define   FAPI_CHANNEL_BW_75RB                                    75
#define   FAPI_CHANNEL_BW_100RB                                   100


/* No. of Tx Antenna Ports */
#define    FAPI_TX_ANTENNA_PORT_1                                 1
#define    FAPI_TX_ANTENNA_PORT_2                                 2
#define    FAPI_TX_ANTENNA_PORT_4                                 4


/* No. of Rx Antenna Ports */
#define   FAPI_RX_ANTENNA_PORT_1                                  1
#define   FAPI_RX_ANTENNA_PORT_2                                  2
#define   FAPI_RX_ANTENNA_PORT_4                                  4

/* POSSIBLE PHICH RESOURCE VALUES */
#define  FAPI_PHICH_R_ONE_SIXTH                                   0
#define  FAPI_PHICH_R_HALF                                        1
#define  FAPI_PHICH_R_ONE                                         2
#define  FAPI_PHICH_R_TWO                                         3

/* Phich Duration */
#define    FAPI_PHICH_D_NORMAL                                    0
#define    FAPI_PHICH_D_EXTENDED                                  1


/* High Speed Flag */
#define   FAPI_HS_UNRESTRICTED_SET                                0
#define   FAPI_HS_RESTRICTED_SET                                  1

/* Hopping Mode */
#define  FAPI_HM_INTER_SF                                         0
#define  FAPI_HM_INTRA_INTER_SF                                   1

/* Types of hopping */
#define FAPI_RS_NO_HOPPING                                        0                                       
#define FAPI_RS_GROUP_HOPPING                                     1  
#define FAPI_RS_SEQUENCE_HOPPING                                  2     

/* Physical Antenna Capabililty */
 #define   FAPI_PHY_ANTENNA_CAP_1                                 1
 #define   FAPI_PHY_ANTENNA_CAP_2                                 2
 #define   FAPI_PHY_ANTENNA_CAP_4                                 4


/* AN Repetition Factor */
 #define   FAPI_AN_REPETITION_FACTOR_2                            2
 #define   FAPI_AN_REPETITION_FACTOR_4                            4
 #define   FAPI_AN_REPETITION_FACTOR_6                            6

/* SPS DL Config Scheduling Interval */
#define  FAPI_SPS_DL_CONFIG_SCHD_INTERVAL_10                      10
#define  FAPI_SPS_DL_CONFIG_SCHD_INTERVAL_20                      20
#define  FAPI_SPS_DL_CONFIG_SCHD_INTERVAL_32                      32
#define  FAPI_SPS_DL_CONFIG_SCHD_INTERVAL_40                      40
#define  FAPI_SPS_DL_CONFIG_SCHD_INTERVAL_64                      64
#define  FAPI_SPS_DL_CONFIG_SCHD_INTERVAL_80                      80
#define  FAPI_SPS_DL_CONFIG_SCHD_INTERVAL_128                     128
#define  FAPI_SPS_DL_CONFIG_SCHD_INTERVAL_160                     160
#define  FAPI_SPS_DL_CONFIG_SCHD_INTERVAL_320                     320
#define   FAPI_SPS_DL_CONFIG_SCHD_INTERVAL_640                    640

/* Types of Resource Allocation */
#define  FAPI_RES_ALLOC_TYPE_0                                    0
#define  FAPI_RES_ALLOC_TYPE_1                                    1
#define  FAPI_RES_ALLOC_TYPE_2                                    2
   
/* vRB Assignment Flag */
 #define   FAPI_LOCALISED                                         0
 #define   FAPI_DISTRIBUTED                                       1

/* tb To CodeWordSwap Flag */
 #define   FAPI_NOSWAPPING                                        0
 #define   FAPI_SWAPPED                                           1

/* types of TPC */
 #define   FAPI_TX_POWER_CONTROL_MINUS_4                         -4
 #define   FAPI_TX_POWER_CONTROL_MINUS_1                         -1
 #define   FAPI_TX_POWER_CONTROL_0                                0
 #define   FAPI_TX_POWER_CONTROL_1                                1
 #define   FAPI_TX_POWER_CONTROL_3                                3
 #define   FAPI_TX_POWER_CONTROL_4                                4

/* Types of Transmission Scheme */
#define   FAPI_SINGLE_ANTENNA_PORT_0                              0 
#define   FAPI_TX_DIVERSITY                                       1
#define   FAPI_LARGE_DELAY_CDD                                    2
#define   FAPI_CLOSED_LOOP_SPATIAL_MULTIPLEXING                   3
#define   FAPI_MULTI_USER_MIMO                                    4    
#define   FAPI_CLOSED_LOOP_RANK_1_PRECODING                       5
#define    FAPI_SINGLE_ANTENNA_PORT_5                             6 
    /* + TM7_8 Changes Start */                                
#define   FAPI_SINGLE_ANTENNA_PORT_7                              7
#define   FAPI_SINGLE_ANTENNA_PORT_8                              8
#define   DUAL_LAYER_TX_PORT_7_AND_8                              9
    /* - TM7_8 Changes End */                                  

/* Types of RNTI */
#define   FAPI_C_RNTI                                             1 
#define   FAPI_RA_RNTI_P_RNTI_SI_RNTI                             2
#define   FAPI_SPS_CRNTI                                          3
#define   FAPI_OTHER_CRNTI                                        4
                                                                
/* types of AggegationLevel */                                  
 #define   FAPI_AGGEGATION_LEVEL_1                                1
 #define   FAPI_AGGEGATION_LEVEL_2                                2
 #define   FAPI_AGGEGATION_LEVEL_4                                4
 #define   FAPI_AGGEGATION_LEVEL_8                                8
                                                                
/* P-A values */                                                
#define   FAPI_DB_MINUS6                                          0
#define   FAPI_DB_MINUS_4DOT77                                    1  
#define   FAPI_DB_MINUS_3                                         2
#define   FAPI_DB_MINUS_1DOT77                                    3
#define   FAPI_DB0                                                4
#define   FAPI_DB1                                                5
#define   FAPI_DB2                                                6 
#define   FAPI_DB3                                                7
 
/* Types of modulation*/
#define   FAPI_QPSK                                               2
#define   FAPI_16QAM                                              4 
#define   FAPI_64QAM                                              6

/* ul Tx Mode type */
#define   FAPI_ULTX_SISO_SIMO                                     0
#define   FAPI_ULTX_MIMO                                          1

/* Types of HI Values */
#define   FAPI_HI_NACK                                            0
#define   FAPI_HI_ACK                                             1 

/* Types of CQI Request */
#define   FAPI_APERIODIC_CQI_NOT_REQUESTED                        0
#define   FAPI_APERIODIC_CQI_REQUESTED                            1

/* ue Tx Antenna Selection */
#define   FAPI_ANT_PORT_NOT_CONFIGURED                            0
#define   FAPI_CONF_UE_PORT_0                                     1   
#define   FAPI_CONF_UE_PORT_1                                     2

/* types of DL Assignment Index */
#define   FAPI_DL_ASSGN_INDEX_1                                   1
#define   FAPI_DL_ASSGN_INDEX_2                                   2
#define   FAPI_DL_ASSGN_INDEX_3                                   3
#define   FAPI_DL_ASSGN_INDEX_4                                   4 

/* types of HARQ feedback TB1-TB2 */
#define   FAPI_ACK                                                1
#define   FAPI_NACK                                               2
#define   FAPI_ACK_OR_NACK                                        3
#define   FAPI_DTX                                                4
#define   FAPI_ACK_OR_DTX                                         5 
#define   FAPI_NACK_OR_DTX                                        6
#define   FAPI_ACK_OR_NACK_OR_DTX                                 7

/* CRC FLAGS */
#define   FAPI_CRC_CORRECT                                        0
#define   FAPI_CRC_ERROR                                          1
                                                                 
/* + TM7_8 Changes Start */
#define MAX_NUM_ANTENNA 4
/*  SPR 8128 change end */
/* --------------------------------------------- */

/* parametrs which can be present in Config.Request */
typedef union FAPI_cellConfigParam_unT
{
     /* Type of duplexing mode 0: TDD,1: FDD,2: HD_FDD */
     uint16_t  duplexingMode;

     /*The power per antenna of the PCFICH with respect to the reference signal.
     Value: 0->10000 represents -6dB to 4dB in steps 0.001dB */
     uint16_t  pcfichPowerOffset;
            
     /*Refers to downlink power allocation. Value is an index into the 
     referenced table.Value: 0 -> 3*/
     uint16_t  pb;

     /*Cyclic prefix type, used for DL 0: CP_NORMAL, 1: CP_EXTENDED. */
     uint16_t  dlCyclicPrefixType;
     
     /*Cyclic prefix type, used for UL 0: CP_NORMAL, 1: CP_EXTENDED. */
     uint16_t  ulCyclicPrefixType;

     /* RF Config */
    
     /*Downlink channel bandwidth in resource blocks.
     Value: 6,15, 25, 50, 75, 100 */
     uint16_t  dlChannelBW;
    
     /*Uplink channel bandwidth in resource blocks.
     Value: 6,15, 25, 50, 75,100 */
     uint16_t  ulChannelBW;

     /* Normalized value levels (relative) to accommodate different 
     absolute Tx Power used by eNb.
     Value: 0 . 255 Representing 0dB to -63.75dB in -0.25dB step */
     uint16_t  refSignalPower;

     /*No. of cell specific transmit antenna ports.
     Value:1,2,4 */
     uint16_t  txAntennaPort;

     /* No. of cell specific receive antenna ports.
     Value: 1, 2, 4 */
     uint16_t  rxAntennaPort;
     
     /* RF Config Ends */

     /* PHICH Config */

     /* No.of resource element groups used for PHICH
     0: PHICH_R_ONE_SIXTH
     1: PHICH_R_HALF
     2: PHICH_R_ONE
     3: PHICH_R_TWO */
     uint16_t  phichResource;

     /* No. resource element groups used for PHICH
     0: PHICH_D_NORMAL
     1: PHICH_D_EXTENDED  */
     uint16_t  phichDuration;

     /* The power per antenna of the PHICH with respect to the reference signal:
     Value: 0-> 10000, represents -6dB to 4dB in steps 0.001dB */
     uint16_t  phichPowOffset;

     /* PHICH Config  Ends */
     
     /* SCH Config */

     /* The power of synchronization signal with respect to the reference signal
     Value: 0 . 9999 represents -6dB to 4dB in step 0.001dB */
     uint16_t  primarySyncSignal;

     /* The power of synchronization signal with respect to the reference signal
     Value: 0 . 9999 represents -6dB to 4dB in step 0.001dB */
     uint16_t  secondarySyncSignal;

     /* The Cell ID sent with the synchronization signal
     Value: 0 . 503 */
     uint16_t  physicalCellId;

     /* SCH Config Ends */

     /* PRACH Config start*/
     /* Provides information about the location and format of the PRACH. 
     See TS36.211, section 5.7. Table 5.7.1-2 for FDD, Table 5.7.1-3 for TDD.        
     The value is an index into the referenced tables. Value: 0 . 63 */ 
     uint16_t  configurationIndex;

     /* PRACH Root sequence index
     Value: 0 . 837 */
     uint16_t  rootSeqIndex;
     
     /* Equivalent to Ncs, 
     TDD: 0 . 6
     FDD: 0 . 15 */
     uint16_t  zeroCorelationZoneConfig;

     /* Indicates if unrestricted, or restricted, set of preambles is used
     0: HS_UNRESTRICTED_SET
     1: HS_RESTRICTED_SET */
     uint16_t  highSpeedFlag;

     /* The first physical resource block available for PRACH
     Value: 0 . UL_channel_bandwidth . 6 */
     uint16_t  freqOffset;
     /* PRACH Config Ends */

     /* PUSCH Config */

     /* If hopping is enabled indicates the type of hopping used
     0: HM_INTER_SF
     1: HM_INTRA_INTER_SF */
     uint16_t  hoppingMode;

     /* The offset used if hopping is enabled
     Value: 0 . 98 */
     uint16_t  hoppingOffset;

     /* The number of sub-bands used for hopping
     Value: 1 . 4 */
     uint16_t  numOfSubBand;

     /* PUSCH Config Ends */
    
     /* PUCCH Config */

     /* The cyclic shift difference
     Value: 1 . 3 */
     uint16_t  deltaPUCCHShift;

     /* The bandwidth, in units of resource blocks, that is available for use 
     by PUCCH formats 2/2a/2b transmission in each slot.
     Value: 0 . 98 */
     uint16_t  nCQIRB;

     /* The number of cyclic shifts used for PUCCH formats 1/1a/1b in a 
     resource block with a mix of formats 1/a/1/ab and 2/2a/2b
     Value: 0 . 7 */
     uint16_t  nAnCs;
     
     /* Value: 0 . 2047 */
     uint16_t  n1PucchAn;

     /* PUCCH Config Ends */

     /* SRS Config */
 
     /* The available SRS bandwidth of the cell
     The value is an index into the referenced table.
     Value: 0 . 7 */
     uint16_t  bandWidthConfiguration;

     /*  - Used for TDD only
     - Indicates how SRS operates in UpPTS subframes
     0: Disabled
     1: Enabled */
     uint16_t  macUpPTS;

     /* The subframe configuration. 
     Needed if semi-static configuration is held in PHY.
     Value: 0 . 15 */
     uint16_t  SRSSubframeConfiguration;

     /* Indicates if SRS and ACK/NACK can be received in the same subframe. 
     Needed if semi-static configuration is held in PHY.
     0: no simultaneous transmission
     1: simultaneous transmission */
     uint8_t  srsAckNackSimulTx;

     /* SRS Config Ends */

     /* Uplink Reference Signal Config */
     /* Indicates the type of hopping to use.
     0: RS_NO_HOPPING
     1: RS_GROUP_HOPPING
     2: RS_SEQUENCE_HOPPING */
     uint16_t  uplinkRSHoping;

     /* The sequence shift pattern used if group hopping is enabled.
     Values: 0 . 29 */
     uint16_t  groupAssignment;

     /* Specifies the cyclic shift for the reference signal used in the cell.
     The value is an index into the referenced table.
     Value: 0 . 7 */
     uint16_t  cyclicShift1forDMRS;
     /* Uplink Reference Signal Config Ends */

     /* TDD FRAME STRUCTURE CONFIG */

     /* indicates the DL/UL subframe structure.
     Value: 0 . 6 */
     uint16_t  subFrameAssignment;

     /* Length of fields DwPTS, GP and UpPTS.
     Value: 0 . 8 */
     uint16_t  specialSubFramePatterns;

     /* TDD FRAME STRUCTURE CONFIG Ends */

     /* + PRS_CHANGES */
     /*PRS bandwidth in resource blocks.
     Value: 6, 15, 25, 50, 75, 100*/
     uint8_t  prsBandWidth;

     /*The cyclic prefix used for PRS transmission.
     Value: 
     0: normal cyclic prefix
     1: extended cyclic prefix*/
     uint8_t  prsCyclicPrifix;
     /* - PRS_CHANGES */

     /* The data report mode for the uplink data.
     0: A CRC.indication message is sent in every subframe.  
     - If ULSCH data has been processed, the CRC.indication contains 
       CRC results for the subframe. 
     - The CRC results are, also, given in the RX.indication message.
     1: The CRC.indication message is not sent. 
     - The CRC results are given in the RX.indication message. */
     uint16_t  dataReportingMode;

     /* The future SFN/SF subframe where the TLVs included in the message 
     should be applied.A 16-bit value where,[15:4] SFN, range 0 . 1023
     [3:0] SF, range 0 . 9 */
     uint16_t  sfnsf;

     /* Below parametrs are used by L1 to report its physical capabilities to 
     the L2/L3 software */

     /* these parameters are used by L1 in Param.response from TDD when PHY
     is in IDLE state */

     /* Indicates the current operational state of the PHY.

     0 = IDLE
     1 = CONFIGURED
     2 = RUNNING   */
     uint16_t    phyState; /* define as macro preprocessor  */
     
     /* The PHY downlink channel bandwidth capability (in resource blocks). 
     Value: bitX :0 = no support, 1= support. 
     Bit0: 6
     Bit1: 15
     Bit2: 25
     Bit3: 50
     Bit4: 75
     Bit5: 100     */

     uint16_t    dlBandWidthSupport;
     
     /* The PHY uplink channel bandwidth capability (in resource blocks). 

     Value: bitX :0 = no support, 1= support.
     Bit0: 6
     Bit1: 15
     Bit2: 25
     Bit3: 50
     Bit4: 75
     Bit5: 100     */
     uint16_t    ulBandWidthSupport;
    
     /* The PHY downlink modulation capability.
     Value: bitX :0 = no support, 1= support.
     Bit0: QPSK
     Bit1: 16QAM
     Bit2: 64QAM */
     uint16_t    dlModulationSupport;

    /* The PHY uplink modulation capability.
    Value: bitX :0 = no support, 1= support.
    Bit0: QPSK
    Bit1: 16QAM
    Bit2: 64QAM    */
    uint16_t    ulModulationSupport;
   
    /* Number of antennas supported.
    Value: 1, 2, 4  */
    uint16_t    phyAntennaCapability;
   
    #ifdef ASTRI_INTF
    /* The method used by the PHY to signal thye 1ms subframe.
       1: MODE A subframe indication is not sent from the PHY.
               Only the subframe interrupt signal is sent.
       2: MODE B Both subframe indication and the subframe
                 interrupt signal are sent from the PHY */
    uint16_t  phySyncMode;
    /* The reporting mode for RACH indication and SR indication.
       - For subframes where a RACH and/or SR opportunity was scheduled:
       0: An indication message was always be produced. In the case of RACH,
          if no premable was received then the indication will report zero
          preambles. In the case of SR, if no SR was received then the
          indication will report zero SR's.
       1: A RACH indication will only be generated if one or more preambles
          was received. A SR indication will only be generated if one or more
          SR's was received. */ 
    uint16_t  rachAndSrReportMode;
    /* A margin between the end of the extraction window and the expected
       frame of the last Ts of the SC-FDMA symbol.
       Value: 0 to 127
       Defualt Vlaue: 0 */  
    uint16_t  extractionWindowMargin;
    /* This is the forgetting factor used in the IIR filtering for PUCCH
       noise power estimation.
       Value: 0 to 65535.
       The 16-bit value is a 16-bit unsigned number with 16 fractional bits.
       Default Value: 0x2000(corresponding to 1/8) */
    uint16_t  pucchNoiseEstimationGamma;
    /* The ration between the theshold for peak detection in PRACH to the average
       power of the correlator output 
       Value: 0 to 65535.
        - The 16-bit value is a 16-bit unsigned number with 16 fractional bits.
       Default Value:
       0x38cc(corresponding to 14.2)for single antenna
       0x22cc(corresponding to 8.7) for two antennas */
    uint16_t  prachFormat4PeakRatio;
    /* The ration between the theshold for peak detection in PRACH to the average
       power of the correlator output 
       Value: 0 to 65535.
       - The 16-bit value is a 16-bit unsigned number with 16 fractional bits.
       Default Value:
       0x38cc(corresponding to 14.2)for single antenna
       0x2266(corresponding to 8.6) for two antennas */
    uint16_t  prachFormat0PeakRatio;
    /* A compensation factor in the Doppler estimation,which tries to
       account for the effect of the SNR of the channel estimate based
       on SRS.
       - If the estimates of the signal power and noise power are perfect
       or accurate, the compensation factor of unity is preferred theortically.
       - However, in case of environment that these power estimates can hardly be
         accurate, smaller weighting imposed in the SNR of the channel estimate
         may improve the performance of doppler extimation.
       Value: 0 to 65535.
       - The 16-bit value is is a 16-bit unsigned number with 16 fractional bits.
       Default Value: 0xffff(corresponding to 1-(2^16)) */ 
    uint16_t  dopplerEstimationCompensationFactor;
    /* The value should be set to target DTX-to-ACK probability.
        - PHY will use this value to estimate the theshold in ACK_NACK
          detection algoritm in PUSCH to meet the target DTX-to-ACK probability.
       Value: 0 to 31
       Default Value: 12 */
    uint16_t  probabilityDtxAckPusch;
    /* The value should be set to target DTX-to-ACK probability.
        - PHY will use this value to estimate the theshold in ACK_NACK
          detection algoritm in PUCCH uint16_t format1, 1a and 1b to meet the
          target DTX-to-ACK probability.
       Value: 0 to 31
       Default Value: 12 */
    uint16_t  probabilityDtxAckPucchFormat1;
    /* The power of DCI with respect to9 the reference signal.
       Value:0 to 10000  represents -6dB to 4 dB in step 0.001dB */
    uint16_t  dciPowerOffset;

    /*
       The value should be used for choosing whether uplink payload will be allocated
                  by L2/L3 or PHY
    */
    uint16_t uplinkpayloadallocationMethod;

	/*	The value should be used for choosing how to maintain run-time synchronization
		0 = No run-time synchronization
		1 = PHY maintains run-time synchronization using PHY Network Monitoring Mode (NMM) feature only in TDD special subframe mode
		2 = L2/L3 manages run-time synchronization
		Default value: 1 */
	uint16_t runTimeSynchronizationMode;

	/*	The value should be used for setting the UL EARFCN
		Default value: 39150 */
	uint16_t ulEarfcn;	
    /*IRC MRC config*/
    uint16_t phy_IRC_MRC;

    /* PHY version number
            */
    uint16_t phyVersion0;

    uint16_t phyVersion1;
    
    #endif
}FAPI_cellConfigParam_un;

/* parametrs which can be present in Config.Request during PHY Reconfig*/
typedef union FAPI_cellReConfigParam_unT
{
     /* Normalized value levels (relative) to accommodate different 
     absolute Tx Power used by eNb.
     Value: 0 . 255 Representing 0dB to -63.75dB in -0.25dB step */
     uint16_t  refSignalPower;

     /* The future SFN/SF subframe where the TLVs included in the message 
     should be applied.A 16-bit value where,[15:4] SFN, range 0 . 1023
     [3:0] SF, range 0 . 9 */
     uint16_t  sfnsf;

    /*Added For Power Control*/
    /*Refers to downlink power allocation*/
    uint16_t pB;
    /*The power per antenna of the PCFICH with respect to the reference 
     * signal*/
    uint16_t pcfichPowerOffset;
    /*The power per antenna of the PHICH with respect to the reference 
     * signal*/
    uint16_t phichPowOffset;
    uint16_t primarySignal;
    uint16_t secondarySignal;

    /* + PRS_CHANGES */
     /*PRS bandwidth in resource blocks.
     Value: 6, 15, 25, 50, 75, 100*/
     uint8_t  prsBandWidth;

     /*The cyclic prefix used for PRS transmission.
     Value: 
     0: normal cyclic prefix
     1: extended cyclic prefix*/
     uint8_t  prsCyclicPrifix;
     /* - PRS_CHANGES */
}FAPI_cellReConfigParam_un;    
/* ------------------------- */

/* cell Config structure */
typedef struct FAPI_cellConfig_stT 
{
     /* tag values are defined as macro preprocessor */
     uint8_t                   tag; 

     /* tag length parameter ensures the complete TLV is a multiple
      * of 4-bytes (32-bits).  */
     uint8_t                   tagLen;

     FAPI_cellConfigParam_un   configParam;     
     
}FAPI_cellConfig_st;


/*cell ReConfig structure */
typedef struct FAPI_cellReConfig_stT
{
    /*tag values are defined as macro preprocessor */
    uint8_t                   tag;
    /* tag length parameter ensures the complete TLV is a multiple
      * of 4-bytes (32-bits).  */
    uint8_t                   tagLen;
    
    FAPI_cellReConfigParam_un configParam;
}FAPI_cellReConfig_st;

/* --------------------------------------------- */

/* UE CNFIG Possible Tags */
//#define     FAPI_HANDLE                                  100
//#define    FAPI_RNTI                                     101
///* CQI CONFIG */
//#define    FAPI_CQI_PUCCH_RESOURCE_INDEX                 102
//#define    FAPI_CQI_PMI_CONFIG_INDEX                     103
//#define    FAPI_CQI_RI_CONFIG_INDEX                      104
//#define    FAPI_CQI_SIMULTANEOUS_ACK_NACK_CQI            105
///* CQI CONFIG ENDS */
///* ACK/NACK CONFIG */
//#define    FAPI_AN_REPETITION_FACTOR                     106
//#define    FAPI_AN_N1_PUCCH_AN_REP                       107
//#define    FAPI_TDD_ACK_NACK_FEEDBACK                    108
///* ACK/NACK CONFIG  Ends*/
///* SRS CONFIG */
//#define    FAPI_SRS_BANDWIDTH                            109
//#define    FAPI_SRS_HOPPING_BANDWIDTH                    110
//#define    FAPI_FREQUENCY_DOMAIN_POSITION                111
//#define    FAPI_SRS_DURATION                             112
//#define    FAPI_ISRS_SRS_CONFIG_INDEX                    113
//#define    FAPI_TRANSMISSION_COMB                        114
//#define    FAPI_SOUNDING_REFERENCE_SYCLIC_SHIFT          115
///* SRS CONFIG  Ends */
///* SR CONFIG */
//#define    FAPI_SR_PUCCH_RESOURCE_INDEX                  116
//#define    FAPI_SR_CONFIG_INDEX                          117
///* SR CONFIG Ends */
///* SPS CONFIG */
//#define   FAPI_SPS_DL_CONFIG_SCHD_INTERVAL               118
//#define   FAPI_SPS_DL_N1_PUCCH_AN_PERSISTENT             119

/* add by lucas todo  */
#define    FAPI_RNTI                                     121

#define   FAPI_TRANSMISSION_MODE_PRESENT                 122
#define   FAPI_TRANSMISSION_MODE                         123

#define   FAPI_CQI_CONFIG_INFO_PRESENT                   124
#define   FAPI_APERIODIC_CQI_CFG_PRESENT                 125
#define   FAPI_APERIODIC_CQI_CFG_EN                      126
#define   FAPI_APERIODIC_CQI_REPORT_MODE                 127
#define   FAPI_PDSCH_EPRE_TO_UE_RS_RATIO                 128//????
#define   FAPI_PERIODIC_CQI_CFG_PRESENT                  129
#define   FAPI_PERIODIC_CQI_CFG_EN                       130
#define   FAPI_CQI_PUCCH_RESOURCE_INDEX                  131
#define   FAPI_CQI_PMI_CONFIG_INDEX                      132
#define   FAPI_CQI_RI_CONFIG_INDEX                       133
#define   FAPI_SIMULTANEOUS_ACK_NACK_AND_CQI             134
#define   FAPI_CQI_FORMAT_INDICATOR_PERIODIC             135
#define   FAPI_CQI_MASK_V920_EN				             136
#define   FAPI_CQI_MASK_V920				             137
#define   FAPI_PMI_RI_REPORT_V920_EN		             138
#define   FAPI_PMI_RI_REPORT_V920			             139
#define   FAPI_PERIODIC_RI_REPORT			             140
#define   FAPI_K                                         141

#define   FAPI_ACK_NACK_CONFIG_PRESENT                   142
#define   FAPI_AN_REPETITION_FACTOR                      143
#define   FAPI_AN_N1_PUCCH_AN_REP                        144
#define   FAPI_TDD_ACK_NACK_FEEDBACK_MODE_EN             145
#define   FAPI_TDD_ACK_NACK_FEEDBACK_MODE                146
#define   FAPI_AN_REPETITION_FACTOR_EN                   147

#define   FAPI_SRS_CONFIG_PRESENT                        148
#define   FAPI_FREQ_DOMAIN_POSITION                      149
#define   FAPI_SRS_CONFIG_INDEX                          150
#define   FAPI_SRS_CYCLIC_SHIFT                          151
#define   FAPI_SRS_DURATION                              152
#define   FAPI_SRS_HOPPING_BANDWIDTH                     153
#define   FAPI_SRS_BANDWIDTH                             154
#define   FAPI_SRS_EN                                    155
#define   FAPI_SRS_TRANSMISSION_COMB                     156

#define   FAPI_SR_CONFIG_PRESENT                         157
#define   FAPI_SR_CONFIG_INDEX                           158
#define   FAPI_SR_PUCCH_RESOURCE_INDEX                   159
#define   FAPI_SR_EN                                     160

#define   FAPI_SPS_DL_CONFIG_PRESENT                     161
#define   FAPI_SPS_DL_EN                                 162
#define   FAPI_SPS_DL_CONFIG_SCHED_INTERVAL              163
#define   FAPI_SPS_DL_N1_PUCCH_AN_PERSISTENT0            164
#define   FAPI_SPS_DL_N1_PUCCH_AN_PERSISTENT1            165
#define   FAPI_SPS_DL_N1_PUCCH_AN_PERSISTENT2            166
#define   FAPI_SPS_DL_N1_PUCCH_AN_PERSISTENT3            167

#define   FAPI_PUSCH_CONFIG_INFO_PRESENT                 168
#define   FAPI_PUSCH_CONFIG_BETAOFFSET_ACK_INDEX         169
#define   FAPI_PUSCH_CONFIG_BETAOFFSET_RI_INDEX          170
#define   FAPI_PUSCH_CONFIG_BETAOFFSET_CQI_INDEX         171


/* SPS CONFIG  Ends*/

/* UE Config parametrs which wiil be filled in FAPI_ueConfig_st */
typedef union FAPI_ueConfigParam_unT
{
    /* An opaque handling to associate the received information in 
    RX.indication */
  
    /* uint32_t  handle; */

    /* The RNTI used for identifying the UE when receiving the PDU.
    Value: 1 . 65535. */

    uint16_t  rnti;

    uint16_t transModePresent;
    /*
        TX_MODE_ATTACH_NOT_COMPLETE = 0,
        TX_MODE_1 = 1,
        TX_MODE_2,
        TX_MODE_3,
        TX_MODE_4,
        TX_MODE_5,
        TX_MODE_6,
        TX_MODE_7,
        TX_MODE_8,
        INVALID_TX_MODE
        */
    uint16_t transMode; /* UE ¡ä?¨º??¡ê¨º????¡§¨¢?DCI?¡ê¨º?¡ê????¡Â????o¨ª¨ª¡§1yPDCCHD?¨¢?¨ª¡§?a¦Ì?PDSCH¨ºy?Y¡ä?¨º?¡¤?¨º? */

    /* CQI Config */

    uint16_t cqiCfgPresent;

    uint16_t aperiodicCqiCfgPresent;

    uint16_t aperiodicCqiCfgEn;

    /*
    CQI_MODE_1_0 =10,
    CQI_MODE_1_1=11,
    CQI_MODE_1_2=12,
    CQI_MODE_2_0=20,
    CQI_MODE_2_1=21,
    CQI_MODE_2_2=22,
    CQI_MODE_3_0=30,
    CQI_MODE_3_1=31,
    CQI_MODE_INVALID = 255
    */
    uint16_t aperiodicCqiReportMode;

    /* -3~6 */
    uint16_t pdschEpreToUeRsRatio;

    uint16_t periodicCqiCfgPresent;

    uint16_t periodicCqiCfgEn;

    /* PUCCH resource for periodc CQI reporting
    Value: 0 . 1185 */

    uint16_t  cqiPucchResourceIndex;

    /* The periodic PMI reporting configuration.
    Value: 0 . 315. */

    uint16_t  cqiPmiConfigIndex;

    /* The periodic RI reporting configuration.
    Value: 0 . 965. */

    uint16_t  cqiRiConfigIndex;

    /* Indicates if simultaneous transmission of CQI and ACK/NACK is allowed.
    Value:
    0: no PUCCH Format 2a/2b
    1: PUCCH Format 2a/2b can be used */

    uint16_t  simultaneousAckNackAndCqi;

    /* 0: PERIODIC_WIDEBAND
     * 1: PERIODIC_SUBBAND
     * */
    uint16_t cqiformatIndicatorPeriodic;

    uint16_t cqiMask_v920_En;

    /* true
     * false
     * */
    uint16_t cqiMask_v920;

    uint16_t pmiRiReport_V920_En;
    /* true
     * false
     * */
    uint16_t pmiRiReport_V920;

    /* 1~4 */
    uint16_t cqiformatIndicatorPeriodic_subband_k;

    /* CQI Config ENDS */
 
    /* ACK/NACK Config */
    uint16_t ackNAckConfigPresent;

    /* The ACK/NACK repetition  factor.
    Value: 2,4,6 */
    
    uint16_t   ackNacknRepetitionFactor;

    /* The ACK/NACK repetition PUCCH resource index.
    Value: 0. 2047 */

    uint16_t  anN1PUCCHANRep;

    /* The TDD ACK/NACK Feedback Mode
    Value:
    0: bundling
    1: multiplexing */

    uint16_t   tddAckNackFeedbackMode;

    uint16_t   tddAckNackFeedbackModeEn;

    /* ACK/NACK Config Ends */

    /* SRS Config */

    uint16_t srsConfigPresent;

    uint16_t srsConfigEn;

    /*  SRS Bandwidth. This value is fixed for a UE and allocated in 
    RRC connection setup.
    Value: 0 . 3. */

    uint16_t   srSBandWidth;

    /* Configures the frequency hopping on the SRS. 
    This value is fixed for a UE and allocated in RRC connection setup.
    Value 0 . 3. */

    uint16_t   srsHoppingBandwidth;

    /* Frequency-domain position, NRRC This value is fixed for a UE and 
    allocated in RRC connection setup.
    Value: 0 . 23. */

    uint16_t   freqDomainPosition;

    /* The duration of the SRS configuration
    Value:
    0: once
    1: indefinite */

    uint16_t   srsDuration;

    /* Defines the periodicity and subframe location of the SRS.
    SRS Configuration Index. This value is fixed for a UE and 
    allocated in RRC connection setup
    Value: 0 . 644. */

    uint16_t   srsConfigIndex;

    /* Configures the frequency location of the SRS. 
    This value is fixed for a UE and allocated in RRC connection setup.
    Value: 0 .  1 */

    uint16_t   txComb;

    /* Configures the SRS sequence generation. 
    This value is fixed for a UE and allocated in RRC connection setup.
    Value: 0 . 7 */

    uint16_t   soundingRefCyclicShift;
    /* SRS Config Ends */

    /* SR Config */
    uint16_t srConfigPresent;

    uint16_t srConfigEn;

    /* Scheduling request PUCCH resource index
    Value: 0 . 2047. */
 
    uint16_t  srPucchResourceIndex;

    /* The scheduling request configuration index.
    Value: 0 . 157. */

    uint16_t   srConfigIndex;

    /* SR Config Ends */

    /* SPS Config */
    uint16_t spsDlConfigPresent;

    uint16_t spsDlEn;

    /* SPS Configuration Interval
    Value: 10,20,32,40,64,80,128,160,320,640 */

    uint16_t  spsDlConfigSchedInterval;

    /* The SPS PUCCH AN Resource configuration.
    The TLV can be repeated four times.
    Value: 0 . 2047. */

    uint16_t  spsDln1PUCCHANPersistent0;
    uint16_t  spsDln1PUCCHANPersistent1;
    uint16_t  spsDln1PUCCHANPersistent2;
    uint16_t  spsDln1PUCCHANPersistent3;

    /* SPS Config Ends */

    uint16_t  uePuschCfgPresent;
    uint16_t  betaOffsetACKIndex;  /* {0~15} */
    uint16_t  betaOffsetRIIndex;   /* {0~15} ¡Á¡é¡êo13¡ê?14¡ê?15?¦Ì¡À¡ê¨¢??¡ä¨º1¨®?*/
    uint16_t  betaOffsetCQIIndex;  /* {0~15}  ¡Á¡é¡êo0¡ê?1?¦Ì¡À¡ê¨¢??¡ä¨º1¨®? */

}FAPI_ueConfigParam_un;
/* -------------------------------   */

/* UE Config structure */
typedef struct FAPI_ueConfig_stT 
{
    /* tag define in  FAPI_ueConig_en */
    uint8_t   tag;

    /* tag length parameter ensures the complete TLV is a multiple of 4-bytes 
    (32-bits).  */
    uint8_t   tagLen;

    /* The Possible values can be of size uint16_t or uint32_t or uint8_t */
    /* For more details please see above parametrs wise in comment */ 
    FAPI_ueConfigParam_un   ueConfigParam;

}FAPI_ueConfig_st;

/* --------------------------------- */

typedef struct FAPI_paramResponseTLV_stT
{
     /* tag values are defined as macro preprocessor */
     uint8_t                   tag; 

     /* tag length parameter ensures the complete TLV is a multiple of 4-bytes (32-bits).  */
     uint8_t                   tagLen;
     
     FAPI_cellConfigParam_un   configParam;     
    
}FAPI_paramResponseTLV_st;


/* macro preprocessor definition for UL/DL config/tx request */

#define    FAPI_DCI_DL_PDU        0
#define    FAPI_BCH_PDU           1
#define    FAPI_MCH_PDU           2
#define    FAPI_DLSCH_PDU         3
#define    FAPI_PCH_PDU           4
#define    FAPI_PRS_PDU           5/* + PRS_CHANGES */

#define   FAPI_DL_DCI_FORMAT_1    0
#define   FAPI_DL_DCI_FORMAT_1A   1
#define   FAPI_DL_DCI_FORMAT_1B   2
#define   FAPI_DL_DCI_FORMAT_1C   3
#define   FAPI_DL_DCI_FORMAT_1D   4
#define   FAPI_DL_DCI_FORMAT_2    5
#define   FAPI_DL_DCI_FORMAT_2A   6
    /* + TM7_8 Changes Start */
#define   FAPI_DL_DCI_FORMAT_2B   7
    /* - TM7_8 Changes End */


#define      FAPI_UL_DCI_FORMAT_0     0
#define      FAPI_UL_DCI_FORMAT_3     1
#define      FAPI_UL_DCI_FORMAT_3A    2

#define       FAPI_HI_PDU                 0
#define       FAPI_DCI_UL_PDU             1

#define       FAPI_ULSCH                    0
#define       FAPI_ULSCH_CQI_RI             1
#define       FAPI_ULSCH_HARQ               2
#define       FAPI_ULSCH_CQI_HARQ_RI        3
#define       FAPI_UCI_CQI                  4
#define       FAPI_UCI_SR                   5
#define       FAPI_UCI_HARQ                 6
#define       FAPI_UCI_SR_HARQ              7
#define       FAPI_UCI_CQI_HARQ             8
#define       FAPI_UCI_CQI_SR               9
#define       FAPI_UCI_CQI_SR_HARQ          10
#define       FAPI_SRS                      11

#ifdef FAPI_1_1_COMPLIANCE
#define       FAPI_HARQ_BUFFER              12
#endif

#define       FAPI_INVALID_PDU_TYPE         255

/* structure for DCI FORMAT 1 */
typedef struct FAPI_dciFormat1_stT
{
    /* The aggregation level used
    Value: 1,2,4,8  */
    uint8_t         aggregationLevel;

    /* Resource allocation type
    Valid for DCI formats: 1,2,2A
    0=type 0
    1=type 1  */
    uint8_t         resAllocationType;

    /* The modulation and coding scheme for 1st transport block
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A
    Value: 0 . 31   */
    uint8_t         mcs_1;

    /* The redundancy version for 1st transport block.
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A
    Value: 0 . 3   */
    uint8_t         redundancyVersion_1;

    /* The encoding for the resource blocks. It's coding is dependent on 
    whether resource allocation type 0, 1, 2 is in use.
    Resource allocation type 0 is explicitly signalled for DCI formats 1, 2, 2A
    Resource allocation type 1 is explicitly signalled for DCI formats 1, 2, 2A
    Resource allocation type 2 is implicit for DCI formats 1A, 1B, 1C, 1D
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A   */
    uint32_t        rbCoding;

    /* The new data indicator for 1st transport block.
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A  */
    uint8_t         newDataIndicator_1;

    /* HARQ process number
    Valid for DCI formats: 1,1A,1B,1D,2,2A
    Value: 0 . 7  */
    uint8_t         harqProcessNum;


    /* Tx power control command for PUCCH.
    Valid for DCI formats: 1,1A,1B,1D,2,2A
    Value:  0,1,2,3 as represented in 36.213 Sec 5.1 for TPC Command Field.
    In case of DCI format 1A and RNTI=SI-RNTI,RA-RNTI or P-RNTI 
    the TPC values are either 0,1 representing N_PRB=2 and N_PRB =3 
    respectively. */
    uint8_t         tpc;

    /* The downlink assignment index. Only used in TDD mode, value ignored 
    for FDD.Valid for DCI formats: 1,1A,1B,1D,2,2A
    Value: 1,2,3,4  */
    uint8_t         dlAssignmentIndex;
    
    /* Offset to the reference signal power.
    Value: 0 . 10000, representing -6 dB to 4 dB in 0.001 dB steps. */
    uint16_t        txPower;

    /* RNTI type Valid for DCI format 1, 1A,2,2A
    1  =  C-RNTI
    2 =  RA-RNTI, P-RNTI, or SI-RNTI.
    3 = SPS-CRNTI   */
    uint8_t         rntiType;

    /* Padding required for 2 bytes */
    uint8_t         padding[1];

}FAPI_dciFormat1_st;

/* ---------------------------------- */

/* structure for DCI FORMAT 1A */
typedef struct FAPI_dciFormat1A_stT
{
    /* The aggregation level used
    Value: 1,2,4,8  */
    uint8_t         aggregationLevel;

    /* Type of virtual resource block used
    Valid for DCI formats: 1A,1B,1D

    0 = localized
    1 = distributed  */
    uint8_t         vRBAssignmentFlag;

    /* The modulation and coding scheme for 1st transport block
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A
    Value: 0 . 31   */
    uint8_t         mcs_1;

    /* The redundancy version for 1st transport block.
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A
    Value: 0 . 3   */
    uint8_t         redundancyVersion_1;

    /* The encoding for the resource blocks. It's coding is dependent on 
    whether resource allocation type 0, 1, 2 is in use.
    Resource allocation type 0 is explicitly signalled for DCI formats 1, 2, 2A
    Resource allocation type 1 is explicitly signalled for DCI formats 1, 2, 2A
    Resource allocation type 2 is implicit for DCI formats 1A, 1B, 1C, 1D
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A   */
    uint32_t        rbCoding;

    /* The new data indicator for 1st transport block.
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A  */
    uint8_t         newDataIndicator_1;

    /* HARQ process number
    Valid for DCI formats: 1,1A,1B,1D,2,2A
    Value: 0 . 7  */
    uint8_t         harqProcessNum;

    /* Tx power control command for PUCCH.
    Valid for DCI formats: 1,1A,1B,1D,2,2A
    Value:  0,1,2,3 as represented in 36.213 Sec 5.1 for TPC Command Field.
    In case of DCI format 1A and RNTI=SI-RNTI,RA-RNTI or P-RNTI 
    the TPC values are either 0,1 representing N_PRB=2 and N_PRB =3 
    respectively. */
    uint8_t         tpc;

    /* The downlink assignment index. Only used in TDD mode, value ignored 
    for FDD.Valid for DCI formats: 1,1A,1B,1D,2,2A
    Value: 1,2,3,4  */
    uint8_t         dlAssignmentIndex;
    
    /* Indicates that PRACH procedure is initiated Valid for DCI formats: 1A
    0 = false
    1=true   */
    uint8_t         allocatePRACHFlag;

    /* The preamble index to be used on the PRACH Valid for DCI formats: 1A
    Value: 0 . 63   */
    uint8_t         preambleIndex;

    /* Offset to the reference signal power.
    Value: 0 . 10000, representing -6 dB to 4 dB in 0.001 dB steps. */
    uint16_t        txPower;

    /* The mask index to be used on the PRACH Valid for DCI formats: 1A
    Value: 0 . 15  */
    uint8_t         pRACHMaskIndex;

    /* RNTI type Valid for DCI format 1, 1A,2,2A
    1  =  C-RNTI
    2 =  RA-RNTI, P-RNTI, or SI-RNTI.
    3 = SPS-CRNTI   */
    uint8_t         rntiType;
#ifdef ASTRI_INTF
    uint8_t         n_gap; 
    uint8_t padding[1];
#else
    uint8_t 		nGap;
    uint8_t 		padding[1];
#endif
}FAPI_dciFormat1A_st;

/* ---------------------------------- */

/* structure for DCI FORMAT 1B */
typedef struct FAPI_dciFormat1B_stT
{
    /* The aggregation level used
    Value: 1,2,4,8  */
    uint8_t         aggregationLevel;

    /* Type of virtual resource block used
    Valid for DCI formats: 1A,1B,1D

    0 = localized
    1 = distributed  */
    uint8_t         vRBAssignmentFlag;

    /* The modulation and coding scheme for 1st transport block
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A
    Value: 0 . 31   */
    uint8_t         mcs_1;

    /* The redundancy version for 1st transport block.
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A
    Value: 0 . 3   */
    uint8_t         redundancyVersion_1;

    /* The encoding for the resource blocks. It's coding is dependent on 
    whether resource allocation type 0, 1, 2 is in use.
    Resource allocation type 0 is explicitly signalled for DCI formats 1, 2, 2A
    Resource allocation type 1 is explicitly signalled for DCI formats 1, 2, 2A
    Resource allocation type 2 is implicit for DCI formats 1A, 1B, 1C, 1D
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A   */
    uint32_t        rbCoding;

    /* The new data indicator for 1st transport block.
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A  */
    uint8_t         newDataIndicator_1;

    /* HARQ process number
    Valid for DCI formats: 1,1A,1B,1D,2,2A
    Value: 0 . 7  */
    uint8_t         harqProcessNum;

    /* The codebook index to be used for precoding
    Valid for DCI formats: 1B,1D 
    2 antenna_ports: 0 . 3
    4 antenna_ports: 0 . 15  */
    uint8_t         tPMI;

    /* Confirmation for precoding
    Valid for DCI formats: 1B
    0 = use precoding indicated in TPMI field
    1 = use precoding indicated in last PMI report on PUSCH  */
    uint8_t         pmi;

    /* Tx power control command for PUCCH.
    Valid for DCI formats: 1,1A,1B,1D,2,2A
    Value:  0,1,2,3 as represented in 36.213 Sec 5.1 for TPC Command Field.
    In case of DCI format 1A and RNTI=SI-RNTI,RA-RNTI or P-RNTI 
    the TPC values are either 0,1 representing N_PRB=2 and N_PRB =3 
    respectively. */
    uint8_t         tpc;

    /* The downlink assignment index. Only used in TDD mode, value ignored 
    for FDD.Valid for DCI formats: 1,1A,1B,1D,2,2A
    Value: 1,2,3,4  */
    uint8_t         dlAssignmentIndex;
    
    /* Offset to the reference signal power.
    Value: 0 . 10000, representing -6 dB to 4 dB in 0.001 dB steps. */
    uint16_t        txPower;

    /* Used in virtual resource block distribution
    Valid for DCI formats: 1B,1C,1D
    0= NGAP1
    1=  NGAP2   */
    uint8_t         nGAP;

    uint8_t padding[3];
}FAPI_dciFormat1B_st;

/* ---------------------------------- */

/* structure for DCI FORMAT 1C */
typedef struct FAPI_dciFormat1C_stT
{
    /* The aggregation level used
    Value: 1,2,4,8  */
    uint8_t         aggregationLevel;

    /* The modulation and coding scheme for 1st transport block
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A
    Value: 0 . 31   */
    uint8_t         mcs_1;

    /* The redundancy version for 1st transport block.
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A
    Value: 0 . 3   */
    uint8_t         redundancyVersion_1;

    /* The new data indicator for 1st transport block.
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A  */
    uint8_t         newDataIndicator_1;

    /* The encoding for the resource blocks. It's coding is dependent on 
    whether resource allocation type 0, 1, 2 is in use.
    Resource allocation type 0 is explicitly signalled for DCI formats 1, 2, 2A
    Resource allocation type 1 is explicitly signalled for DCI formats 1, 2, 2A
    Resource allocation type 2 is implicit for DCI formats 1A, 1B, 1C, 1D
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A   */
    uint32_t        rbCoding;

    /* Used in virtual resource block distribution
    Valid for DCI formats: 1B,1C,1D
    0= NGAP1
    1=  NGAP2   */
    uint8_t         nGAP;

    /* The transport block size
    Valid for DCI formats: 1C
    Value: 0 . 31   */
    uint8_t         tbSizeIndex;

    /* Offset to the reference signal power.
    Value: 0 . 10000, representing -6 dB to 4 dB in 0.001 dB steps. */
    uint16_t        txPower;

}FAPI_dciFormat1C_st;

/* ---------------------------------- */

/* structure for DCI FORMAT 1D */
typedef struct FAPI_dciFormat1D_stT
{
    /* The aggregation level used
    Value: 1,2,4,8  */
    uint8_t         aggregationLevel;

    /* Type of virtual resource block used
    Valid for DCI formats: 1A,1B,1D

    0 = localized
    1 = distributed  */
    uint8_t         vRBAssignmentFlag;

    /* The modulation and coding scheme for 1st transport block
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A
    Value: 0 . 31   */
    uint8_t         mcs_1;

    /* The redundancy version for 1st transport block.
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A
    Value: 0 . 3   */
    uint8_t         redundancyVersion_1;

    /* The encoding for the resource blocks. It's coding is dependent on 
    whether resource allocation type 0, 1, 2 is in use.
    Resource allocation type 0 is explicitly signalled for DCI formats 1, 2, 2A
    Resource allocation type 1 is explicitly signalled for DCI formats 1, 2, 2A
    Resource allocation type 2 is implicit for DCI formats 1A, 1B, 1C, 1D
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A   */
    uint32_t        rbCoding;

    /* The new data indicator for 1st transport block.
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A  */
    uint8_t         newDataIndicator_1;

    /* HARQ process number
    Valid for DCI formats: 1,1A,1B,1D,2,2A
    Value: 0 . 7  */
    uint8_t         harqProcessNum;

    /* The codebook index to be used for precoding
    Valid for DCI formats: 1B,1D 
    2 antenna_ports: 0 . 3
    4 antenna_ports: 0 . 15  */
    uint8_t         tPMI;

    /* Tx power control command for PUCCH.
    Valid for DCI formats: 1,1A,1B,1D,2,2A
    Value:  0,1,2,3 as represented in 36.213 Sec 5.1 for TPC Command Field.
    In case of DCI format 1A and RNTI=SI-RNTI,RA-RNTI or P-RNTI 
    the TPC values are either 0,1 representing N_PRB=2 and N_PRB =3 
    respectively. */
    uint8_t         tpc;

    /* The downlink assignment index. Only used in TDD mode, value ignored 
    for FDD.Valid for DCI formats: 1,1A,1B,1D,2,2A
    Value: 1,2,3,4  */
    uint8_t         dlAssignmentIndex;
    
    /* Used in virtual resource block distribution
    Valid for DCI formats: 1B,1C,1D
    0= NGAP1
    1=  NGAP2   */
    uint8_t         nGAP;

    /* Offset to the reference signal power.
    Value: 0 . 10000, representing -6 dB to 4 dB in 0.001 dB steps. */
    uint16_t        txPower;

    /* Indicates the DL power offset type for multi-user MIMO transmission
    Valid for DCI formats: 1D
    Value: 0 . 1   */
    uint8_t         dlPowerOffset;

    uint8_t padding[3];
}FAPI_dciFormat1D_st;

/* ---------------------------------- */

/* structure for DCI FORMAT 2 */
typedef struct FAPI_dciFormat2_stT
{
    /* The aggregation level used
    Value: 1,2,4,8  */
    uint8_t         aggregationLevel;

    /* Resource allocation type
    Valid for DCI formats: 1,2,2A
    0=type 0
    1=type 1  */
    uint8_t         resAllocationType;

    /* The modulation and coding scheme for 1st transport block
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A
    Value: 0 . 31   */
    uint8_t         mcs_1;

    /* The redundancy version for 1st transport block.
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A
    Value: 0 . 3   */
    uint8_t         redundancyVersion_1;

    /* The encoding for the resource blocks. It's coding is dependent on 
    whether resource allocation type 0, 1, 2 is in use.
    Resource allocation type 0 is explicitly signalled for DCI formats 1, 2, 2A
    Resource allocation type 1 is explicitly signalled for DCI formats 1, 2, 2A
    Resource allocation type 2 is implicit for DCI formats 1A, 1B, 1C, 1D
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A   */
    uint32_t        rbCoding;

    /* The new data indicator for 1st transport block.
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A  */
    uint8_t         newDataIndicator_1;

    /* Indicates the mapping of transport block to codewords
    Valid for DCI formats: 2,2A
    0 = no swapping
    1 = swapped    */
    uint8_t         tbToCodeWordSwapFlag;

    /* The modulation and coding scheme for 2nd transport block. 
    Valid for DCI formats: 2,2A
    Value: 0 . 31    */
    uint8_t         mcs_2;

    /* The redundancy version for 2nd transport block. 
    Valid for DCI formats: 2,2A
    Value: 0 . 3   */
    uint8_t         redundancyVersion_2;

    /* The new data indicator for 2nd transport block.
    Valid for DCI formats: 2,2A */
    uint8_t         newDataIndicator_2;

    /* HARQ process number
    Valid for DCI formats: 1,1A,1B,1D,2,2A
    Value: 0 . 7  */
    uint8_t         harqProcessNum;

    /* Precoding information
    Valid for DCI formats: 2,2A
    2 antenna_ports: 0 . 7
    4 antenna_ports: 0 . 63   */
    uint8_t         preCodingInfo;

    /* Tx power control command for PUCCH.
    Valid for DCI formats: 1,1A,1B,1D,2,2A
    Value:  0,1,2,3 as represented in 36.213 Sec 5.1 for TPC Command Field.
    In case of DCI format 1A and RNTI=SI-RNTI,RA-RNTI or P-RNTI 
    the TPC values are either 0,1 representing N_PRB=2 and N_PRB =3 
    respectively. */
    uint8_t         tpc;

    /* Offset to the reference signal power.
    Value: 0 . 10000, representing -6 dB to 4 dB in 0.001 dB steps. */
    uint16_t        txPower;

    /* The downlink assignment index. Only used in TDD mode, value ignored 
    for FDD.Valid for DCI formats: 1,1A,1B,1D,2,2A
    Value: 1,2,3,4  */
    uint8_t         dlAssignmentIndex;
    
    /* RNTI type Valid for DCI format 1, 1A,2,2A
    1  =  C-RNTI
    2 =  RA-RNTI, P-RNTI, or SI-RNTI.
    3 = SPS-CRNTI   */
    uint8_t         rntiType;

}FAPI_dciFormat2_st;

/* ---------------------------------- */

/* structure for DCI FORMAT 2A */
typedef struct FAPI_dciFormat2A_stT
{
    /* The aggregation level used
    Value: 1,2,4,8  */
    uint8_t         aggregationLevel;

    /* Resource allocation type
    Valid for DCI formats: 1,2,2A
    0=type 0
    1=type 1  */
    uint8_t         resAllocationType;

    /* The modulation and coding scheme for 1st transport block
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A
    Value: 0 . 31   */
    uint8_t         mcs_1;

    /* The redundancy version for 1st transport block.
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A
    Value: 0 . 3   */
    uint8_t         redundancyVersion_1;

    /* The encoding for the resource blocks. It's coding is dependent on 
    whether resource allocation type 0, 1, 2 is in use.
    Resource allocation type 0 is explicitly signalled for DCI formats 1, 2, 2A
    Resource allocation type 1 is explicitly signalled for DCI formats 1, 2, 2A
    Resource allocation type 2 is implicit for DCI formats 1A, 1B, 1C, 1D
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A   */
    uint32_t        rbCoding;

    /* The new data indicator for 1st transport block.
    Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A  */
    uint8_t         newDataIndicator_1;

    /* Indicates the mapping of transport block to codewords
    Valid for DCI formats: 2,2A
    0 = no swapping
    1 = swapped    */
    uint8_t         tbToCodeWordSwapFlag;

    /* The modulation and coding scheme for 2nd transport block. 
    Valid for DCI formats: 2,2A
    Value: 0 . 31    */
    uint8_t         mcs_2;

    /* The redundancy version for 2nd transport block. 
    Valid for DCI formats: 2,2A
    Value: 0 . 3   */
    uint8_t         redundancyVersion_2;

    /* The new data indicator for 2nd transport block.
    Valid for DCI formats: 2,2A */
    uint8_t         newDataIndicator_2;

    /* HARQ process number
    Valid for DCI formats: 1,1A,1B,1D,2,2A
    Value: 0 . 7  */
    uint8_t         harqProcessNum;

    /* Precoding information
    Valid for DCI formats: 2,2A
    2 antenna_ports: 0 . 7
    4 antenna_ports: 0 . 63   */
    uint8_t         preCodingInfo;

    /* Tx power control command for PUCCH.
    Valid for DCI formats: 1,1A,1B,1D,2,2A
    Value:  0,1,2,3 as represented in 36.213 Sec 5.1 for TPC Command Field.
    In case of DCI format 1A and RNTI=SI-RNTI,RA-RNTI or P-RNTI 
    the TPC values are either 0,1 representing N_PRB=2 and N_PRB =3 
    respectively. */
    uint8_t         tpc;

    /* Offset to the reference signal power.
    Value: 0 . 10000, representing -6 dB to 4 dB in 0.001 dB steps. */
    uint16_t        txPower;

    /* The downlink assignment index. Only used in TDD mode, value ignored 
    for FDD.Valid for DCI formats: 1,1A,1B,1D,2,2A
    Value: 1,2,3,4  */
    uint8_t         dlAssignmentIndex;
    
    /* RNTI type Valid for DCI format 1, 1A,2,2A
    1  =  C-RNTI
    2 =  RA-RNTI, P-RNTI, or SI-RNTI.
    3 = SPS-CRNTI   */
    uint8_t         rntiType;

}FAPI_dciFormat2A_st;

/* ---------------------------------- */

/* + TM7_8 Changes Start */
typedef struct FAPI_dciFormat2B_stT
{
    /* The aggregation level used
     * Value: 1,2,4,8  */
    uint8_t         aggregationLevel;

    /* Resource allocation type
     * Valid for DCI formats: 1,2,2A, 2B
     * 0=type 0
     * 1=type 1  */
    uint8_t         resAllocationType;

    /* The modulation and coding scheme for 1st transport block
     * Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A, 2B
     * Value: 0 . 31   */
    uint8_t         mcs_1;

    /* The redundancy version for 1st transport block.
     * Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A, 2B
     * Value: 0 . 3   */
    uint8_t         redundancyVersion_1;

    /* The encoding for the resource blocks. It's coding is dependent on
     * whether resource allocation type 0, 1, 2 is in use.
     * Resource allocation type 0 is explicitly signalled for DCI formats 1, 2, 2A, 2B
     * Resource allocation type 1 is explicitly signalled for DCI formats 1, 2, 2A, 2B
     * Resource allocation type 2 is implicit for DCI formats 1A, 1B, 1C, 1D
     * Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A, 2B   */
    uint32_t        rbCoding;

    /* The new data indicator for 1st transport block.
     * Valid for DCI formats: 1,1A,1B,1C,1D ,2,2A, 2B  */
    uint8_t         newDataIndicator_1;

    /* The modulation and coding scheme for 2nd transport block.
     * Valid for DCI formats: 2,2A, 2B
     * Value: 0 . 31    */
    uint8_t         mcs_2;

    /* The redundancy version for 2nd transport block.
     * Valid for DCI formats: 2,2A, 2B
     * Value: 0 . 3   */
    uint8_t         redundancyVersion_2;

    /* The new data indicator for 2nd transport block.
     * Valid for DCI formats: 2,2A, 2B */
    uint8_t         newDataIndicator_2;

    /* HARQ process number
     * Valid for DCI formats: 1,1A,1B,1D,2,2A, 2B
     * Value: 0 . 7  */
    uint8_t         harqProcessNum;
    /* Scrambling identity */
    uint8_t         scramblingId;

    /* Tx power control command for PUCCH.
     * Valid for DCI formats: 1,1A,1B,1D,2,2A, 2B
     * Value:  0,1,2,3 as represented in 36.213 Sec 5.1 for TPC Command Field.
     * In case of DCI format 1A and RNTI=SI-RNTI,RA-RNTI or P-RNTI
     * the TPC values are either 0,1 representing N_PRB=2 and N_PRB =3
     * respectively. */
    uint8_t         tpc;
    
    /* The downlink assignment index. Only used in TDD mode, value ignored
     * for FDD.Valid for DCI formats: 1,1A,1B,1D,2,2A, 2B
     * Value: 1,2,3,4  */
    uint8_t         dlAssignmentIndex;

    /* Offset to the reference signal power.
     * Value: 0 . 10000, representing -6 dB to 4 dB in 0.001 dB steps. */
    uint16_t        txPower;

    /* RNTI type Valid for DCI format 1, 1A,2,2A, 2B
     * 1  =  C-RNTI
     * 2 =  RA-RNTI, P-RNTI, or SI-RNTI.
     * 3 = SPS-CRNTI   */
    uint8_t         rntiType;

    uint8_t   padding[1];
}FAPI_dciFormat2B_st;

/* - TM7_8 Changes End */
/* ---------------------------------- */

/* DCI PDU Structure */
typedef struct FAPI_dciDLPduInfo_st
{
    /* Format of the DCI
    0 = 1
    1 = 1A
    2 = 1B
    3 = 1C
    4 = 1D
    5 = 2
    6 = 2A */
    uint8_t         dciFormat; /* defined in  FAPI_dlDCIFormatInfo_en. */

    /* CCE index used to send the DCI. 
    Value: 0 . 88 */
    uint8_t         cceIndex;

    /* The RNTI used for identifying the UE when receiving the PDU
    Valid for all DCI formats
    Value: 1 . 65535. */
    uint16_t        rnti;

    /* this dci PDU can be of any structre mentioned above based 
    upon dciFormat type. Parsing entity is required to typecast this 
    with apropriate DCI Format structure */
    uint8_t         dciPdu[FAPI_VAR_SIZE(x)];

}FAPI_dciDLPduInfo_st;

/* ---------------------------------- */

/* BCH PDU structure */
typedef struct FAPI_bchConfigPDUInfo_stT
{
    /* The length (in bytes) of the associated MAC PDU, delivered in 
    TX.request. This should be the actual length of the MAC PDU, 
    which may not be a multiple of 32-bits. */
    uint16_t         bchPduLen;
    
    /* This is a count value which is incremented every time a BCH, MCH, PCH or
    DLSCH PDU is included in the DL_CONFIG.request message.
    This value is repeated in TX.request and associates the control information
    to the data.It is reset to 0 for every subframe.
    Range 0 . 65535 */
    uint16_t         pduIndex;

    /* Offset to the reference signal power.
    Value: 0 . 10000, representing -6 dB to 4 dB in 0.001 dB steps.  */
    uint16_t         txPower;

    uint8_t          padding[2];
}FAPI_bchConfigPDUInfo_st;

/* ---------------------------------- */

/* MCH CONFIG PDU structure */
typedef struct FAPI_mchConfigPDUInfo_stT
{
    /* The length (in bytes) of the associated MAC PDU, delivered in 
    TX.request.  This should be the actual length of the MAC PDU, 
    which may not be a multiple of 32-bits  */
    uint16_t         mchPduLen;

    /* This is a count value which is incremented every time a BCH, MCH, PCH or
    DLSCH PDU is included in the DL_CONFIG.request message.
    This value is repeated in TX.request and associates the control information
    to the data.It is reset to 0 for every subframe.
    Range 0 . 65535    */
    uint16_t         pduIndex;

    /* The RNTI associated with the MCH
    Value: 1 . 65535.  */
    uint16_t         rnti;

    /* Resource allocation type
    0 = type 0
    1 = type 1
    2 = type 2   */
    uint8_t          resAllocationType;

    /* The modulation type used in the transport channels
    0: QPSK
    1: QAM16
    2: QAM64  */
    uint8_t          modulationType;

    /* The encoding for the resource blocks. It's coding is dependent on 
    whether resource allocation type 0, 1, 2 is in use. */
    uint32_t         rbCoding;
 
    /* Offset to the reference signal power.
    Value: 0 . 10000, representing -6 dB to 4 dB in 0.001 dB steps. */
    uint16_t         txPower;

    uint8_t          padding[2];
}FAPI_mchConfigPDUInfo_st;

/* ---------------------------------- */

/* BEAM Froming Vector Structure */
typedef struct FAPI_beamFormingVectorInfo_stT
{
    /* Index of subband for which the following beam forming vector is
    applied.         */
    uint8_t         subBandIndex;

    /* Number of physical antennas */
    uint8_t         numOfAntenna;

    /* For each physical antenna */
    /* Beam forming vector element for physical antenna 
    #i real 8 bits followed by imaginery 8 bits  */
    uint16_t        bfValue_per_antenna[FAPI_VAR_SIZE(numOfAntenna)];

}FAPI_beamFormingVectorInfo_st;

/* ---------------------------------- */

/* DLSCH CONFIG structure */
typedef struct FAPI_dlSCHConfigPDUInfo_stT
{
    /* The length (in bytes) of the associated MAC PDU, delivered in 
    TX.request. This should be the actual length of the MAC PDU, 
    which may not be a multiple of 32-bits. */
    uint16_t                 dlschPduLen;

    /* This is a count value which is incremented every time a BCH, MCH, PCH 
    or DLSCH PDU is included in the DL_CONFIG.request message.
    This value is repeated in TX.request and associates the control information
    to the data.It is reset to 0 for every subframe. 
    Range 0 . 65535   */
    uint16_t                 pduIndex;

    /* The RNTI associated with the UE
    Value: 1 . 65535. */
    uint16_t                 rnti;

    /* Resource allocation type
    0 = type 0
    1 = type 1
    2 = type 2  */
    uint8_t                  resAllocationType;

    /* Type of virtual resource block used. This should match the value sent in
    the DCI Format 1A, 1B, 1D PDU which allocated this grant.
    0 = localized
    1 = distributed     */
    uint8_t                  vRBAssignmentFlag;

    /* The encoding for the resource blocks. It's coding is dependent on 
    whether resource allocation type 0, 1, 2 is in use. This should match 
    the value sent in the DCI Format PDU which allocated this grant. */
    uint32_t                 rbCoding;

    /* 2: QPSK
    4: 16QAM
    6: 64QAM   */
    uint8_t                  mcs;

    /* HARQ redundancy version. This should match the value sent in the 
    DCI Format PDU which allocated this grant.
    Value: 0 . 3.   */
    uint8_t                  redundancyVersion;

    /* The transport block transmitted to this RNTI. A value of 2 indicates 
    this is the second transport block for either DCI format 2 or 2A. 
    For other DCI values this field will always be 1.
    Value: 1 . 2  */
    uint8_t                  transportBlocks;

    /* Indicates the mapping of transport block to codewords. 
    This should match the value sent in the DCI Format 2, 2A PDU which 
    allocated this grant.
    0 = no swapping
    1 = swapped      */
    uint8_t                  tbToCodeWordSwapFlag;

    /* The MIMO mode used in the PDU
 
    0: SINGLE_ANTENNA_PORT_0, 
    1: TX_DIVERSITY, 
    2: LARGE_DELAY_CDD, 
    3: CLOSED_LOOP_SPATIAL_MULTIPLEXING, 
    4: MULTI_USER_MIMO, 
    5: CLOSED_LOOP_RANK_1_PRECODING, 
    6: SINGLE_ANTENNA_PORT_5.  */
    uint8_t                  transmissionScheme;
#ifndef OS_LINUX
    uint8_t                  transmissionMode;
#endif
    /* The number of layers used in transmission.
    Value: 1 . 4    */    
    uint8_t                  numOfLayers;

    /* Only valid when transmission scheme = 3, 4, 5.
    Defines the number of subbands and codebooks used for PMI. 
    If value=1 then a single PMI value is supplied which should be 
    used over all RB.
    Value 0 -> 13  */
    uint8_t                  numOfSubBand;
    
   /*} */

    /* The UE capabilities category
    Value:1 . 5   */
    uint8_t                  ueCatagoryCapacity;

    /* The ratio of PDSCH EPRE to cell-specific RS EPRE among PDSCH REs 
    in all the OFDM symbols not containing cell-specific RS in dB.
    0: -6dB
    1: -4.77dB
    2: -3dB
    3: -1.77dB
    4: 0dB
    5: 1dB
    6: 2dB
    7: 3dB          */
    uint8_t                  pA;

    /* Delta power offset, value: 0..1, 
    Refer to: Table 7.1.5-1 in [6] for Multi-user MIMO mode. 
    Takes value zero for all other modes. */
    uint8_t                  deltaPowerOffsetAIndex;

    /* Used in virtual resource block distribution
    0= NGAP1
    1=  NGAP2   */
    uint8_t                  nGap;

    /* Used with DCI format 1A and RNTI=SI-RNTI or RA-RNTI. 
    This should match the value sent in the TPC field of the 
    DCI 1A PDU which allocated this grant.
    0: NPRB = 2 
    1: NPRB = 3   */
    uint8_t                  nPRB;

    /* Number of PRBs that are treated as one subband */
    uint16_t                 numRbPerSubBand;
    
    /* Number of beam forming vectors
    One beam forming vector is specified for each subband */
    uint16_t                 numbfVector;
    
#ifdef ASTRI_INTF
    uint8_t                  transmissionMode;  
    uint8_t                  padding[3];		
#endif
#ifdef FAPI_DEVIATIONS    
    /* + TM7_8 Changes Start */
    /* used with DCI format 2B and 2C , valid valus are 0 and 1 */
    uint8_t                  nScId;
    /*Ratio of PDSCH EPRE to UE specific Reference signal
     *Valid only when Transmission Mode is 7 and modulation scheme is QPSK */
    /* + TM7_EPRE */
    uint8_t                  padding[3];    
    /* - TM7_EPRE */
    /* - TM7_8 Changes End */
#endif    

#ifndef OS_LINUX   
    /* added by j.zhou */
    uint8_t         dciFormat;
    uint8_t			dciPadding[3];
#endif
 
    /* subBandInfo contains codebook indices per numOfSubBand{ */
    /* Only valid when transmission scheme = 3, 4, 5.
    Defines the codebook used.
    When antenna port = 1: NA
    When antenna port = 2: 0..3
    When antenna port = 4: 0..15   */
    uint8_t                  subBandInfo[FAPI_VAR_SIZE(numOfSubBand)];
   
    /* Beam forming vectors */
    FAPI_beamFormingVectorInfo_st     bfVector[FAPI_VAR_SIZE(numbfVector)];

}FAPI_dlSCHConfigPDUInfo_st;

/* ---------------------------------- */

/* Paging config structure */
typedef struct FAPI_pchPduConfigInfo_stT
{
    /* The length (in bytes) of the associated MAC PDU, delivered in 
    TX.request. This should be the actual length of the MAC PDU, 
    which may not be a multiple of 32-bits. */
    uint16_t           pchPduLen;

    /* This is a count value which is incremented every time a BCH, MCH, PCH 
    or DLSCH PDU is included in the DL_CONFIG.request message. This value is 
    repeated in TX.request and associates the control information to the data.
    It is reset to 0 for every subframe.
    Range 0 . 65535   */
    uint16_t           pduIndex;

    /* The P-RNTI associated with the paging
     Value: 0xFFFE  */
    uint16_t           pRNTI;

    /* Resource allocation type
    2 = type 2  */
    uint8_t            resAllocationType;

    /* Type of virtual resource block used. This should match the value sent 
    in the DCI Format 1A, 1B, 1D PDU which allocated this grant.
    0 = localized
    1 = distributed    */
    uint8_t            vRBAssignmentFlag;

    /* The encoding for the resource blocks. It's coding is dependent on 
    whether resource allocation type 0, 1, 2 is in use. This should match
    the value sent in the DCI Format PDU which allocated this grant. */
    uint32_t           rbCoding;
   
    /* For PCH PDU only QPSK modulation is allowed.
    0: QPSK   */
    uint8_t            mcs;

    /* For PCH PDU only redundancy version 0 is allowed
    Value: 0    */
    uint8_t            redundancyVersion;

    /* The number of transport blocks transmitted to this RNTI. 
    Only 1 transport block is sent on the PCH per subframe.
    Value: 1    */
    uint8_t            numOftransportBlocks;

    /* Reserved.  This parameter is not used on the PCH transport channel */
    uint8_t            tbToCodeWordSwapFlag;
 
    /* The MIMO mode used in the PDU
    0: SINGLE_ANTENNA_PORT_0, 
    1: TX_DIVERSITY, 
    6: SINGLE_ANTENNA_PORT_5.    */
    uint8_t            transmissionScheme;

    /* The number of layers used in transmission
    Value: 1 . 4    */
    uint8_t            numOfLayers;

    /* Reserved.  This parameter is not used on the PCH transport channel. */
    uint8_t            codeBookIndex;

    /* Reserved.  This parameter is not used on the PCH transport channel. */
    uint8_t            ueCatagoryCapacity;

    /* The ratio of PDSCH EPRE to cell-specific RS EPRE among PDSCH REs in 
    all the OFDM symbols not containing cell-specific RS in dB.
    0: -6dB
    1: -4.77dB
    2: -3dB
    3: -1.77dB
    4: 0dB
    5: 1dB
    6: 2dB
    7: 3dB    */
    uint8_t            pA;

    /* Used with DCI 1A format. This should match the value sent in the 
    TPC field of the DCI 1A PDU which allocated this grant.
    0: NPRB = 2 
    1: NPRB = 3    */
    uint8_t            nPRB;

    /* Offset to the reference signal power.
    Value: 0 . 10000, representing -6 dB to 4 dB in 0.001 dB steps.  */
    uint16_t           txPower;
 
#ifdef INTF_9131
        /*Used in virtual resource block distribution 
          0= NGAP1
          1= NGAP2 */ 
     uint8_t             nGap;
     uint8_t             padding[3];
#endif

#ifndef OS_LINUX
     uint8_t			dciFormat;
     uint8_t			dciPadding[3];
#endif

}FAPI_pchPduConfigInfo_st;
/* + PRS_CHANGES */
typedef struct FAPI_prsPduConfigInfo_stT
{
    uint16_t transmissionPower;
}FAPI_prsPduConfigInfo_st;
/* - PRS_CHANGES */
/* ---------------------------------- */

/* DL CONFIG union */
typedef union FAPI_dlConfigPDUInfo_unT
{

    FAPI_dciDLPduInfo_st         DCIPdu;
    FAPI_bchConfigPDUInfo_st     BCHPdu;
    FAPI_mchConfigPDUInfo_st     MCHPdu;
    FAPI_dlSCHConfigPDUInfo_st   DlSCHPdu;
    FAPI_pchPduConfigInfo_st     PCHPdu;
    /* + PRS_CHANGES */
    FAPI_prsPduConfigInfo_st     PRSPdu;
    /* - PRS_CHANGES */
} FAPI_dlConfigPDUInfo_un;

/* ---------------------------------- */

/* DL Config PDU Structure */
typedef struct FAPI_dlConfigPDUInfo_stT
{
    /* 0: DCI DL PDU,
       1: BCH PDU, 
       2: MCH PDU,
       3: DLSCH PDU,
       4: PCH PDU,   
       5: PRS PDU */
    uint8_t     pduType; /* defined in FAPI_dlPDUTypeInfo_en */
    
    /*Size of the PDU control information (in bytes).
    This length value includes the 2 bytes required for the PDU type 
    and PDU size parameters. The size will always be a multiple of 4 bytes. */
    uint8_t     pduSize;

    uint8_t     padding[2];

    FAPI_dlConfigPDUInfo_un     dlConfigpduInfo;
}FAPI_dlConfigPDUInfo_st;

/* ---------------------------------- */

/* DL TLV Structure */
typedef struct FAPI_dlTLVInfo_stT
{
      /* Range 0 -> 1
      0: Payload is carried directly in the value field
      1: Pointer to payload is in the value field  */
#ifndef REAL_UE_ENVIORNMENT      
   /* CAUTION : Used as rnti; only in simulator enviornment - only for debugging purpose 
      Please take care while updating tag in TLV */
#endif      
      uint16_t      tag;

      /* Length of the payload in bytes */
      uint16_t      tagLen;
      
      /* Always a multiple of 32-bits.
      Tag=0: Only the most significant bytes of the size indicated by .length.
      field are valid. Remaining  bytes are zero padded to the nearest 32-bit 
      boundary.
      Tag=1: Pointer to the payload. Occupies 32-bits  */
      uint32_t      value[FAPI_VAR_SIZE(tagLen)];

}FAPI_dlTLVInfo_st;

/* ---------------------------------- */

/* DL PDU Info Structure */
typedef struct FAPI_dlPduInfo_stT
{
      /* The total length (in bytes) of the PDU description and PDU data. */
      uint16_t           pduLen;
      
      /* This is a count value which starts from 0. It is incremented for each 
      BCH, MCH, PCH or DLSCH PDU. This value was included in TX.request and 
      associates the data to the control information.It is reset to 0 for 
      every subframe.
      Range 0 . 65535 */
      uint16_t           pduIndex;

      /* The number of TLVs describing the data of the transport block. */
      uint32_t           numOfTLV;
      
      /* Always a multiple of 32-bits */
      FAPI_dlTLVInfo_st  dlTLVInfo[FAPI_VAR_SIZE(numOfTLV)];

}FAPI_dlPduInfo_st;

/* ---------------------------------- */

/* UL HI PDU structure */
typedef struct FAPI_dlHiPduInfo_stT
{
     /* 0: HI PDU */
     uint8_t     pduType;
     
     /* Size of the PDU control information (in bytes).
     This length value includes the 2 bytes required for the PDU type 
     and PDU size parameters.The size will always be a multiple of 4 bytes. */
     uint8_t     hipduSize; /* FIXME : Where did this come from - not there in doc */

     /* This value is the starting resource block assigned to the ULSCH grant 
     associated with this HI response. It should match the value sent in the 
     DCI format 0 which allocated the ULSCH grant
     Value: 0 . 100   */
     uint8_t     rbStart;   

     /* This value is the 2nd cyclic shift for DMRS assigned to the ULSCH grant
     associated with this HI response. It should match the value sent in the 
     DCI format 0 which allocated the ULSCH grant
     Value: 0 . 7  */
     uint8_t     cyclicShift2_forDMRS;

     /* The PHICH value which is sent on the resource.
     0: HI_NACK
     1: HI_ACK     */
     uint8_t     hiValue;

     /* Is used in the calculation of the PHICH location. For TDD only.
     1 = TDD subframe configuration 0 is used and the ULSCH grant associated 
         with this HI was received in subframe 4 or 9
     0 = in all other cases   */
     uint8_t     iPHICH;

     /* Offset to the reference signal power.
     Value: 0 . 10000, representing -6 dB to 4 dB in 0.001 dB steps. */
     uint16_t    txPower;
#ifndef OS_LINUX
     uint8_t  nPhichGrp;
     uint8_t  nPhichSeq;    
     uint8_t  padding[2];
#endif
}FAPI_dlHiPduInfo_st;

/* ---------------------------------- */

/* UL DCI PDU Structure */ 
typedef struct FAPI_dlDCIPduInfo_stT
{
     /* 1: HI PDU */
     uint8_t     pduType;
     
     /* Size of the PDU control information (in bytes).
     This length value includes the 2 bytes required for the PDU type 
     and PDU size parameters.The size will always be a multiple of 4 bytes. */
     uint8_t      ulDCIPDUSize; 

     /* Format of the DCI
     0 = 0
     1 = 3
     2 = 3A  */
     uint8_t     ulDCIFormat;

     /* CCE index used to send the DCI. 
     Value: 0 . 88   */
     uint8_t     cceIndex;

     /* The RNTI used for identifying the UE when receiving the PDU
     Valid for all DCI formats
     Value: 1 . 65535.    */
     uint16_t    rnti;

     /* The aggregation level used
     Value: 1,2,4,8   */
     uint8_t     aggLevel;

     /* The starting resource block for this ULSCH allocation.
     Valid for DCI format 0
     Value: 0 . 100     */
     uint8_t     rbStart;

     /* The number of resource blocks allocated to this ULSCH grant.
     Valid for DCI format 0
     Value: 0 . 100      */
     uint8_t     numOfRB;

     /* The modulation and redundancy version.
     Valid for DCI format 0
     Value: 0 . 31    */
     uint8_t     mcs;

     /* The 2nd cyclic shift for DMRS assigned to the UE in the ULSCH grant. 
     Valid for DCI format 0
     Value: 0 . 7    */
     uint8_t     cyclicShift2_forDMRS;

     /* Indicates if hopping is being used.
     Valid for DCI format 0
     0 = no hopping, 1= hopping enabled    */
     uint8_t     freqEnabledFlag;

     /* The frequency hopping bits
     Valid for DCI format 0
     Value: 0 . 3  */
     uint8_t     freqHoppingBits;

     /* The new data indicator for the transport block.
     Valid for DCI format 0    */
     uint8_t     newDataIndication;

     /* Indicates how the CRC is calculated on the PDCCH.
     Valid for DCI format 0
     0 = Not configured;
     1 = Configured and using UE port 0;
     2 = Configured and using UE port 1.   */
     uint8_t     ueTxAntennaSelection;

     /* Tx power control command for PUSCH.
     Valid for DCI format 0
     Value:  0,1,2,3 as represented in 36.213 Sec 5.1 for TPC Command Field.
     */
     uint8_t     tpc;

     /* Aperiodic CQI request flag
     Valid for DCI format 0
     0 = Aperiodic CQI not requested
     1 = Aperiodic CQI requested    */
     uint8_t     cqiRequest;

     /* UL index. Valid for TDD mode only.
     Valid for DCI format 0
     Value: 0,1,2,3    */
     uint8_t     ulIndex;

     /* DL assignment index. Valid for TDD mode only.
     Valid for DCI format 0
     Value: 1,2,3,4     */
     uint8_t     dlAssignmentIndex;

#ifndef FAPI_DEVIATIONS
     uint8_t     padding[1];
#else 
     /* CLPC_REWORK CHG */
     uint8_t     padding[3];
     /* Transmission Power for DCI 0/3/3A PDU */
     uint16_t    txPower;
#endif

     /* TPC commands for PUCCH and PUSCH
     Valid for DCI formats: 3,3A   */
     uint32_t    tpcBitMap;

     /* CLPC_REWORK_CHG END */

}FAPI_dlDCIPduInfo_st;

/* ---------------------------------- */

/* CQI Information */
typedef struct FAPI_cqiPduInfo_stT
{
     /* The PUCCH Index value  
     Value: 0 . 1185    */
     uint16_t   pucchIndex; 

     /* The size of the DL CQI/PMI in bits.
     Value: 0 . 255   */
     uint8_t    dlCqiPmiSize;

     uint8_t    padding[1];
}FAPI_cqiPduInfo_st;

/* ---------------------------------- */

/* SR Information */
typedef struct FAPI_srPduInfo_stT
{
     /* The PUCCH Index value  
     Value: 0 . 2047    */
     uint16_t	 pucchIndex;
	 #ifdef ASTRI_INTF
	 uint8_t	 padding[2]; 
	 #endif

}FAPI_srPduInfo_st;

/* SR Information Ends */
/* ---------------------------------- */

/* TDD Harq Information */
typedef struct FAPI_tddHarqPduInfo_stT
{
    /* For ACK_NACK Mode 0 (Bundling) and 1 (Multiplexing), 
    this is the size of the ACK/NACK in bits.
    Value: 1 . 4
    For Special Bundling this is the expected number of ACK/NACK 
    responses (UDAI + NSPS).
    Value: 0 . 9     */
    uint8_t   harqSize;

    /* The format of the ACK/NACK response expected. For TDD only.
    0 = BUNDLING
    1 = MULTIPLEXING        */
    uint8_t   acknackMode;

    /* The number of ACK/NACK responses received in this subframe. 
    For TDD only. 
    Value: 0 . 4
    (Value 0 is only valid for Special Bundling.)   */
    uint8_t   numOfPUCCHResource;

#ifdef ASTRI_INTF
    uint8_t   padding[1];
#endif
    /* SPR 1680 changes start*/
    /* HARQ resource 0, value: 0 . 2047 */
    uint16_t   n_PUCCH_1_0;

    /* HARQ resource 1, value: 0 . 2047 */
    uint16_t   n_PUCCH_1_1;

    /* HARQ resource 2, value: 0 . 2047 */
    uint16_t   n_PUCCH_1_2;

    /* HARQ resource 3, value: 0 . 2047 */
    uint16_t   n_PUCCH_1_3;
    /* SPR 1680 changes end*/

#ifndef ASTRI_INTF
    uint8_t   padding[1];
#endif
}FAPI_tddHarqPduInfo_st;

/* TDD Harq Information Ends */
/* ---------------------------------- */

/* FDD Harq Information */
typedef struct FAPI_fddHarqPduInfo_stT
{
    /* The PUCCH Index value for ACK/NACK
    Value: 0 . 2047    */
    uint16_t  pucchIndex;

    /* The PUCCH Index value for ACK/NACK
    Value: 0 . 2047    */
#ifndef FAPI_1_1_COMPLIANCE    
#ifndef ASTRI_INTF
    uint16_t  pucchIndex1;
#endif
#endif

    /*  The size of the ACK/NACK in bits.
    Value: 1 . 2   */
    uint8_t   harqSize;

#ifndef ASTRI_INTF
    uint8_t   padding[3];
#else
    uint8_t   padding[1];
#endif
}FAPI_fddHarqPduInfo_st;

/* FDD Harq Information Ends */
/* ---------------------------------- */

/* SRS PDU */
typedef struct FAPI_srsPduInfo_stT
{
    /* An opaque handling returned in the SRS.indication */
    uint32_t      handle;
    
    /* The size of the PDU in bytes. */
    uint16_t      size;
    
    /* The RNTI used for identifying the UE when receiving the PDU
    Value: 1 . 65535.   */
    uint16_t      rnti;

    /* SRS Bandwidth. This value is fixed for a UE and allocated in 
    RRC connection setup.
    Value: 0 . 3   */
    uint8_t       srsBandWidth;

    /* Frequency-domain position, NRRC This value is fixed for a UE and 
    allocated in RRC connection setup.
    Value: 0 . 23    */
    uint8_t       freqDomainPosition;

    /* Configures the frequency hopping on the SRS. This value is fixed 
    for a UE and allocated in RRC connection setup.
    Value 0 . 3    */
    uint8_t       srsHoppingBandWidth;

    /* Configures the frequency location of the SRS. 
    This value is fixed for a UE and allocated in RRC connection setup.
    Value: 0 .  1    */
    uint8_t       transmissionComb;

    /* Defines the periodicity and subframe location of the SRS.
    SRS Configuration Index. This value is fixed for a UE and 
    allocated in RRC connection setup.
    Value: 0 . 1023.   */
    uint16_t       isrsSRSConfigIndex;

    /* Configures the SRS sequence generation. This value is fixed for a UE 
    and allocated in RRC connection setup.
    Value: 0 . 7    */
    uint8_t       soundingRefCyclicShift;
#ifdef ASTRI_INTF
    uint8_t       userIndexDopplerEstimation;  
    uint8_t       newUserIndicator; 
    uint8_t       padding[3]; 
#else
    uint8_t       padding[1];
#endif
}FAPI_srsPduInfo_st;

/* SRS PDU Ends */
/* ---------------------------------- */

/* CQI_RI information */
typedef struct FAPI_cqiRiPduInfo_stT
{
    /* The size of the DL CQI/PMI in bits in case of rank 1 report.
    Value: 0 . 255   */
    uint8_t     dlCqiPmiSizeRank_1;

    /* The size of the DL CQI/PMI in bits in case of rank>1 report.
    Value: 0 . 255   */
    uint8_t     dlCqiPmiSizeRankGT_1;

    /* The size of RI in bits
    Value:1 . 2    */
    uint8_t     riSize;

    /* Delta offset for CQI. This value is fixed for a UE and allocated in 
    RRC connection setup.
    Value: 0 . 15    */
    uint8_t     deltaOffsetCQI;

    /* Delta offset for RI. This value is fixed for a UE and allocated in 
    RRC connection setup.
    Value: 0 . 15     */
    uint8_t     deltaOffsetRI;

    uint8_t     padding[3];
}FAPI_cqiRiPduInfo_st;

/* CQI_RI information Ends */
/* ---------------------------------- */

/* UCI_SR PDU */
typedef struct FAPI_uciSrPduInfo_stT
{
       /* An opaque handling returned in the RX_CQI.indication */
       uint32_t            handle;
       
       /* The RNTI used for identifying the UE when receiving the PDU
       Value: 1 . 65535.   */
       uint16_t            rnti;
#ifdef ASTRI_INTF
       uint8_t             nSrs;           //(ASTRI)
       uint8_t             padding[1];      //(ASTRI)
#endif

       FAPI_srPduInfo_st  srInfo;

}FAPI_uciSrPduInfo_st;

/* UCI_SR PDU Ends */
/* ---------------------------------- */

/* UCI_CQI PDU */
typedef struct FAPI_uciCqiPduInfo_stT
{
       /* An opaque handling returned in the RX_CQI.indication */
       uint32_t            handle;
       
       /* The RNTI used for identifying the UE when receiving the PDU
       Value: 1 . 65535.   */
       uint16_t            rnti;

       uint8_t             padding[2];

       FAPI_cqiPduInfo_st cqiInfo;

}FAPI_uciCqiPduInfo_st;

/* UCI_CQI PDU Ends */
/* ---------------------------------- */

/* UCI_HARQ PDU */
typedef struct FAPI_uciHarqPduInfo_stT
{
       /* An opaque handling returned in the RX_CQI.indication */
       uint32_t                 handle;
       
       /* The RNTI used for identifying the UE when receiving the PDU
       Value: 1 . 65535.   */
       uint16_t                 rnti;
	 /* Added according to SBL_FAPI*/ 
#ifdef ASTRI_INTF	   
       uint8_t                  nSrs;   //(ASTRI)
       uint8_t                  padding[1];
#else
       uint8_t                  padding[2];
#endif       
       /* depending upon Mode ( TDD or FDD ) */
#ifdef TDD_CONFIG
       FAPI_tddHarqPduInfo_st     harqInfo;

#elif defined FDD_CONFIG
       FAPI_fddHarqPduInfo_st     harqInfo;
#endif

}FAPI_uciHarqPduInfo_st;

/* UCI_HARQ PDU Ends */
/* ---------------------------------- */

/* UCI_SR_HARQ PDU */
typedef struct FAPI_uciSrHarqPduInfo_stT
{
       /* An opaque handling returned in the RX_CQI.indication */
       uint32_t                 handle;
       
       /* The RNTI used for identifying the UE when receiving the PDU
       Value: 1 . 65535.   */
       uint16_t                 rnti;
#ifdef ASTRI_INTF
       uint8_t                  nSrs;   //(ASTRI) 
       uint8_t                  padding[1];  
#endif

       FAPI_srPduInfo_st       srInfo;

       /* depending upon Mode ( TDD or FDD ) */
#ifdef TDD_CONFIG
       FAPI_tddHarqPduInfo_st     harqInfo;

#elif defined FDD_CONFIG
       FAPI_fddHarqPduInfo_st     harqInfo;
#endif

}FAPI_uciSrHarqPduInfo_st;

/* UCI_SR_HARQ PDU Ends */
/* ---------------------------------- */

/* UCI_CQI_HARQ PDU */

typedef struct FAPI_uciCqiHarqPduInfo_stT
{
       /* An opaque handling returned in the RX_CQI.indication */
       uint32_t                 handle;
#ifndef ASTRI_INTF
       /* The RNTI used for identifying the UE when receiving the PDU
       Value: 1 . 65535.   */
       uint16_t                 rnti;
       
       FAPI_cqiPduInfo_st       cqiInfo;
       uint8_t                  padding[2];
#else
       /* The RNTI used for identifying the UE when receiving the PDU
       Value: 1 . 65535.   */
       uint16_t                 rnti;

       uint8_t                  padding[2];

       FAPI_cqiPduInfo_st       cqiInfo;
#endif
       /* depending upon Mode ( TDD or FDD ) */
#ifdef TDD_CONFIG
       FAPI_tddHarqPduInfo_st     harqInfo;

#elif defined FDD_CONFIG
       FAPI_fddHarqPduInfo_st     harqInfo;
#endif

}FAPI_uciCqiHarqPduInfo_st;

/* UCI_CQI_HARQ PDU Ends */
/* ---------------------------------- */

/* UCI_CQI_SR PDU */
typedef struct FAPI_uciCqiSrPduInfo_stT
{
       /* An opaque handling returned in the RX_CQI.indication */
       uint32_t                 handle;
       
       /* The RNTI used for identifying the UE when receiving the PDU
       Value: 1 . 65535.   */
       uint16_t                 rnti;

	   #ifdef ASTRI_INTF
	   uint8_t                 nSrs;   //(ASTRI)  

	   uint8_t                 padding[1];
	   #endif

       FAPI_cqiPduInfo_st      cqiInfo;

       FAPI_srPduInfo_st       srInfo;

}FAPI_uciCqiSrPduInfo_st;

/* UCI_CQI_SR PDU Ends */
/* ---------------------------------- */

/* UCI_CQI_SR HARQ_PDU */
typedef struct FAPI_uciCqiSrHarqPduInfo_stT
{
       /* An opaque handling returned in the RX_CQI.indication */
       uint32_t                 handle;
       
       /* The RNTI used for identifying the UE when receiving the PDU
       Value: 1 . 65535.   */
       uint16_t                rnti;

       #ifdef ASTRI_INTF
       uint8_t                 nSrs;   //(ASTRI) 

       uint8_t				   padding[1];   
       #endif
	   
       FAPI_srPduInfo_st       srInfo;

       FAPI_cqiPduInfo_st      cqiInfo;
       
       /* depending upon Mode ( TDD or FDD ) */
#ifdef TDD_CONFIG
       FAPI_tddHarqPduInfo_st     harqInfo;

#elif defined FDD_CONFIG
       FAPI_fddHarqPduInfo_st     harqInfo;
#endif

}FAPI_uciCqiSrHarqPduInfo_st;

/* UCI_CQI_SR HARQ_PDU  Ends */
/* ---------------------------------- */

/* ULSCH PDU */
typedef struct FAPI_ulSCHPduInfo_stT 
{
     /* An opaque handling returned in the RX.indication */ 
     uint32_t      handle;
     
     /* The size of the ULSCH PDU in bytes as defined by the relevant UL grant.
     The size can be 0 if UCI over ULSCH without data is configured. 
     The size of CQI/RI/HARQ are not added to this element. */
     uint16_t       size;

     /* The RNTI used for identifying the UE when receiving the PDU
     Value: 1 . 65535. */
     uint16_t       rnti;

#ifndef OS_LINUX
     /* added by zj TODO */
     uint32_t    rntiFlag;
#endif 

     /* The starting resource block for this ULSCH allocation. 
     This should match the value sent in the DCI Format 0 PDU 
     which allocated this grant.
     Value: 0 . 99    */
     uint8_t       rbStart;

     /* The number of resource blocks allocated to this ULSCH grant. 
     This should match the value sent in the DCI Format 0 PDU 
     which allocated this grant.
     Value: 1 . 100    */
     uint8_t       numOfRB;

     /* 2: QPSK
     4: 16QAM
     6: 64QAM   */
     uint8_t       modulationType;

     /* The 2nd cyclic shift for DMRS assigned to the UE in the ULSCH grant. 
     This should match the value sent in the DCI Format 0 PDU 
     which allocated this grant.
     Value: 0 . 7 */
     uint8_t       cyclicShift2forDMRS;

     /* Indicates if hopping is being used. This should match the value 
     sent in the DCI Format 0 PDU which allocated this grant.
     0 = no hopping, 1= hopping enabled   */
     uint8_t       freqHoppingenabledFlag;

     /* The frequency hopping bits. This should match the value sent in 
     the DCI Format 0 PDU which allocated this grant.
     Value: 0 . 3      */
     uint8_t       freqHoppingBits;

     /* Specify whether this received PUSCH is a new transmission from UE. 
     This should match the value sent in the DCI Format 0 PDU which 
     allocated this grant. */
     uint8_t       newDataIndication;

     /* Redundancy version
     Value: 0 . 3  */
     uint8_t       redundancyVersion;

     /* HARQ Process number.
     TDD 0 . 15
     FDD 0 . 7   */
     uint8_t       harqProcessNumber;

     /* 0 = SISO/SIMO
     1 = MIMO     */
     uint8_t       ulTxMode;

     /* The current HARQ transmission count of this transport block. 
     Valid if frequency hopping enabled. */
     uint8_t       currentTxNB;

     /* Indicates if the resource blocks allocated for this grant 
     overlap with the SRS configuration.
     0 = no overlap
     1 = overlap     */
     uint8_t       nSRS;

#ifdef ASTRI_INTF
	 uint8_t       freqAvgWindowSize;		

     uint8_t       padding[3];
#endif

}FAPI_ulSCHPduInfo_st;

/* ULSCH PDU Ends */
/* ---------------------------------- */

/* Initial transmission parameters */
typedef struct FAPI_initialTxParam_stT
{
       /* 0 = last OFDM symbol is not punctured
       1 = last OFDM symbol is punctured. */
       uint8_t  nSRSInitial;

       /* The number of resource blocks used in the initial transmission 
       of this transport block.
       Value: 1 . 100    */
       uint8_t  initialNumOfRB;

       uint8_t  padding[2];
}FAPI_initialTxParam_st;

/* Initial transmission parameters Ends */
/* ---------------------------------- */



typedef struct FAPI_ulSCHHarqInfo_stT
{
    /*The size of the ACK/NACK in bits Value 1-2*/
    uint8_t         harqSize;
    /*Delta offset for HARQ. This value is fixed for a UE and allocated in
     *RRC connection setup. Value: 0-15*/
    uint8_t         deltaOffsetHarq;
    /*The format of the ACK/NACK response expected. For TDD only.
     * 0 = BUNDLING
     * 1 = MULTIPLEXING
     * */
	uint8_t         ackNackMode;
#ifdef ASTRI_INTF    
     uint8_t         ackNackBundlingScrambSeqIndex;
#else
     uint8_t         padding[1];
#endif
	
} FAPI_ulSCHHarqInfo_st;

/* ---------------------------------- */
/* ULSCH_CQI_HARQ_RI PDU */
typedef struct FAPI_ulSCHCqiHarqRIPduInfo_stT
{
       FAPI_ulSCHPduInfo_st      ulSCHPduInfo;

       FAPI_cqiRiPduInfo_st      cqiRiInfo;

       FAPI_ulSCHHarqInfo_st      harqInfo;

       FAPI_initialTxParam_st    initialTxParamInfo;

}FAPI_ulSCHCqiHarqRIPduInfo_st;

/* ULSCH_CQI_HARQ_RI PDU Ends */
/* ---------------------------------- */
    
/* ---------------------------------- */
/* ULSCH_HARQ PDU */
typedef struct FAPI_ulSCHHarqPduInfo_stT
{
       FAPI_ulSCHPduInfo_st       ulSCHPduInfo;
       
       FAPI_ulSCHHarqInfo_st      harqInfo;

       FAPI_initialTxParam_st     initialTxParamInfo;

}FAPI_ulSCHHarqPduInfo_st;

/* ULSCH_HARQ PDU  Ends */
/* ---------------------------------- */

/* ULSCH_CQI_RI PDU */
typedef struct FAPI_ulSCHCqiRiPduInfo_stT
{
      FAPI_ulSCHPduInfo_st           ulSCHPduInfo;

      FAPI_cqiRiPduInfo_st           cqiRiInfo;

      FAPI_initialTxParam_st         initialTxParamInfo;

}FAPI_ulSCHCqiRiPduInfo_st;

/* ULSCH_CQI_RI PDU Ends */
/* ---------------------------------- */

/* HARQ BUFFER PDU */
#ifdef FAPI_1_1_COMPLIANCE
typedef struct FAPI_ulSCHHarqBufferReleasePduInfo_stT
{
    uint32_t     handle;
    
    uint16_t     rnti;
    
    uint8_t      padding[2]; 

}FAPI_ulSCHHarqBufferReleasePduInfo_st;

/* HARQ BUFFER PDU Ends*/

/* ---------------------------------- */
#endif

/* UL CONFIG PDU Structure */
typedef struct FAPI_ulPDUConfigInfo_stT 
{
     /* 0: ULSCH,
     1 : ULSCH_CQI_RI,
     2 : ULSCH_HARQ,
     3 : ULSCH_CQI_HARQ_RI,
     4 : UCI_CQI,
     5 : UCI_SR, 
     6 : UCI_HARQ,
     7 : UCI_SR_HARQ,
     8 : UCI_CQI_HARQ,
     9 : UCI_CQI_SR,
     10: UCI_CQI_SR_HARQ,
     11: SRS */
     uint8_t  ulConfigPduType; /*Tags are defined in
                                 FAPI_ulConfigPduInfo_en */

     /* Size of the PDU control information (in bytes).
     This length value includes the 2 bytes required for the PDU type 
     and PDU size parameters.The length will always be a multiple of 4 bytes. */
     uint8_t  ulConfigPduSize;

     uint8_t  padding[2];

     /* need to typecase it with appropriate 
     UL CONFIG PDU TYPE (ulConfigPduType)*/
     uint8_t  ulPduConfigInfo[FAPI_VAR_SIZE(x)];

}FAPI_ulPDUConfigInfo_st;

/* ---------------------------------- */

/* UL DATA PDU Structure */
typedef struct FAPI_ulDataPduIndication_stT
{
    /* The handle received in the ULSCH PDU. */
    uint32_t     handle;

    /* The RNTI passed to the PHY in a DL_CONFIG.request ULSCH PDU 
    Value: 1 . 65535. */
    uint16_t     rnti;

    /* Length of PDU in bytes. */
    uint16_t     length;
    
    /* Gives the PDU#i data address offset from the beginning of the 
    'Number of PDUs' field.An offset of 0 indicates a CRC or decoding error. */
    uint16_t     dataOffset;

    /* The timing advance measured for this PDU and UE.
    Value: T_A from 0 to 1282   */
    uint16_t     timingAdvance;
#ifdef ASTRI_INTF
	uint16_t     timingAdvance_ts;
#endif
	
    /* SNR 
    Value: 0-255, representing -64dB to 63.5dB, with 0.5dB step size. */
    uint8_t      ulCqi;
#ifdef ASTRI_INTF
    uint8_t    padding[1];
#else
    uint8_t    padding[3];
#endif

}FAPI_ulDataPduIndication_st;

/* ---------------------------------- */

/* FDD HARQ PDU Info Structure */
typedef struct FAPI_fddHarqPduIndication_stT
{
#ifdef FAPI_1_1_COMPLIANCE    
    uint32_t handle;
#endif    
    /* The RNTI passed to the PHY in an uplink subframe configuration PDU. 
    Value: 1 . 65535.  */
#ifdef ASTRI_INTF
    uint32_t    handle;
#endif
    uint16_t   rnti;

    /* HARQ feedback of 1st TB.
    Range 1 . 7
    1 = ACK
    2 = NACK
    3 = ACK or NACK
    4 = DTX
    5 = ACK or DTX
    6 = NACK or DTX
    7 = ACK or NACK or DTX   */
    uint8_t    harqTB1;
    
    /* HARQ feedback of 2nd TB. 
    Range 1 . 7
    1 = ACK
    2 = NACK
    3 = ACK or NACK
    4 = DTX
    5 = ACK or DTX
    6 = NACK or DTX
    7 = ACK or NACK or DTX   */
    uint8_t    harqTB2;
#ifdef ASTRI_INTF
	uint8_t     pucchRssi; //(ASTRI)   

    uint8_t     padding[3]; //(ASTRI)  
#endif

}FAPI_fddHarqPduIndication_st;

/* ---------------------------------- */

/* TDD Bundling type harq info structure */ 
typedef struct FAPI_tddBundlingHarqInfoT
{
    /* Indicates HARQ results. 
    Range 1 . 7
    1 = ACK
    2 = NACK
    3 = ACK or NACK
    4 = DTX
    5 = ACK or DTX
    6 = NACK or DTX
    7 = ACK or NACK or DTX    */
    uint8_t  value0;

    /* Indicates HARQ results. Range 1 . 7 same as above */
    uint8_t  value1;

    uint8_t  padding[2];

}FAPI_tddBundlingHarqInfo_st;

/* ---------------------------------- */

/* TDD multiplexing type harq info structure */ 
typedef struct FAPI_tddMultiplexingHarqInfo_stT
{
    /* Indicates HARQ results. 
    Range 1 . 7
    1 = ACK
    2 = NACK
    3 = ACK or NACK
    4 = DTX
    5 = ACK or DTX
    6 = NACK or DTX
    7 = ACK or NACK or DTX    */
    uint8_t  value0;

    /* Indicates HARQ results. Range 1 . 7 same as above */
    uint8_t  value1;

    /* Indicates HARQ results. Range 1 . 7 same as above */
    uint8_t  value2;

    /* Indicates HARQ results. Range 1 . 7 same as above */
    uint8_t  value3;

}FAPI_tddMultiplexingHarqInfo_st;

/* ---------------------------------- */

/* TDD Special type harq info structure */ 
typedef struct FAPI_tddSpecialBundlingHarqInfo_stT
{
    /* Number of ACK among multiple ACK/NACK responses 
    0 = 0 or None (UE detect at least one DL assignment is missed)
    1 = 1 or 4 or 7 ACKs reported
    2 = 2 or 5 or 8 ACKs reported
    3 = 3 or 6 or 9 ACKs reported
    4 = DTX (UE did not transmit anything)  */
    uint8_t  value0;

    uint8_t  padding[3];
}FAPI_tddSpecialBundlingHarqInfo_st;

/* ---------------------------------- */

/* TDD HARQ PDU Info Structure */
typedef struct FAPI_tddHarqPduIndication_stT
{
    /* The handle received in the ULSCH PDU or UCI PDU. */
    uint32_t    handle;

    /* The RNTI passed to the PHY in an uplink subframe configuration PDU. 
    Value: 1 . 65535.  */
    uint16_t    rnti;

    /* The format of the ACK/NACK response expected. The bundling and 
    multiplexing options are passed to the PHY in an uplink subframe 
    configuration PDU. If the ACK/NACK is combined with either CQI or SR 
    information then a special ACK/NACK encoding is used which reports 
    the number of ACKs, rather than providing specific ACK/NACK values. 
    This is identified separately and called SPECIAL_BUNDLING in this API. 
    0 = BUNDLING
    1 = MULTIPLEXING   */
    uint8_t     mode;

    /* The number of ACK/NACK results reported for this UE.
    Value: 1 . 4 
    For Special Bundling this is the expected number of ACK/NACK 
    responses (UDAI + NSPS).
    Value: 0 . 9  */
    uint8_t     numOfAckNack;

    /* The format of the data is dependent on the HARQ mode; 
    BUNDLING, MULTIPLEXING, or SPECIAL BUNDLING.  
    need to typecast it with appropriate TTD Harq structures
    defined above */
#ifdef ASTRI_INTF  
    uint8_t     pucchRssi; //(ASTRI) 
    uint8_t     padding[3]; //(ASTRI)
#endif
    /* SPR 8543 Fix Start */
    #define MAX_HARQ_BUFFER_SIZE 4
    uint8_t     harqBuffer[MAX_HARQ_BUFFER_SIZE];
    /* SPR 8543 Fix End */

}FAPI_tddHarqPduIndication_st;

/* ---------------------------------- */

/* CRC PDU Structure */
typedef struct FAPI_crcPduIndication_stT
{
    /* The handle received in the ULSCH PDU */
    uint32_t    handle;

    /* The RNTI passed to the PHY in an uplink subframe configuration PDU. 
    Value: 1 . 65535.  */
    uint16_t    rnti;

    /* A flag indicating if a CRC error was detected.
    0: CRC_CORRECT
    1:CRC_ERROR   */
    uint8_t     crcFlag;
#ifdef ASTRI_INTF  
    uint8_t     rssi;           //ASTRI
    uint8_t     effective_snr;  //ASTRI
    uint8_t     padding[3];
#else
    uint8_t     padding[1];
#endif
}FAPI_crcPduIndication_st;

/* ---------------------------------- */
#if 1
    /* CRC Received Interference Power */
typedef struct FAPI_crc_Resource_Block_stT
{
    uint8_t     Received_Interference_Power;
    uint8_t     rssi;
    uint8_t     padding[2];

}FAPI_crc_Resource_Block_st;
#endif
/* SR PDU Structure */
typedef struct FAPI_srPduIndication_stT
{
    /* The handle received in the ULSCH PDU */
    uint32_t    handle;

    /* The RNTI passed to the PHY in an uplink subframe configuration PDU. 
    Value: 1 . 65535.  */
    uint16_t    rnti;
#ifdef ASTRI_INTF
    uint8_t     pucch_rssi;   //ASTRI
    uint8_t		pucch_snr;
#else
    uint8_t     padding[2];
#endif
}FAPI_srPduIndication_st;

/* ---------------------------------- */

/* CQI Indication structure */
typedef struct FAPI_cqiPduIndication_stT
{
    /* The handle received in the ULSCH PDU or UCI PDU.*/
    uint32_t    handle;

    /* The RNTI identifying the UE. For semi-static information held in the MAC
     this will be the value passed to the PHY in a UL_CONFIG.request CQI PDU.  
    Value: 1 . 65535.  */
    uint16_t    rnti;

    /* Length of PDU in bytes. */
    uint16_t    length;
   
    /* Gives the PDU#i data address offset from the beginning of the 
    'Number of PDUs' field. An offset of 0 indicates a CRC or decoding error, 
    or only RI received on PUSCH. */
    uint16_t    dataOffset;

    /* The timing advance measured for this PDU and UE.
    Value: T_A from 0 to 1282   */
    uint16_t  timingAdvance;
#ifdef ASTRI_INTF
	uint16_t  timingAdvance_ts;
#endif

    /* SNR 
    Value: 0-255, representing -64dB to 63.5dB, 
    with 0.5dB step size.   */
    uint8_t    ulCqi;

    /* The rank indication reported by the UE on PUSCH
    Value: 0..4
    0 = RI not received
    1..4 = RI value    */
    uint8_t   ri;
#ifndef ASTRI_INTF
    uint8_t   padding[2];
#endif

}FAPI_cqiPduIndication_st;

/* ---------------------------------- */

/* RACH PDU Structure */
typedef struct FAPI_rachPduIndication_stT
{
    /* The RA-RNTI value. 
    Value: 1 . 65535.  */
    uint16_t    rnti;
 
    /* The measured timing advance for the preamble. 
    Value: 0 . 1282   */
    uint16_t    timingAdvance;
#ifdef ASTRI_INTF    
    uint16_t    timingAdvance_ts;
#endif
    /* The detected preamble
    Value: 0 . 63  */
    uint8_t     preamble;

#ifdef ASTRI_INTF
    uint8_t     rssi;
#else //Elaine: bug fix on the rach ind definition
    uint8_t     padding[3];
#endif
}FAPI_rachPduIndication_st;

/* ---------------------------------- */

/* SRS PDU Structure */
typedef struct FAPI_srsPduIndication_stT
{
    /* The handle received in the SRS PDU. */
    uint32_t    handle;
    
    /* The RNTI passed to the PHY in an uplink subframe configuration PDU */
    uint16_t    rnti;

    /* FFS.  Values: 0 . 255, */
    uint16_t    dopplerEstimation; 

   /* The timing advance measured for the UE.
   Value: T_A from 0 to 1282 */
    uint16_t    timingAdvance;
#ifdef ASTRI_INTF    
    uint16_t    timingAdvance_ts;
#endif
    /* Number of resource blocks to be reported for this UE */
    uint8_t     numOfRB;

    /* The starting point of the RBs to be reported.  */
    uint8_t     rbStart;

    /* Field size dependent on configured bandwidth
    SNR for RBs, each RBs report one SNR. 
    Value: 0-255, representing -64dB to 63.5dB, with 0.5dB step size..*/
#ifdef ASTRI_INTF  
    uint8_t     padding[2];
#endif

    uint8_t     snr[FAPI_VAR_SIZE(numOfRB)]; 

}FAPI_srsPduIndication_st;

/* ---------------------------------- */

/* For error codes FAPI_SFN_OUT_OF_SYNC and FAPI_MSG_INVALID_SFN */
typedef struct FAPI_errMsgBody1_stT
{
    /* The SFN/SF value received in the message */
    uint16_t  recvdSfnSf;

    /* The SFN/SF value the PHY was expecting to receive in the message */
    uint16_t  expectedSfnSf;

}FAPI_errMsgBody1_st;

/* if error codes is FAPI_MSG_PDU_ERR */
typedef struct FAPI_errMsgBody2_stT
{
#ifdef ASTRI_INTF  
    uint16_t  receivedSfnSf;
#endif
    uint8_t    subErrCode;

    /* Indicates if this error was in a DL subframe configuration or an 
    UL subframe configuration.
    0 = DL, 1 = UL */
    uint8_t    direction;  

    /* The RNTI in the received PDU. If the error occurred in a MCH, or BCH, 
    PDU this value is set to 0  */
    uint16_t   rnti;

    /* The PDU Type  parameter specified in both DL subframe configuration and 
    UL subframe configuration   */
    uint8_t   pduType;

    uint8_t   padding[1];
}FAPI_errMsgBody2_st;

/* if error codes is FAPI_MSG_HI_ERR */
typedef struct FAPI_errMsgBody3_stT
{
#ifdef ASTRI_INTF  
      uint16_t  receivedSfnSf;
      uint16_t  rnti;
#endif
      uint8_t   subErrCode;

      /* The PHICH RB Index parameters specified in each HI PDU */
      uint8_t   phichLowestulRbIndex;

#ifdef ASTRI_INTF  
      uint8_t   padding[2];
#endif
}FAPI_errMsgBody3_st;

/* if error codes is FAPI_MSG_TX_ERR */
typedef struct FAPI_errMsgBody4_stT
{
#ifdef ASTRI_INTF  
      uint16_t  receivedSfnSf;
#endif
       uint8_t   subErrCode;

      /* The PDU index parameter specified for each PDU */
      uint8_t   pduIndex;

}FAPI_errMsgBody4_st;


/* Union of all message body types */
typedef union FAPI_errMsgBody_unT {
    FAPI_errMsgBody1_st msgBody1;
    
    FAPI_errMsgBody2_st msgBody2;
    
    FAPI_errMsgBody3_st msgBody3;
    
    FAPI_errMsgBody4_st msgBody4;
} FAPI_errMsgBody_un;



/* Message definition starts from here */

/*  ------------------------------- */
/* 
 * Every message changed betweem MAC/L3 and PHY shall follow this format
 */
typedef struct FAPI_l1ApiMsg_stT
{
     /* Message type ID */
     FAPI_msgIdType_t     msgId;
    
     /* Length of vendor-specific message body (bytes) */
     uint8_t              lenVendorSpecific; 
   
     /* Length of message body (bytes) */
     uint16_t             msgLen;

     /* Message body.
      * This shall contain one of the below mentioned structure.
      * Each of the below mentioned structure are specific to
      * different message type */
     uint8_t              msgBody[FAPI_VAR_SIZE(msgLen)];

     /* Vendor-specific message body */
     uint8_t              vendorMsgBody[FAPI_VAR_SIZE(lenVendorSpecific)];
      
}FAPI_l1ApiMsg_st;

/*  ------------------------------- */

/* + TM7_8 Changes Start */
#define MAX_NUM_ANTENNA 4

/* + E_CID_5.3 */
typedef struct FAPI_REL9TARecievedForUE_stT
{
    /* The RNTI passed to the PHY */
    uint8_t    preamble;
    
    uint8_t   padding[1];
   /* The timing advance  measured for the postioning of UE. 
    Value:  0 -  2047 (measured in steps of 2Ts)
            2048 - 7690 (measured in steps of 8Ts) */
    uint16_t     timingAdvanceR9;

}FAPI_REL9TARecievedForUE_st;

typedef struct FAPI_UEMeasurementInRACH_stT
{
    /* Number of UEs Whose reports are reported*/
    uint16_t                    numOfpreambles;

    uint8_t                     padding[2];
   /*AOA and Rel9 TA information for the UEs */
    FAPI_REL9TARecievedForUE_st    rel9TAInfo[FAPI_VAR_SIZE(numOfpreambles)];
}FAPI_UEMeasurementInRACH_st;

typedef struct FAPI_VendorSpecificForRACH_stT
{
    /*UE specific measurements information Structure*/
    FAPI_UEMeasurementInRACH_st  ueMeasInfoInRACHForUEs;

}FAPI_VendorSpecificForRACHInd_st;

/* - E_CID_5.3 */

/* + TM7_8 Changes End */
typedef struct FAPI_AOA_REL9TARecievedForUE_stT
{
    /* The RNTI passed to the PHY */
    uint16_t    rnti;
    
   /* + E_CID_5.3 */ 
   /* The timing advance  measured for the postioning of UE. 
    Value:  0 -  2047 (measured in steps of 2Ts)
            2048 - 7690 (measured in steps of 8Ts) */
    uint16_t     timingAdvanceR9;

    /* AoA info for 4 Antennas */
    uint16_t     AoA[MAX_NUM_ANTENNA];
   /* - E_CID_5.3 */ 
}FAPI_AOA_REL9TARecievedForUE_st;

typedef struct FAPI_UEMeasurementIndication_stT
{
    /* Number of UEs Whose reports are reported*/
    uint16_t                    numOfUEs;
    uint8_t                     padding[2];
   /*AOA and Rel9 TA information for the UEs */
    FAPI_AOA_REL9TARecievedForUE_st    aoaRel9TAInfo[FAPI_VAR_SIZE(numOfUEs)];
}FAPI_UEMeasurementIndication_st;

typedef struct FAPI_VendorSpecific_stT
{
    /*UE specific measurements information Structure*/
    FAPI_UEMeasurementIndication_st  ueMeasInfoForUEs;

}FAPI_VendorSpecific_st;

/* - TM7_8 Changes End */
/* in Param request we need to fill only msgId of struct FAPI_L1Api_Msg_st */
/* The message length in the generic header = 0. */ 

/* PHY_PARAM_REQUEST: PARAM Request*/

/*  ------------------------------- */
/* PHY PARAM RESPONSE: This defines the message body for PHY_PARAM_RESPONSE
 * API */

typedef struct FAPI_paramResponse_stT
{
#ifdef ASTRI_INTF
uint32_t errCode;
#else
     FAPI_l1ErrorCodes_en        errCode; /* possble error codes are
                                             defined in FAPI_l1ErrorCodes_en*/
#endif
     uint8_t                     numOfTlv;
/*+ SPR 9285*/
#ifdef ASTRI_INTF
     uint8_t                     padding[3];
#else
     uint8_t                     padding[2];
#endif
/*- SPR 9285*/
     FAPI_paramResponseTLV_st   tlvs[FAPI_VAR_SIZE(numOfTlv)];
     
}FAPI_paramResponse_st;

/*  ------------------------------- */


/* PHY CELL CONFIG REQUEST:This defines the message body for
 * PHY_CELL_CONFIG_REQUEST */

typedef struct FAPI_phyCellConfigRequest_stT
{
     uint8_t               numOfTlv;

     uint8_t               padding[3];
     /* Please refer FAPI document to know mandatory TLVs for IDLE,
      * CONFIGURED, RUNNING state for FDD/TDD */
     FAPI_cellConfig_st        configtlvs[FAPI_VAR_SIZE(numOfTlv)];

}FAPI_phyCellConfigRequest_st;

/* PHY_RECONFIG_CELL_REQ : This defines the message body for 
*  PHY_RECONFIG_CELL_REQUEST 
*/
typedef struct FAPI_phyCellReConfigRequest_stT
{
    uint8_t                 numOfTlv;
    uint8_t                 padding[3];
    /* Please refer FAPI document to know reconfig TLVs for 
     * RECONFIG during running state of PHY 
     */
    FAPI_cellReConfig_st    configtlvs[FAPI_VAR_SIZE(numOfTlv)];
}FAPI_phyCellReConfigRequest_st;


/*  ------------------------------- */
/* PHY CELL CONFIG RESPONSE:This defines the message body for
 * PHY_CELL_CONFIG_RESPONSE */
typedef struct FAPI_phyCellConfigResp_stT
{
  /* Possible values are defined in FAPI_l1ErrorCodes_en. */
    FAPI_l1ErrorCodes_en    errorCode; 

    /* Number of invalid or unsupported TLVs contained in the message body. */
    uint8_t                 numOfInvalidOrUnsupportedTLV;

    /* Number of missing TLVs contained in the message body. 
    If the PHY is in the CONFIGURED state this will always be 0. */
    uint8_t                 numOfMissingTLV;

/*+ SPR 9285*/
    uint8_t                 padding;
/*- SPR 9285*/
    /* A list of invalid or unsupported TLVs . 
    each TLV is presented in its entirety. */
    FAPI_cellConfig_st          listOfTLV[FAPI_VAR_SIZE(numOfInvalidOrUnsupportedTLV)];
    
    /* A list of missing TLVs . each TLV is presented in its entirety */
    FAPI_cellConfig_st          listOfMissingTlv[FAPI_VAR_SIZE(numOfTlv)];

}FAPI_phyCellConfigResp_st;

/*  ------------------------------- */
/* START REQUEST:This defines the message body for
 * PHY_START_REQUEST */

/* No message body is defined for START.request.
 * The message length in the generic header = 0 */

/*  ------------------------------- */

/* STOP REQUEST:This defines the message body for
 * PHY_STOP_REQUEST */

/* No message body is defined for STOP.request.
 * The message length in the generic header = 0 */

/*  ------------------------------- */

/* STOP INDICATION:This defines the message body for
 * PHY_STOP_INDICATION */

/* No message body is defined for STOP.indication.
 * The message length in the generic header = 0 */

/*  ------------------------------- */

/* PHY UE CONFIG REQUEST: This defines the message body for
 * PHY_UE_CONFIG_REQUEST.
 * The message is only valid if semi-static configuration is kept in PHY.
 */

typedef struct FAPI_phyUeConfigRequest_stT
{
     uint8_t                numOfTlv;
     uint16_t                ueIndex;
     uint16_t				cfgMode; /* 1:Creat Ue Cfg; 2:Updata Ue Cfg;3:Release Ue */
     FAPI_ueConfig_st       configtlvs[FAPI_VAR_SIZE(numOfTlv)];

}FAPI_phyUeConfigRequest_st;

/*  ------------------------------- */

/* PHY UE CONFIG RESPONSE: This defines the message body for
 * PHY_UE_CONFIG_RESPONSE
 * The message is only valid if semi-static configuration is kept in PHY.
 */

typedef struct FAPI_phyUeConfigResp_stT
{
  /* Possible values are defined in FAPI_l1ErrorCodes_en. */
    FAPI_l1ErrorCodes_en    errorCode; 

    /* Number of invalid or unsupported TLVs contained in the message body. */
    uint8_t                 numOfInvalidOrUnsupportedTLV;

    /* Number of missing TLVs contained in the message body. 
    If the PHY is in the CONFIGURED state this will always be 0. */
    uint8_t                 numOfMissingTLV;

/*+ SPR 9285*/
    uint8_t                 padding;
/*- SPR 9285*/
    /* A list of invalid or unsupported TLVs . 
    each TLV is presented in its entirety. */
    FAPI_ueConfig_st        listOfTLV[FAPI_VAR_SIZE(numOfInvalidOrUnsupportedTLV)];
    
    /* A list of missing TLVs . each TLV is presented in its entirety */
    FAPI_ueConfig_st        listOfMissingTlv[FAPI_VAR_SIZE(numOfTlv)];

}FAPI_phyUeConfigResp_st;

/*  ------------------------------- */

/* PHY Notification : This defines the message body for
 * PHY_ERROR_INDICATION */
#if 0 //Elaine 
typedef struct FAPI_phyErrorIndication_stT
{
    /* Indicate which message received by the PHY has an error */
    FAPI_msgIdType_t        msgId;

/*+ SPR 9285*/
    uint8_t                 padding[2];
/*- SPR 9285*/

    FAPI_l1ErrorCodes_en    errorCode;

    FAPI_errMsgBody_un      errMsgBody;

}FAPI_phyErrorIndication_st;
#else
typedef struct FAPI_phyErrorIndication_stT
{
    /* Indicate which message received by the PHY has an error */
    FAPI_msgIdType_t        msgId;

/*+ SPR 9285*/
    uint8_t                 padding[3]; //Elaine
/*- SPR 9285*/

    uint32_t        errorCode;

    FAPI_errMsgBody_un      errMsgBody;

}FAPI_phyErrorIndication_st;
#endif

/* if error codes is FAPI_MSG_INVALID_STATE or FAPI_MSG_BCH_MISSING */
/* then errValues are not supported in notification */
/*  ------------------------------- */

/* SUBFRAME indication : This defines the message body for
 * PHY_UL_SUBFRAME_INDICATION */
typedef struct FAPI_subFrameIndication_stT
{
    /* A 16-bit value where,[15:4] SFN, range 0 . 1023[3:0] SF, range 0 . 9 */
    uint16_t   sfnsf;
    #ifdef ASTRI_INTF 
	uint8_t   padding[2];
    #endif
}FAPI_subFrameIndication_st;

/*  ------------------------------- */

/*****************************************************************************
   Downlink Subframe for following information
    - DL_CONFIG_Request
    - HI_DCI0_CONFIG
    - DL Data : TX_Request
 *****************************************************************************/

/* DL_CONFIG_Request MSG : This defines the message body for
 * PHY_DL_CONFIG_REQUEST */

typedef struct FAPI_dlConfigRequest_stT
{
    /* A 16-bit value where,[15:4] SFN, range 0 . 1023[3:0] SF, range 0 . 9 */ 
    uint16_t                    sfnsf;

    /* The length of the downlink subframe configuration. 
    Range 0 . 65535. */
    uint16_t                    length;

    /* The number of OFDM symbols for the PDCCH.
    Value:0 . 4 */
    uint8_t                     cfi;

    /*The number of DCI PDUs included in this message.
    Range: 0 . 255 */
    uint8_t                     numDCI;

    /* Number of PDUs that are included in this message.
    Range 0 . 514 */
    uint16_t                    numOfPDU;

    /* Offset to the reference signal power.
    Value: 0 . 10000, representing -6 dB to 4 dB in 0.001 dB steps. */
    uint16_t                    txPowerForPCFICH;

    /* Number of unique RNTIs sent on the PDSCH. 
    - a PCH PDU will have an unique RNTI and should be included in this value
    - a DLSCH PDU can be one transport block sent to a UE with an unique RNTI.
      This RNTI should be included in this value
    - a DLSCH PDU can be one of two transport blocks sent to a UE. In this case
      the two DLSCH PDUs will share the same RNTI. Only one RNTI should be 
      included in this value. */
    uint8_t                     numOfPDSCHRNTI;

    uint8_t                     padding[1];
    /* possible values of PDU Type supported are defined 
     * as macro preprocessor */

    FAPI_dlConfigPDUInfo_st       dlConfigpduInfo[FAPI_VAR_SIZE(numOfPDU)];

}FAPI_dlConfigRequest_st;

/* + TM7_EPRE */
typedef struct FAPI_PdschEpreToUeRsRatio_stT
{
    uint16_t        rnti;
    /*pdschEpreToUeRsRatio;*/
    int8_t          pdschEpreToUeRsRatio;
    uint8_t         padding[1];

}FAPI_PdschEpreToUeRsRatio_st;


typedef struct FAPI_VendorSpecificForDLConfig_stT
{
    /* Number of UEs Whose reports are reported*/
    uint16_t                    numOfUEs;
    uint8_t                     padding[2];
    
    /* Structure of pdschEpreToUeRsRatio;*/
    FAPI_PdschEpreToUeRsRatio_st  pdschEpreToUeRsRatioInfo[FAPI_VAR_SIZE(numOfUEs)];

}FAPI_VendorSpecificForDLConfig_st;
/* - TM7_EPRE */


/*  ------------------------------- */

/* HI DCI0 REQUEST : This defines the message body for
 * PHY_DL_HI_DCI0_REQUEST . */

typedef struct FAPI_dlHiDCIPduInfo_stT
{
    /* The SFN/SF in this message should be the same as the corresponding 
    DL_CONFIG.request message. A 2-byte value where,
    [15:4] SFN, range 0 . 1023
    [3:0] SF, range 0 . 9    */
    uint16_t                    sfnsf;

    /* Number of DCI PDUs included in this message */
    uint8_t                     numOfDCI;

    /* Number of HI PDUs included in this message */
    uint8_t                     numOfHI;

    uint8_t                     hidciPduInfo[FAPI_VAR_SIZE(numOfHI)];

}FAPI_dlHiDCIPduInfo_st;

/*  ------------------------------- */

/* DL DATA TX REQUEST : This defines the message body for 
 * PHY_DL_TX_REQUEST .  */

typedef struct FAPI_dlDataTxRequest_stT
{
    /* The SFN/SF in this message should be the same as the corresponding 
    DL_CONFIG.request message.A 2-byte value where,[15:4] SFN, range 0 . 1023
    [3:0] SF, range 0 . 9   */
    uint16_t        sfnsf;

    /* Number of PDUs included in this message. */
    uint16_t        numOfPDU;

    /* For each PDU */
    FAPI_dlPduInfo_st  dlPduInfo[FAPI_VAR_SIZE(numOfPDU)];

}FAPI_dlDataTxRequest_st;

/*  ------------------------------- */

/*****************************************************************************
   Uplink Subframe Information
    - UL_CONFIG_Request
    - UL Data
 *****************************************************************************/
/* UL CONFIG REQUEST : This defines the message body for
 * PHY_UL_CONFIG_REQUEST.
 * 
 * The supported PDUs are dependent on whether the semi-static information
 * and uplink HARQ signalling calculation is held in the MAC or PHY.
 * 
 */

typedef struct FAPI_ulConfigRequest_stT
{
    /* A 16-bit value where,[15:4] SFN, range 0 . 1023[3:0] SF, range 0 . 9 */
    uint16_t                     sfnsf;

    /* The length of the uplink subframe configuration. 
    Range 0 . 65535. */
    uint16_t                        ulConfigLen;

    /* The number of UL SCHs PDUs included in this message. */
    uint8_t                         numOfPdu;

    /* If semi-static information is held in the MAC
    For FDD:
    0: No RACH in this subframe
    1: RACH present in this subframe
    For TDD:
    Bits 0:5 indicate which RACH frequency resources are used in this 
    subframe,
    0: This RACH frequency index is not used
    1: This RACH frequency index is used 
    If semi-static information is held in the PHY this parameter is ignored. */
    uint8_t                         rachFreqResources;

    /* If semi-static information is held in the MAC
    0: No SRS in this subframe
    1: SRS present in this subframe
    If semi-static information is held in the PHY this parameter is ignored. */
    uint8_t                         srsPresent;

    uint8_t                         padding[1];
   /* For Number of PDUs */
    FAPI_ulPDUConfigInfo_st  ulPduConfigInfo[FAPI_VAR_SIZE(numOfPdu)];

}FAPI_ulConfigRequest_st;

/*  ------------------------------- */

/* RX ULSCH DATA INDICATION  : This defines the message body for 
 * PHY_UL_RX_ULSCH_INDICATION . */

typedef struct FAPI_rxULSCHIndication_stT
{
     /* The SFN/SF of the SUBFRAME this information was received in.
     A 16-bit value where,[15:4] SFN, range 0 . 1023[3:0] SF, range 0 . 9 */
     uint16_t               sfnsf;

     /* Number of PDUs included in this message. */
     uint16_t               numOfPdu;

     /* For (Number of PDUs) { */
     FAPI_ulDataPduIndication_st  ulDataPduInfo[FAPI_VAR_SIZE(numOfPdu)];
     /* } */

     /* Contents of PDU#1...PDU#n These will be MAC PDUs. */
     uint8_t                pduBuffer[FAPI_VAR_SIZE(x)];

}FAPI_rxULSCHIndication_st;

/*  ------------------------------- */

/* UL HARQ INDICATION  : This defines the message body for   
 * PHY_UL_HARQ_INDICATION . */

typedef struct FAPI_harqIndication_stT
{
    /* The SFN/SF of the SUBFRAME this information was received in.
    A 16-bit  value where,[15:4] SFN, range 0 . 1023 [3:0] SF, range 0 . 9 */ 
    uint16_t             sfnsf;

    /* Number of HARQs included in this message. */
    uint16_t             numOfHarq;

    /* For (Number of HARQs) depending upon Mode ( TDD or FDD ) */
#ifdef TDD_CONFIG
    FAPI_tddHarqPduIndication_st harqPduInfo[FAPI_VAR_SIZE(numOfHarq)];

#elif defined FDD_CONFIG
    FAPI_fddHarqPduIndication_st harqPduInfo[FAPI_VAR_SIZE(numOfHarq)];
#endif

}FAPI_harqIndication_st;

/*  ------------------------------- */

/* UL CRC INDICATION  : This defines the message body for  
 * PHY_UL_CRC_INDICATION . */

typedef struct FAPI_crcIndication_stT
{
    /* The SFN/SF of the SUBFRAME this information was received in.
    A 16-bit  value where,[15:4] SFN, range 0 . 1023 [3:0] SF, range 0 . 9 */ 
    uint16_t            sfnsf;

    /* Number of CRCs included in this message. */
    uint16_t            numOfCrc;

#ifdef ASTRI_INTF 
	uint8_t             numberOfResourceBlocks; 
	uint8_t             padding[3];
#endif

    /* For (Number of CRCs) */
    FAPI_crcPduIndication_st  crcPduInfo[FAPI_VAR_SIZE(numOfCrc)];

}FAPI_crcIndication_st;

/*  ------------------------------- */

/* UL RX SR INDICATION  : This defines the message body for  
 * PHY_UL_RX_SR_INDICATION . */

typedef struct FAPI_rxSRIndication_stT
{
    /* The SFN/SF of the SUBFRAME this information was received in.
    A 16-bit  value where,[15:4] SFN, range 0 . 1023 [3:0] SF, range 0 . 9 */ 
    uint16_t          sfnsf;

    /* Number of SRs included in this message.  */
    uint16_t          numOfSr;

    /* For (Number of SRs) */
    FAPI_srPduIndication_st srPduInfo[FAPI_VAR_SIZE(numOfSr)];

}FAPI_rxSRIndication_st;

/*  ------------------------------- */

/* UL RX CQI INDICATION  : This defines the message body for  
 * PHY_UL_RX_CQI_INDICATION . */

typedef struct FAPI_rxCqiIndication_stT
{
    /* The SFN/SF of the SUBFRAME this information was received in.
    A 16-bit  value where,[15:4] SFN, range 0 . 1023 [3:0] SF, range 0 . 9 */ 
    uint16_t          sfnsf;

    /* Number of Cqi included in this message.  */
    uint16_t          numOfCqi;

    /* For (Number of Cqis) */
    FAPI_cqiPduIndication_st cqiPduInfo[FAPI_VAR_SIZE(numOfCqi)];

    /* Contents of PDU#1....PDU#n These will be CQI PDUs. */
    uint8_t                pduBuffer[FAPI_VAR_SIZE(x)];

}FAPI_rxCqiIndication_st;
/*  ------------------------------- */

/* UL RACH INDICATION  : This defines the message body for  
 * PHY_UL_RACH_INDICATION . */

typedef struct FAPI_rachIndication_stT
{
    /* The SFN/SF of the SUBFRAME this information was received in.
    A 16-bit  value where,[15:4] SFN, range 0 . 1023 [3:0] SF, range 0 . 9 */ 
    uint16_t              sfnsf;

    /* Number of RACH preambles */
    uint16_t              numOfPreamble;
#ifdef ASTRI_INTF 
    uint8_t               numOfResource;
    uint8_t               padding[3];
#endif
    /* For (Number of Preambles) */
    FAPI_rachPduIndication_st   rachPduInfo[FAPI_VAR_SIZE(numOfPreamble)];

#ifdef ASTRI_INTF 
//TBD 
   //FAPI_rachResourceIndication_st  rachResourceInfo[FAPI_VAR_SIZE(numOfResource)];
#endif

}FAPI_rachIndication_st;

/*  ------------------------------- */

/* UL SRS INDICATION  : This defines the message body for  
 * PHY_UL_SRS_INDICATION . */

typedef struct FAPI_srsIndication_stT
{
    /* The SFN/SF of the SUBFRAME this information was received in.
    A 16-bit  value where,[15:4] SFN, range 0 . 1023 [3:0] SF, range 0 . 9 */ 
    uint16_t           sfnsf;

    /* Number of UEs contributing to the uplink SRS */
    uint16_t           numOfUe;

    /* For (Number of UEs) */
    FAPI_srsPduIndication_st srsPduInfo[FAPI_VAR_SIZE(numOfUe)];

}FAPI_srsIndication_st;

/* SPR 1308 start*/
/* This structure defined the paramters 
  which received in the PARAM_RESPONSE message*/
typedef struct FAPI_paramRespParam_stT
{
     /* The PHY downlink channel bandwidth capability (in resource blocks). 

     Value: bitX :0 = no support, 1= support.
     Bit0: 6
     Bit1: 15
     Bit2: 25
     Bit3: 50
     Bit4: 75
     Bit5: 100     */
	uint16_t    dlBandWidthSupport;
     
     /* The PHY uplink channel bandwidth capability (in resource blocks). 

     Value: bitX :0 = no support, 1= support.
     Bit0: 6
     Bit1: 15
     Bit2: 25
     Bit3: 50
     Bit4: 75
     Bit5: 100     */
     uint16_t    ulBandWidthSupport;
    
     /* The PHY downlink modulation capability.
     Value: bitX :0 = no support, 1= support.
     Bit0: QPSK
     Bit1: 16QAM
     Bit2: 64QAM */
     uint16_t    dlModulationSupport;

    /* The PHY uplink modulation capability.
    Value: bitX :0 = no support, 1= support.
    Bit0: QPSK
    Bit1: 16QAM
    Bit2: 64QAM    */
    uint16_t    ulModulationSupport;
   
    /* Number of antennas supported.
    Value: 1, 2, 4  */
    uint16_t    phyAntennaCapability;

}FAPI_paramRespParam_stT;
/* SPR 1308  end */

/* +DYNAMIC_ICIC */
typedef struct FAPI_measReportInd_stT 
{
    uint32_t bitmask;

    int32_t interferencePerPRB[100];

    int32_t thermalNoise;

    int32_t dlRsTxPower;

}FAPI_measReportInd_st;

typedef struct FAPI_measReportInfo_stT
{
    uint16_t          sfnsf;

    /* For (Number of Reports) */
    FAPI_measReportInd_st measReportInfo;

}FAPI_measReportInfo_st;
/* -DYNAMIC_ICIC */
/* Vendor Specific Structures */
#ifdef INTF_9131

/* This structure defines UE specific information required in PHY to identify a UE record.
   This field will be sent as vendor specific message body for each PDU after UL_Config_Req message  */
typedef struct FAPI_vendorSpecificUEId_stT
{
    /* UE ID to identify UE record in PHY */
    uint8_t            ueId;
    uint8_t            padding[3];
}FAPI_vendorSpecificUEId_st;

/* This structure defines Freescale error code from PHY to MAC in vendor specific message body of error indication message */
typedef struct FAPI_vendorSpecificErrorCode_stT
{
    /* Freescale specific error codes */
    uint32_t           errorCode;
}FAPI_vendorSpecificErrorCode_st;
#endif
/*  ------------------------------- */
/* Set the alignment back to default. Only structures in this file are aligned
 * to 32 bit boundary. */
#pragma pack() 

#endif /* LTE_FAPI_MAC_PHY_INTERFACE_H */
