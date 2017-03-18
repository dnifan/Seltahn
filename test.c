// This file is used to test the Seltahn compiler.

int max(int a, int b) {
    if (a >= b)
        return a;
    else if (a < b)
        return b;
}

int main()
{
    int m = max(100, 500);

}