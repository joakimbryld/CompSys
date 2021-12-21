

int first_function(int rdx, int rcx, int rsi, int rdi){
    int temp;
    int rval = 0;

    while(rdx <= rval){
        temp = rsi + 8*rval;
        rcx = temp;
        temp = rdi + 8*rval;
        rval += 1;
    }

    return rval;
}


int second_function(int *rdi, int rcx, int rdx){
    int temp;
    int rsi;
    int *rval;
    temp = *rdi + rcx*8;

    if (temp == *rdi){
        return 0;
    }

    while(temp > *rdi){
        rval = &rdx;
        rsi = rcx;

        if (*rval > rsi){
            rdi = rval;
        }

        rdi += 8;
        rsi += 8;
        rdx += 8;

        rcx = *rdi;
    }

    return *rval;
}
