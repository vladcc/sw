/* ic -- icon change v1.0
 * this module can change the icons of executable files */

// includes
#include <stdio.h>
#include <windows.h>
#include <string.h>

// structs and typedefs
typedef struct
{
    BYTE        bWidth;          // Width, in pixels, of the image
    BYTE        bHeight;         // Height, in pixels, of the image
    BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
    BYTE        bReserved;       // Reserved ( must be 0)
    WORD        wPlanes;         // Color Planes
    WORD        wBitCount;       // Bits per pixel
    DWORD       dwBytesInRes;    // How many bytes in this resource?
    DWORD       dwImageOffset;   // Where in the file is this image?
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct
{
    WORD           idReserved;   // Reserved (must be 0)
    WORD           idType;       // Resource Type (1 for icons)
    WORD           idCount;      // How many images?
    //LPICONDIRENTRY   idEntries; // An entry for each image (idCount of 'em) - we do not need it
} ICONDIR, *LPICONDIR;

typedef struct
{
   BITMAPINFOHEADER   icHeader;   // DIB header
   RGBQUAD         icColors[1];   // Color table
   BYTE            icXOR[1];      // DIB bits for XOR mask
   BYTE            icAND[1];      // DIB bits for AND mask
} ICONIMAGE, *LPICONIMAGE;

typedef struct
{
   BYTE   bWidth;               // Width, in pixels, of the image
   BYTE   bHeight;              // Height, in pixels, of the image
   BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
   BYTE   bReserved;            // Reserved
   WORD   wPlanes;              // Color Planes
   WORD   wBitCount;            // Bits per pixel
   DWORD  dwBytesInRes;         // how many bytes in this resource?
   WORD   nID;                  // the ID
} GRPICONDIRENTRY, *LPGRPICONDIRENTRY;

typedef struct 
{
   WORD            idReserved;   // Reserved (must be 0)
   WORD            idType;       // Resource type (1 for icons)
   WORD            idCount;      // How many images?
   //LPGRPICONDIRENTRY idEntries; // The entries for each image - we do not need it
} GRPICONDIR, *LPGRPICONDIR;

//constans
const char REMOVE_RES[] = "-r";

// code
int main(int argc, char * argv[])
{
	/* it's a small program, do everything in main */
	
	if (3 > argc)
	{
		printf("Use:\n%s <exe> <icon>    - change icon\n", argv[0]);
		printf("%s <exe> <icon> -r - change icon and remove all other resources\n", argv[0]);
		return -1;
	}
	
	
	HRSRC hUpdateRes;
	HANDLE hFile;
	ICONDIR * pIconDir;
	ICONDIRENTRY * pIconDirEntry;
	GRPICONDIR * pGrpIconDir;
	GRPICONDIRENTRY * pGrpIconDirEntry;
	DWORD dwBytesRead;
	BOOL result;
	BOOL remove_resources = FALSE;
	int i, iID;
	
	if ((4 == argc) && (0 == strcmp(REMOVE_RES, argv[3])))
		remove_resources = TRUE;
	
	// open icon file for reading
	hFile = CreateFile(argv[2], GENERIC_READ, 
			   0,
			   NULL,
			   OPEN_EXISTING,
			   FILE_ATTRIBUTE_NORMAL,
			   NULL);
			   
	if (INVALID_HANDLE_VALUE == hFile)
	{
		printf("Err: couldn't open %s\n", argv[2]);
		return -1;
	}
			   
	// we need an ICONDIR to hold the data
	int size = GetFileSize(hFile, NULL); 
	pIconDir = malloc(size);

	/* read the whole file
	 * we are going to treat the .ico file as a flat array of bytes */
	ReadFile(hFile, pIconDir, size, &dwBytesRead, NULL);
	
	// open exe for resource update
	hUpdateRes = BeginUpdateResource(argv[1], remove_resources);
	if (NULL == hUpdateRes)
	{
		printf("Err: couldn't open %s\n", argv[1]);
		return -1;
	}

	size = (sizeof(WORD) * 3 + (pIconDir->idCount * 14)); 
	
	int saveSize = size;
	/* allocate space for group icon dir + as many icon dir entries 
	 * as there are icons in the file */
	pGrpIconDir = malloc(size);
	pGrpIconDir->idReserved = pIconDir->idReserved;
	pGrpIconDir->idType = pIconDir->idType;
	pGrpIconDir->idCount = pIconDir->idCount;
	
	int j;
	for (i = 0, j = 0, iID = 400; i < pIconDir->idCount; ++i, ++iID, ++j)
	{
		// iterate and add the icons to the resource section
		char *  bPoint = (char *)pIconDir;
		bPoint += 6 + sizeof(ICONDIRENTRY) * j;

		//pIconDirEntry = pIconDir + 6 + 16 * i;
		pIconDirEntry = (LPICONDIRENTRY)bPoint;
		size = pIconDirEntry->dwBytesInRes;
		
		char * rsrcSave;
		rsrcSave = (char *)pIconDir;
		rsrcSave += pIconDirEntry->dwImageOffset;
		
		result = UpdateResource(hUpdateRes,		// update resource handle
		RT_ICON,                        		// change dialog box resource
		MAKEINTRESOURCE(iID),					// group icon id
		0, 										// neutral language
		rsrcSave,								// ptr to resource info
		size);									// size of resource info
		if (result == FALSE)
		{
			printf("Err: couldn't add resource.\n");
			return -1;
		}
		
		bPoint = (char *)pGrpIconDir;
		
		bPoint += 6 + 14 * j;
		pGrpIconDirEntry =(LPGRPICONDIRENTRY)bPoint;
		pGrpIconDirEntry->bWidth = pIconDirEntry->bWidth;
		pGrpIconDirEntry->bHeight = pIconDirEntry->bHeight;
		pGrpIconDirEntry->bColorCount = pIconDirEntry->bColorCount;
		pGrpIconDirEntry->bReserved = pIconDirEntry->bReserved;
		pGrpIconDirEntry->wPlanes = pIconDirEntry->wPlanes;
		pGrpIconDirEntry->wBitCount = pIconDirEntry->wBitCount;
		pGrpIconDirEntry->dwBytesInRes = pIconDirEntry->dwBytesInRes;
		pGrpIconDirEntry->nID = iID;
	}
	
	// set as main icon
	result = UpdateResourceA(hUpdateRes,	// update resource handle
	RT_GROUP_ICON,							// change group icon resource
	"MAINICON",								// group icon id
	0,										// neutral language
	pGrpIconDir,							// ptr to resource info
	saveSize);								// size of resource info
	if (result == FALSE)
	{
		printf("Err: couldn't add resource.\n");
		return -1;
	}
	
	if (!EndUpdateResource(hUpdateRes, FALSE))
	{
		printf("Err: couldn't write changes to file.\n");
		return -1;
	}
	
	free(pIconDir);
	free(pGrpIconDir);
	puts("Icon change successful.");

	return 0;
}
