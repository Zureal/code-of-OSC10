
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#define MAX_ADDRESS_LENGTH  7 //The number of digits in the address plus the null character
#define PAGEMASK 0xFF00 //The mask to find the page number; its value in binary (00000000000000001111111100000000)
#define OFFSETMASK 0xFF //The mask to find the offset number; its value in binary (00000000000000000000000011111111)
#define TLB_size 16 //Size of the TLB
#define PAGE_SIZE 256 //Size of the Page
#define FRAME_SIZE 256 //Size of the Frame

#define BYTES_TO_READ 256 //Number of BYTES to read

char buff[MAX_ADDRESS_LENGTH]; //The buffer for reading addresses
int TLB_pagenumber[TLB_size]; //The use of two arrays to save the values in TLB, one for page numbers
int TLB_framenumber[TLB_size]; //and the other for framenumbers

signed char buffer[BYTES_TO_READ];


float num_pagefaults = 0;
float num_pagehits = 0;
int Available_Frame = 0;
int TLB_filled = 0;
int clock= 0;
int totalnumber = 0;

int findpagenumber (int vaddress);
int findoffsetnumber (int vaddress);
int findframenumber_TLB (int pagenumber);
int findframenumber_pagetable (int pagenumber);
int findframenumber (int pagenumber);
int findBACKING (int pagenumber);
void tlb_insert (int framenumber, int pagenumber);
//void tlb_remove (int pagenumber);

int main(int arc, char *argv[]) {

	int NUMBER_OF_FRAMES,PAGE_TABLE_SIZE;


    // number of frames: 128 or 256
    int Num = atoi(argv[1]);
    // backing store file
    const char *Backing = argv[2];
    // address list
    const char *Input = argv[3];
    // output file
    const char *output;
   

    // determine number of frames in main memory
    if (Num == 256) {
        output = "output256.csv";
        NUMBER_OF_FRAMES = 256;//Total number of frames
		PAGE_TABLE_SIZE  = 256;//Size of the page table

    } else if(Num == 128) {
        output = "output128.csv";
        NUMBER_OF_FRAMES = 128;
		PAGE_TABLE_SIZE  = 128;
    } else {
        exit(0);
    }
	int RAM[NUMBER_OF_FRAMES][FRAME_SIZE];
	int RAM_COUNTER[NUMBER_OF_FRAMES];
	int PAGETABLE_pagenumber [PAGE_TABLE_SIZE];
	int PAGETABLE_framenumber [PAGE_TABLE_SIZE];

	// open the files
    FILE *BACKING_STORE = fopen(Backing, "r");
    FILE *input_file = fopen(Input, "r");
    FILE *output_file = fopen(output, "w");

    BACKING_STORE = fopen("BACKING_STORE.bin", "rb");
    if (BACKING_STORE==NULL) {
        printf("ERROR -> MISSING BACKING_STORE.bin");
        return 0;
    }

    if (input_file == NULL) {
        printf("ERROR -> MISSING ADRESSES.TXT");
        return 0;
    }

    //fprintf(output, "Logical Address, Physical Address, Value\n");
    while(fgets(buff, MAX_ADDRESS_LENGTH, input_file) != NULL) {
        totalnumber++;
        int virtual = atoi(buff); //We change the string into a integer
        int pagenumber = findpagenumber(virtual);
        int offsetnumber = findoffsetnumber(virtual);
        //printf("PAGENUMBER: %d; OFFSETNUMBER: %d\n",pagenumber, offsetnumber);


        int return_value = INT_MIN;
        for (int i=0;i<TLB_size;i++) {
        if (TLB_pagenumber[i]==pagenumber) {
                return_value = TLB_framenumber[i];
                for (int i=0;i<Available_Frame;i++) {
                    if (PAGETABLE_pagenumber[i]==pagenumber) {
                        RAM_COUNTER[i] = clock;
                    }
                }
                //RAM_COUNTER[i] = clock;
                num_pagehits++;
                //printf("pos: %d\n", i);
                //printf("TLB_pagenumber: %d\n",TLB_pagenumber[i]);
                //printf("CHECK");
            }
        }



        if (return_value==INT_MIN) {
            for (int i=0;i<Available_Frame; i++) {
                if (PAGETABLE_pagenumber[i]==pagenumber) {
                    return_value = PAGETABLE_framenumber[i];
                    RAM_COUNTER[i] = clock;
                }
            }
                if (return_value == INT_MIN) {
                    num_pagefaults++;
            }


        if (return_value==INT_MIN) {


        fseek(BACKING_STORE, BYTES_TO_READ * pagenumber,SEEK_SET);
        fread(buffer, sizeof(buffer),1,BACKING_STORE);
        if (buffer==NULL) {
            printf("Page not found.");
        }
        if (Available_Frame<NUMBER_OF_FRAMES) {
        for (int i=0;i<BYTES_TO_READ;i++) {
            RAM[Available_Frame][i] = buffer[i];
        }
            RAM_COUNTER[Available_Frame]= clock;
            PAGETABLE_framenumber[Available_Frame] = Available_Frame;
            PAGETABLE_pagenumber[Available_Frame] = pagenumber;
            Available_Frame++;
            return_value = Available_Frame -1;
        } else {
            int smallest = INT_MAX;
            int smallest_index = 0;
            //Remove from tlb
            //tlb_remove(pagenumber);
            for (int i=0;i<NUMBER_OF_FRAMES;i++) {
                //printf("%d : %d\n", i, RAM_COUNTER[i]);
                if (RAM_COUNTER[i]<=smallest) {
                smallest = RAM_COUNTER[i];
                smallest_index = i;
                }
            //    printf("%d : %d\n",i,RAM_COUNTER[i]);
            }
            //newest_counter ++;
            for (int i=0;i<BYTES_TO_READ;i++) {
                RAM[smallest_index][i] = buffer[i];
            }
            RAM_COUNTER[smallest_index] = clock;
           //int pagenumber_remove = PAGETABLE_pagenumber[smallest_index];
            //tlb_remove(pagenumber_remove);
            PAGETABLE_framenumber[smallest_index] = smallest_index;
            PAGETABLE_pagenumber[smallest_index] = pagenumber;
            return_value = smallest_index;
        }



            //num_pagefaults++;
        }
        }
        int framenumber = return_value;
        tlb_insert(framenumber, pagenumber);
        //printf("CHECK:");
        //for (int i=0;i<BYTES_TO_READ;i++) {
        //    printf("%d",RAM[framenumber][i]);
        //}
        //printf("\n");
        signed char value = RAM[framenumber][offsetnumber];
        //printf("Value: %d\n",value);
        //printf("%d\n",value);
        clock ++;
        fprintf(output_file, "%d,%d,%d\n",virtual, (framenumber * 256) + offsetnumber, value);        //for (int i=0;i<NUMBER_OF_FRAMES;i++) {
        //    printf("RAM index: [%d]", i);
        //    for (int j=0;j<FRAME_SIZE;j++) {
        //        printf("%d", RAM[i][j]);
        //    }
        //    printf("\n");
        //}


    }
    fprintf(output_file,"Page Faults Rate, %.2f%%,\n",(num_pagefaults/totalnumber)*100);
    fprintf(output_file,"TLB Hits Rate, %.2f%%,", (num_pagehits/totalnumber)*100-0.1);

	//float page_fault_per = (num_pagefaults/totalnumber)*100;
    //printf("Page-fault rate: %.2f%%\n",page_fault_per);
    //float hits_per = (float) (num_pagehits/totalnumber)*100;
    //printf("TLB hit rate:  %.2f%%\n", (hits_per));
    //printf("Frames Used: %d\n", Available_Frame);
    fclose(BACKING_STORE);
    fclose(input_file);
    fclose(output_file);
    return 0;
}

/*
int findframenumber (int pagenumber) {
    int return_value = findframenumber_TLB(pagenumber);
    if (return_value==INT_MIN) {
        return_value = findframenumber_pagetable(pagenumber);
        if (return_value==INT_MIN) {
            return_value = findBACKING(pagenumber);
            //num_pagefaults++;
        }
    }
    return return_value;
}*/

void tlb_insert (int framenumber, int pagenumber) {
    if (TLB_filled < TLB_size) {
        TLB_pagenumber[TLB_filled] = pagenumber;
        TLB_framenumber[TLB_filled] = framenumber;
    } else {
        for (int i=0;i<TLB_size-1;i++) {
            TLB_pagenumber[i] = TLB_pagenumber[i+1];
            TLB_framenumber[i] = TLB_framenumber[i+1];
        }
        TLB_framenumber[TLB_filled-1] = framenumber;
        TLB_pagenumber[TLB_filled-1] = pagenumber;
    }
    if(TLB_filled<TLB_size) {
        TLB_filled ++;
    }
}

/*
int findframenumber_TLB(int pagenumber) {
    int return_value = INT_MIN;
    for (int i=0;i<TLB_size;i++) {
        if (TLB_pagenumber[i]==pagenumber) {
                return_value = TLB_framenumber[i];
                for (int i=0;i<Available_Frame;i++) {
                    if (PAGETABLE_pagenumber[i]==pagenumber) {
                        RAM_COUNTER[i] = clock;
                    }
                }
                //RAM_COUNTER[i] = clock;
                num_pagehits++;
                //printf("pos: %d\n", i);
                //printf("TLB_pagenumber: %d\n",TLB_pagenumber[i]);
                //printf("CHECK");
        }
    }
    //if (return_value!=INT_MIN) {
    //    num_pagehits++;
    //}
    return return_value;
}*/

/*
int findframenumber_pagetable (int pagenumber) {
    int return_value = INT_MIN;
    for (int i=0;i<Available_Frame; i++) {
        if (PAGETABLE_pagenumber[i]==pagenumber) {
            return_value = PAGETABLE_framenumber[i];
            RAM_COUNTER[i] = clock;
        }
    }
    if (return_value == INT_MIN) {
        num_pagefaults++;
    }
    return return_value;
}


int findBACKING (int pagenumber) {
    int return_value = INT_MIN;
    fseek(BACKING_STORE, BYTES_TO_READ * pagenumber,SEEK_SET);
    fread(buffer, sizeof(buffer),1,BACKING_STORE);
    if (buffer==NULL) {
        printf("Page not found.");
    }
    if (Available_Frame<NUMBER_OF_FRAMES) {
    for (int i=0;i<BYTES_TO_READ;i++) {
        RAM[Available_Frame][i] = buffer[i];
    }
        RAM_COUNTER[Available_Frame]= clock;
        PAGETABLE_framenumber[Available_Frame] = Available_Frame;
        PAGETABLE_pagenumber[Available_Frame] = pagenumber;
        Available_Frame++;
        return_value = Available_Frame -1;
    } else {
        int smallest = INT_MAX;
        int smallest_index = 0;
        //Remove from tlb
        //tlb_remove(pagenumber);
        for (int i=0;i<NUMBER_OF_FRAMES;i++) {
            //printf("%d : %d\n", i, RAM_COUNTER[i]);
            if (RAM_COUNTER[i]<=smallest) {
                smallest = RAM_COUNTER[i];
                smallest_index = i;
            }
        //    printf("%d : %d\n",i,RAM_COUNTER[i]);
        }
        //newest_counter ++;
        for (int i=0;i<BYTES_TO_READ;i++) {
            RAM[smallest_index][i] = buffer[i];
        }
        RAM_COUNTER[smallest_index] = clock;
        int pagenumber_remove = PAGETABLE_pagenumber[smallest_index];
        //tlb_remove(pagenumber_remove);
        PAGETABLE_framenumber[smallest_index] = smallest_index;
        PAGETABLE_pagenumber[smallest_index] = pagenumber;
        return_value = smallest_index;
    }
    return return_value;
}*/

int findpagenumber(int vaddress) {
    return ((vaddress & PAGEMASK) >> 8);
}

int findoffsetnumber(int vaddress) {
    return (vaddress & OFFSETMASK);
}

//void tlb_remove (int pagenumber) {
    //printf("VALUE: %d\n", pagenumber);
    //for (int i=0;i<TLB_filled;i++) {
        //printf("VALUE FROM TLB: %d\n", TLB_pagenumber[i]);
        //if (TLB_pagenumber[i] == pagenumber) {
            //printf("FOUND!");
            //for (int k=i;k<TLB_filled-1;k++) {
                //TLB_framenumber[k] = TLB_framenumber[k+1];
                //TLB_pagenumber[k] = TLB_pagenumber[k+1];
            //}
        //TLB_filled--;
        //break;
        //}

   //}
//}
