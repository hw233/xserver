#ifndef RESCONV_H
#define RESCONV_H


#ifndef TDR_METALIB_RESCONV_VERSION 
#define TDR_METALIB_RESCONV_VERSION 	7 /*version of metalib*/
#endif

#ifndef TDR_METALIB_RESCONV_HASH 
#define TDR_METALIB_RESCONV_HASH 	"8b987e62a3fd646200ba80f8623ca307" /*hash of metalib*/
#endif

/*   Define c types.   */
#ifndef TDR_CUSTOM_C_TYPES
#define TDR_CUSTOM_C_TYPES
    #include <stddef.h>
    #include <time.h>
    #include <sys/types.h>

    #if !defined(_WIN32) && !defined(_WIN64)

        #include <stdint.h>
        #include <inttypes.h>

    #else /*if !defined(_WIN32) && !defined(_WIN64)*/

        //The stdint declaras
        typedef  signed char  int8_t;
        typedef  short int16_t;
        typedef  int   int32_t;
        typedef unsigned char  uint8_t;
        typedef unsigned short uint16_t;
        typedef unsigned int   uint32_t;
        #if _MSC_VER >= 1300
            typedef unsigned long long 	uint64_t;
            typedef long long 	int64_t;
        #else /* _MSC_VER */
            typedef unsigned __int64	uint64_t;
            typedef __int64	int64_t;
        #endif /* _MSC_VER */

    #endif /*if !defined(_WIN32) && !defined(_WIN64)*/

    typedef int64_t tdr_longlong;
    typedef uint64_t tdr_ulonglong;
    typedef uint16_t tdr_wchar_t;  /**<Wchar基本数据类型*/
    typedef uint32_t tdr_date_t;	/**<data基本数据类型*/
    typedef uint32_t tdr_time_t;	/**<time基本数据类型*/
    typedef uint64_t tdr_datetime_t; /**<datetime基本数据类型*/
    typedef uint32_t tdr_ip_t;  /**<IPv4数据类型*/
#endif /*TDR_CUSTOM_C_TYPES*/


/*   User defined includes.   */


/*   User defined macros.   */
#define TRES_FILE_MAGIC                                  	1397052237
#define TRES_TRANSLATE_METALIB_HASH_LEN                  	36 	/* 元数据库hash值的长度 */
#define TRES_ENCORDING_LEN                               	32 	/* max length for encording string */


/*   Structs/unions prototype.   */
struct tagTResHead;
typedef struct tagTResHead                                         	TRESHEAD;
typedef struct tagTResHead                                         	*LPTRESHEAD;

struct tagTResHeadExt;
typedef struct tagTresHeadExt                                       TRESHEADEXT;
typedef struct tagTresHeadExt                                       *LPTRESHEADEXT;

typedef struct tagTResHeadAll                                               TRESHEADALL;
typedef struct tagTResHeadAll                                               *LPTRESHEADALL;
/*   Define structs/unions.   */
#pragma pack(1)

/* struct of the Head of Resource File */
struct tagTResHead
{
    int32_t iMagic;                                   	/*   Magic number of Resource file */
    int32_t iVersion;                                 	/*   version of the Resource file */
    int32_t iUnit;                                    	/*   size of each resource calced by Byte */
    int32_t iCount;                                   	/*   total count of resources */
    char szMetalibHash[TRES_TRANSLATE_METALIB_HASH_LEN]; 	/*  Ver.2 hash of the resource metalib */
    int32_t iResVersion;                              	/*  Ver.3 version of the Resource struct */
    tdr_datetime_t tCreateTime;                       	/*  Ver.4 create time of resource file */
    char szResEncording[TRES_ENCORDING_LEN];          	/*  Ver.4 encording of the rescource data */
    char szContentHash[TRES_TRANSLATE_METALIB_HASH_LEN]; 	/*  Ver.5 hash of the content */
};


struct tagTresHeadExt
{
	int32_t iDataOffset;
	int32_t iBuff;
};

/* struct of the Head of Resource File */
struct tagTResHeadAll
{
	TRESHEAD stResHead;
	TRESHEADEXT stResHeadExt;
};

#pragma pack()


#endif /* RESCONV_H */
