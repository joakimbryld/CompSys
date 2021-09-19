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

int CheckIfUTF8(FILE *file){
    int counter = 0;
    char element = fgetc(file);
    while ((element = fgetc(file)) != EOF) { 
      if (element >> 7 == 0) {
        continue;

      }
      else if (element >> 5 == 6) {// 2 bytes
        unsigned char nextElement = fgetc(file);
        if (nextElement >> 6 == 2) {
          continue;
        }
        else 
          counter += 1;
          return 1;
      }

      else if (element >> 4 == 14) {

        unsigned char nextElement = fgetc(file);
        if (nextElement >> 6 == 2) {
          

          unsigned char nextElement = fgetc(file);
          if (nextElement >> 6 == 2) {
              continue;

          }
          else 
            counter += 1;
            return 1;

        }
        else
          counter += 1;
          return 1;
      }


      else if (element >> 3 == 30) {

        unsigned char nextElement = fgetc(file);
        if (nextElement >> 6 == 2) {
          if (nextElement >> 6 == 2) {
            if (nextElement >> 6 == 2) {
            continue;
            }
            else 
              counter += 1;
              return 1;
          }
          else 
            counter += 1;
            return 1;

        }

        else 
          counter += 1;
          return 1;

      }

      }
    return counter;
}

int CheckFile(const char * path) {
    FILE *file;
    file = fopen(path,"r");
    if (file !=NULL) {

        int element = fgetc(file);
        if (element == EOF) {               // tjekker om filen er tom
            printf("%s: empty\n", path);
            fclose(file);
            return 0;
        }

      
        else if (CheckIfAscii(file) <= 0) {
          printf("%s: ASCII text\n", path);
          return 0;
        }
        else if (CheckIfUTF8(file) <= 0) {
          printf("%s: UTF-8 Unicode text\n", path);
          return 0;
        }
        
        else if ((CheckIfISO(file)) <= 0) {
            printf("%s: ISO-8859 text\n", path);
            return 0;
        }

        


        else
          printf("%s: data\n", path);
          return 0;
                
    }
    else
        
      printf("Filen eksisterer ikke\n" );
      return 1;
        
  
}



int main(int argc, char* argv[]) {
    if (argc != 2) { 
        fprintf(stderr, "%s", "Usage: file path\n");
        return 1;
    }

    assert(argc == 2);

    
    return CheckFile(argv[1]);
    
}

