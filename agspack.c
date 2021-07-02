#define _GNU_SOURCE
#include "Clib32.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "version.h"
#define ADS ":::AGSpack " VERSION "by rofl0r:::"

static int usage(char *argv0) {
	dprintf(2, ADS
		"\nusage:\n%s OPTIONS directory target-pack\n\n"
		"OPTIONS:\n"
		"-e: recreate original exe stub\n"
	, argv0);
	return 1;
}

int main(int argc, char** argv) {
	int c, exe_opt = 0;
	while((c = getopt(argc, argv, "e")) != -1) switch(c) {
		default: return usage(argv[0]);
		case 'e': exe_opt = 1; break;
	}
	if (!argv[optind] || !argv[optind+1])
		return usage(argv[0]);

	char *dir = argv[optind];
	char *pack = argv[optind+1];
	char fnbuf[512];
	char line[1024];
	FILE* fp;
	snprintf(fnbuf, sizeof(fnbuf), "%s/%s", dir, "agspack.info");
	if(!(fp = fopen(fnbuf, "r"))) {
		dprintf(2, "couldnt open %s\n", fnbuf);
		return 1;
	}
	if(exe_opt) {
		snprintf(fnbuf, sizeof(fnbuf), "%s/%s", dir, "agspack.exestub");
		if(access(fnbuf, R_OK) == -1) {
			dprintf(2, "exestub requested, but couldnt read %s\n", fnbuf);
			return 1;
		}
	}
	size_t index = 0;
	struct AgsFile ags_b, *ags = &ags_b;
	AgsFile_init(ags, pack);
	AgsFile_setSourceDir(ags, dir);
	AgsFile_setDataFileCount(ags, 1); //TODO
	if(!AgsFile_setDataFile(ags, 0, "AGSPACKv" VERSION)) {
		dprintf(2, "error: packname exceeds 20 chars");
		return 1;
	}
	if(exe_opt) AgsFile_setExeStub(ags, "agspack.exestub");

	while(fgets(line, sizeof(line), fp)) {
		size_t l = strlen(line);
		if(l) line[l - 1] = 0;
		char *p = strchr(line, '=');
		if(!p) return 1;
		*p = 0; p++;
		if(0) ;
		else if(strcmp(line, "agsversion") == 0)
			AgsFile_setVersion(ags, atoi(p));
		else if(strcmp(line, "filecount") == 0)
			AgsFile_setFileCount(ags, atoi(p));
		else if(isdigit(*line))
			if(!AgsFile_setFile(ags, index++, p)) {
				perror(p);
				return 1;
			}
	}
	fclose(fp);
	size_t l = AgsFile_getFileCount(ags);
	for(index = 0; index < l; index++) {
		// TODO read from input file, but it seems to be all 0 for some games.
		AgsFile_setFileNumber(ags, index, 0);
	}
	int ret = AgsFile_write(ags);
	if(!ret) perror("write");
	AgsFile_close(ags);
	return !ret;
}

