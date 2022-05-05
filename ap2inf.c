/*
 * ap2inf - Convert apl2em disk files of Infocom games into standard
 *          data files. Currently only works on single disk games,
 *          so no multi-disk version 4 games can be done.
 *
 * Stephen Tjasink 1994 - 1997
 *
 */

#ifdef __MSDOS__
#include <mem.h>    /* for memcpy() prototype */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Size and type definitions */

#define DISKSIZE   143360
#define TRACKSIZE  4096
#define SECTORSIZE 256

/* these definitions seem fairly safe... 8, 16 and 32  bits respectively */

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long lword;

/* global variables */

int ppiSectorOrder[2][16] =
{
  {0x0,0xd,0xb,0x9,0x7,0x5,0x3,0x1,0xe,0xc,0xa,0x8,0x6,0x4,0x2,0xf},
  {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf}
};
int   iInterleaveScheme = -1;
lword lwDataLength = 0;
word  wChecksum = 0;   /* there are no games with 0x0000 as valid checksum */
int   iPadding = 1;
int   iZVersion;
char* pszDiskFile = NULL;
char* pszDataFile = NULL;
char* pszProgName;
int   iNumTracks;
int   iBytesOver;

/***************************************************************************/

void* SafeMalloc
(
  size_t uSize
)
{
  void* pvPtr;

  pvPtr = calloc(1, uSize);
  if (pvPtr == NULL)
  {
    fprintf(stderr,"Error allocating %u bytes of memory.\n", uSize);
    exit(-1);
  }
  return pvPtr;
}

/***************************************************************************/

void ReadDskTrack
(
  FILE* pstFile,
  int   iTrackNum,
  byte* pbTrackPtr
)
{
  int   i;
  byte* pbTempTrack;

  pbTempTrack = (byte*) SafeMalloc(TRACKSIZE * sizeof(byte));
  fseek(pstFile, (long) iTrackNum * TRACKSIZE, SEEK_SET);
  fread(pbTempTrack, TRACKSIZE, 1, pstFile);
  for (i = 0; i < 16; i++)
  {
    memcpy(pbTrackPtr + i * SECTORSIZE, pbTempTrack + ppiSectorOrder[iInterleaveScheme][i] * SECTORSIZE, SECTORSIZE);
  }
  free(pbTempTrack);
}

/***************************************************************************/

void ReadDataTrack
(
  FILE* pstFile,
  int   iTrackNum,
  byte* pbTrackPtr
)
{
  fseek(pstFile, (long) iTrackNum * TRACKSIZE, SEEK_SET);
  fread(pbTrackPtr, TRACKSIZE, 1, pstFile);
}

/***************************************************************************/

void WriteDataTrack
(
  FILE* pstFile,
  int   iTrackNum,
  byte* pbTrackPtr,
  int   iNumBytes
)
{
  fseek(pstFile, (long) iTrackNum * TRACKSIZE, SEEK_SET);
  fwrite(pbTrackPtr, iNumBytes, 1, pstFile);
}

/***************************************************************************/

void GetInterleave
(
  FILE* pstFile
)
{
  byte* pbSector;
  char* pszInterpreter = NULL;

  if (iInterleaveScheme != -1)
  {
    printf("Using user-specified interleave scheme %d.\n", iInterleaveScheme);
    return;
  }
  else
  {
    pbSector = (byte*) SafeMalloc(SECTORSIZE * sizeof(byte));
    fseek(pstFile, 0L, SEEK_SET);
    fread(pbSector, SECTORSIZE * sizeof(byte), 1, pstFile);
    switch (pbSector[0x33])
    {
      case 0x08:
      {
        pszInterpreter = "B";
        iInterleaveScheme = 0;
        break;
      }
      case 0x49:
      {
        pszInterpreter = "E";
        iInterleaveScheme = 1;
        break;
      }
      case 0x3a:
      {
        pszInterpreter = "H";
        iInterleaveScheme = 0;
        break;
      }
      case 0xe1:
        pszInterpreter = "H (cracked by The Digital Gang)";
        iInterleaveScheme = 0;
        break;
      {
      }
      case 0x85:
      {
        pszInterpreter = "K (cracked by Coast to Coast)";
        iInterleaveScheme = 0;
        break;
      }
      case 0x3d:
      {
        pszInterpreter = "M";
        iInterleaveScheme = 0;
        break;
      }
      default:
      {
        iInterleaveScheme = 0;
      }
    }
    free(pbSector);
  }

  if (pszInterpreter)
  {
    printf("Apple ][ interpreter %s detected. Using interleave scheme %d.\n", pszInterpreter, iInterleaveScheme);
  }
  else
  {
    printf("Unknown interpreter. Using interleave scheme 0.\nSpecify scheme 1 with -i if it fails.\n");
  }
}

/***************************************************************************/

void LoadVariables
(
  FILE* pstFile
)
{
  byte* pbTrack;

  pbTrack = (byte*) SafeMalloc(TRACKSIZE * sizeof(byte));
  ReadDskTrack(pstFile, 3, pbTrack);

  iZVersion = pbTrack[0];
  printf("ZCode version is %d.\n", iZVersion);

  if (lwDataLength != 0)
  {
    printf("Using user-specified data length %lu.\n", lwDataLength);
  }
  else
  {
    lwDataLength = pbTrack[0x1a];
    lwDataLength <<= 8;
    lwDataLength += pbTrack[0x1b];
    switch (iZVersion)
    {
      case 6:
      {
        lwDataLength *= 2;
      }
      case 5:
      case 4:
      {
        lwDataLength *= 2;
      }
      case 3:
      case 2:
      case 1:
      {
        lwDataLength *= 2;
      }
    }
    if (lwDataLength == 0)
    {
      printf("No data length present in game header or specified with -l.\nUsing maximum length of 131072 bytes.\n");
      lwDataLength = 131072UL;
    }
    else
    {
      printf("Using data length in game header: %lu bytes.\n", lwDataLength);
    }
  }

  if (wChecksum != 0)
  {
    printf("Using user-specified checksum %#04x\n", wChecksum);
  }
  else
  {
    wChecksum = pbTrack[0x1c];
    wChecksum <<= 8;
    wChecksum += pbTrack[0x1d];
    if (wChecksum == 0)
    {
      printf("No checksum present in game header or specified with -c.\n");
    }
    else
    {
      printf("Using checksum in game header: %#04x\n", wChecksum);
    }
  }

  free(pbTrack);
}

/***************************************************************************/

void PrintInstructions
(
  void
)
{
  fprintf(stderr, "Usage: %s [options] <input file> <output file>\n\n", pszProgName);
  fprintf(stderr, "Where <input file>  is an apl2em-compatible Apple ][ disk image\n");
  fprintf(stderr, "  and <output file> is the name of the data file to be created.\n");
  fprintf(stderr, "  and [options]     is one or more of the following:\n");
  fprintf(stderr, "      -h     displays this help\n");
  fprintf(stderr, "      -ix    interleave type: x is 0 or 1\n");
  fprintf(stderr, "      -lxxx  data length: xxx is a positive number of bytes not 0\n");
  fprintf(stderr, "      -cxxx  checksum: xxx an unsigned 16-bit number not 0\n");
  fprintf(stderr, "      -pxxx  padding: pad with 0's to a multiple of xxx bytes\n");
  fprintf(stderr, "  For more information, read the ap2inf README file\n");
}

/***************************************************************************/

void ExtractData
(
  FILE* pstInFile,
  FILE* pstOutFile
)
{
  int   i;
  byte* pbCurrTrack;
  int   iPadAmount;
  byte* pbPadding;

  printf("Extracting data file...\n\n");

  /* Work out number of tracks and number of bytes used in last track */
  iNumTracks = (int) (lwDataLength / TRACKSIZE);
  iBytesOver = (int) (lwDataLength - iNumTracks * TRACKSIZE);

  /* Print out numbers and dots */
  for (i = 0; i < 4 + iNumTracks; i++)
  {
    printf("%01x ", i & 0x0f);
  }
  printf("\n");
  printf(". . . ");

  pbCurrTrack = (byte*) SafeMalloc(TRACKSIZE * sizeof(byte));

  /* Process all the full tracks */
  for (i = 3; i < 3 + iNumTracks; i++)
  {
    ReadDskTrack(pstInFile, i, pbCurrTrack);
    WriteDataTrack(pstOutFile, i - 3, pbCurrTrack, TRACKSIZE);
    printf(". ");
  }

  /* Process last track */
  ReadDskTrack(pstInFile, i, pbCurrTrack);
  WriteDataTrack(pstOutFile, i - 3, pbCurrTrack, iBytesOver);
  printf(". \n\n");

  /* Add padding */
  if (iPadding > 1)
  {
    iPadAmount = iPadding - (int) (lwDataLength % iPadding);
    printf("Padding to a multiple of %d bytes.\n\n", iPadding);
    pbPadding = (byte*) SafeMalloc(sizeof(byte) * iPadAmount);
    fwrite(pbPadding, iPadAmount, 1, pstOutFile);
    free(pbPadding);
  }

  fclose(pstInFile);
  fclose(pstOutFile);
  free(pbCurrTrack);
  printf("Data file %s has been written.\n\n", pszDataFile);
}

/***************************************************************************/

void CheckChecksum
(
  FILE* pstInFile
)
{
  int i,j;
  word wCheckTemp;
  byte* pbCurrTrack;

  printf("Verifying data file...\n\n");

  wCheckTemp = 0;
  pbCurrTrack = (byte*) SafeMalloc(sizeof(byte) * TRACKSIZE);

  /* Skip 64 bytes of first track */
  ReadDataTrack(pstInFile, 0, pbCurrTrack);
  for (j = 64; j < TRACKSIZE; j++)
  {
    wCheckTemp += pbCurrTrack[j];
  }

  /* Count all the bytes of the other full tracks */
  for (i = 1; i < iNumTracks; i++)
  {
    ReadDataTrack(pstInFile,i,pbCurrTrack);
    for (j = 0; j < TRACKSIZE; j++)
    {
      wCheckTemp += pbCurrTrack[j];
    }
  }

  /* Count valid bytes of last track */
  ReadDataTrack(pstInFile, i, pbCurrTrack);
  for (j = 0; j < iBytesOver; j++)
  {
    wCheckTemp += pbCurrTrack[j];
  }

  free(pbCurrTrack);

  /* Output result */
  if (wCheckTemp == wChecksum)
  {
    printf("Data file verified as correct.\n");
  }
  else
  {
    printf("Data file corrupt. Required checksum = %#04x. Actual checksum = %#04x.\n", wChecksum, wCheckTemp);
  }
}

/***************************************************************************/

void GetOptions
(
  int    argc,
  char** argv
)
{
  int iParameter;
  int iParamLength;

  pszProgName = (char*) malloc((strlen(argv[0]) + 1) * sizeof(char));
  strcpy(pszProgName, argv[0]);
  for (iParameter = 1; iParameter < argc; iParameter++)
  {
    if (argv[iParameter][0] == '-')
    {
      iParamLength = strlen(argv[iParameter]);
      if (iParamLength < 2)
      {
        continue;
      }
      switch (argv[iParameter][1])
      {
        case 'i':
        {
          if (iParamLength < 3)
          {
            fprintf(stderr, "Must specify 0 or 1 with -i.\n");
            exit(-1);
          }
          iInterleaveScheme = argv[iParameter][2] - '0';
          if (iInterleaveScheme > 1)
          {
            fprintf(stderr, "Must specify 0 or 1 with -i.\n");
            exit(-1);
          }
          break;
        }
        case 'l':
        {
          if (iParamLength < 3)
          {
            fprintf(stderr, "Must specify a nonzero number with -l.\n");
            exit(-1);
          }
          lwDataLength = strtol(argv[iParameter] + 2, NULL, 0);
          if (lwDataLength == 0)
          {
            fprintf(stderr, "Must specify a nonzero number with -l.\n");
            exit(-1);
          }
          break;
        }
        case 'c':
        {
          if (iParamLength < 3)
          {
            fprintf(stderr, "Must specify a nonzero number below 65536 with -c.\n");
            exit(-1);
          }
          wChecksum = (word) strtol(argv[iParameter] + 2, NULL, 0);
          if (wChecksum == 0)
          {
            fprintf(stderr, "Must specify a nonzero number below 65536 with -c.\n");
            exit(-1);
          }
          break;
        }
        case 'p':
        {
          if (iParamLength < 3)
          {
            fprintf(stderr, "Must specify a nonzero number with -p.\n");
            exit(-1);
          }
          iPadding = (int) strtol(argv[iParameter] + 2, NULL, 0);
          if (iPadding == 0)
          {
            fprintf(stderr, "Must specify a nonzero number with -p.\n");
            exit(-1);
          }
          break;
        }
        case 'h':
        {
          PrintInstructions();
          exit(-1);
          break;
        }
        default:
        {
          fprintf(stderr, "Illegal option -%c\n", argv[iParameter][1]);
          exit(-1);
        }
      }
    }
    else
    {
      if (pszDiskFile == NULL)
      {
        pszDiskFile = (char*) malloc((strlen(argv[iParameter]) + 1) * sizeof(char));
        strcpy(pszDiskFile, argv[iParameter]);
      }
      else if (pszDataFile == NULL)
      {
        pszDataFile = (char*) malloc((strlen(argv[iParameter]) + 1) * sizeof(char));
        strcpy(pszDataFile, argv[iParameter]);
      }
    }
  }
  if (pszDiskFile == NULL || pszDataFile == NULL)
  {
    PrintInstructions();
    exit(-1);
  }
}

/***************************************************************************/

int main
(
  int    argc,
  char** argv
)
{
  FILE* pstInFile;
  FILE* pstOutFile;

  printf("ap2inf v1.4.2 by Stephen Tjasink, 1994 - 1997.\n\n");
  GetOptions(argc, argv);
  pstInFile = fopen(pszDiskFile, "rb");
  if (pstInFile == NULL)
  {
    fprintf(stderr, "Error opening %s for reading.\n", pszDiskFile);
    exit(-1);
  }
  else
  {
    printf("Opened %s for reading as disk file\n", pszDiskFile);
  }
  pstOutFile = fopen(pszDataFile, "wb");
  if (pstOutFile == NULL)
  {
    fprintf(stderr, "Error opening %s for writing.\n", pszDataFile);
    exit(-1);
  }
  GetInterleave(pstInFile);
  LoadVariables(pstInFile);
  ExtractData(pstInFile, pstOutFile);
  if (wChecksum)
  {
    pstInFile = fopen(pszDataFile, "rb");
    if (pstInFile == NULL)
    {
      fprintf(stderr,"Error opening %s as input file for checksum.\n", pszDataFile);
      exit(-1);
    }
    CheckChecksum(pstInFile);
  }
  return 0;
}

/***************************************************************************/

