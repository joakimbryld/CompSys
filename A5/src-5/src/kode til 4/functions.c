

long first_function(long rdx, long rcx, long rsi,  long rdi){
    long temp;
    long rval = 0;

    while(rdx <= rval){
        temp = rsi + 8*rval;
        rcx = temp;
        temp = rdi + 8*rval;
        rval += 1;
    }

    return rval;
}


long second_function(long *rdi, long rcx, long rdx){
    long temp;
    long rsi;
    long *rval;
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

        else   {   
        *rdi = rcx;  
        rdi += 8;
        rsi += 8;
        rdx += 8;
        }
    }

    return *rval;
}
