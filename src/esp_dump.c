#include	<stdio.h>

void main(int argc, char *argv[]) 
{
        FILE    *inpFps;
	int	buf;
	int	i = 0, j = 0;
		
	if (argc != 2) {
	} else {

		inpFps = fopen(argv[1], "r");
		if (inpFps == NULL) {
			fprintf(stderr,"Can not find input file!\n");
			fclose(inpFps);
		}
		printf("#object\n");
		
//		while ((buf = fgetc(inpFps)) != EOF ){
		for (j = 0;j < 2; j++) {
			for (i = 0; i < 768 * 4; i++){
				buf = fgetc(inpFps);
				if (!(i%4)) {
					printf("%d %03d ", j, i / 4);
				}
				printf("%02x", buf);
				if ((i%4) == 3) {
					printf(";\n");
				}
			}
		}
	}
}

				
			
/*			if (!(i%4)) {
				printf("0x%06x ", i);
			}
			printf("%02x", buf);
			if ((i%4) == 3) {
				printf("\n");
			}
			i++;
		}
	}
}
*/
					
