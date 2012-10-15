//
// signGP.c
// Read the x-load.bin file and write out the x-load.bin.ift file.
// The signed image is the original pre-pended with the size of the image
// and the load address.  If not entered on command line, file name is
// assumed to be x-load.bin in current directory and load address is
// 0x40200800.

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <malloc.h>


main(int argc, char *argv[])
{
	int	i;
	char	ifname[FILENAME_MAX], ofname[FILENAME_MAX], ch = 0;
	FILE	*ifile, *ofile;
	unsigned long	loadaddr, len;
	struct stat	sinfo;
	int pad = 0;
    int endaddr = 0x4020F000;

	// Default to x-load.bin and 0x40200800.
	strcpy(ifname, "x-load.bin");
	loadaddr = 0x40200800;

	if ((argc == 2) || (argc == 3))
		strcpy(ifname, argv[1]);

	if (argc == 3)
		loadaddr = strtol(argv[2], NULL, 16);

	// Form the output file name.
	strcpy(ofname, ifname);
	strcat(ofname, ".ift");

	// Open the input file.
	ifile = fopen(ifname, "rb");
	if (ifile == NULL) {
		printf("Cannot open %s\n", ifname);
		exit(0);
	}

	// Get file length.
	stat(ifname, &sinfo);
	len = sinfo.st_size;

	// Open the output file and write it.
	ofile = fopen(ofname, "wb");
	if (ofile == NULL) {
		printf("Cannot open %s\n", ofname);
		fclose(ifile);
		exit(0);
	}

	// Pad 1 sector of zeroes.
	//ch = 0x00;
	//for (i=0; i<0x200; i++)
	//	fwrite(&ch, 1, 1, ofile);

	/*
	 * Workaround TI OMAP 3630 MLO Boot bug on Kunlun P2
	 * It can be found when the size of final ift named MLO
	 * is just 21000, BOOTROM fail to load MLO:
	 * No banner messages on serial, system hang.
	 * This may be a bug of TI's BOOTROM.
	 *
	 * - We append 4 bytes to workaround this.
	 *   <zhi.wang@windriver.com>
	 */

	//if (len + 8 == 21000)
		//pad = 4;
    
    /*
     * Workaround: the error will be trigged not only at 21000 but some other size, 
     * so pad the MLO to be max valid SRAM to avoid the error
     * <qli@via-telecom.com>
     *
     */
    if((len + 8) < (endaddr - loadaddr)){
        pad = endaddr - loadaddr - len - 8;
    }else{
        printf("Error: the size of x-loader.bin(%lu) is more than max SRAM(%lu).\n", len, endaddr-loadaddr-8);
        fclose(ifile);
	    fclose(ofile);
        exit(-1);
    }

	len += pad;

	fwrite(&len, 1, 4, ofile);
	fwrite(&loadaddr, 1, 4, ofile);
	for (i=0; i<len-pad; i++) {
		fread(&ch, 1, 1, ifile);
		fwrite(&ch, 1, 1, ofile);
	}

	/* Pad dummy bytes. */
	for (i = 0; i < pad; i++)
		fwrite(&ch, 1, 1, ofile);

	fclose(ifile);
	fclose(ofile);
}
