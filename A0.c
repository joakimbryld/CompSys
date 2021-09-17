#include <stdio.h>
#include <assert.h>
#include <unistd.h>


int CheckIfAscii(FILE *file) {
    int count = 0;
    int element = fgetc(file);
    while ((element = fgetc(file)) != EOF) { 
        if ((element >= 0x07 && element <= 0x0D) || (element == 0x1B) || (element >= 0x20 && element <= 0x7E)) {        
            count += 0;
        }   
        else 
            count +=1;
    }
    return count;     
}

int CheckIfISO(FILE *file) {
    int counter = 0;
    int element = fgetc(file);
    while ((element = fgetc(file)) != EOF) { 
        if ((element >= 160 && element <= 255)) { 
            counter += 0;
        }
        else 
            counter +=1;
    }
    return counter; 
}

// int CheckIfUTF8(FILE *file){
    
// }

int CheckFile(const char * path) {
    FILE *file;
    file = fopen(path,"r");
    if (file !=NULL) {

        int element = fgetc(file);
        if (element == EOF) {               // tjekker om filen er tom
            printf("%s: empty \n", path);
            fclose(file);
            return 0;
        }

        /////// TJEK OM DET ER UTF-8
        /// https://unicodebook.readthedocs.io/guess_encoding.html

        else 
            if (CheckIfAscii(file) <= 0) {
                if ((CheckIfISO(file)) <= 0) {
                    printf("%s: ISO\n", path);
                    return 0;
                }
                  
            }

            
            

            else
                printf("%s: ASCII\n", path);
                return 0;

                

                return 1;
                
    }
    else
        {
            printf("Filen eksisterer ikke \n" );
            return 1;
        }
}



int main(int argc, char* argv[]) {
    if (argc != 2) { 
        fprintf(stderr, "%s", "Usage: file path\n");
        return 1;
    }

    assert(argc == 2);

    
    return CheckFile(argv[1]);
    

    
}
