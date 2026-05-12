// this is a dumb example just to test the ccompiler
// ...

// ... // frfr // r

float dummy(){
    
    return 005.010030f;
}

char global;

int main(){

    int a = 16;
    
    int b = a + 1;

    /*
        the compiler can't even compile this part correctly yet
        it does not have operation precedence yet
    */
    b = a + 1 / b * a;

    a = global;

    return b;
}
