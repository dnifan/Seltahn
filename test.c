// This file is used to test the Seltahn compiler.

struct my_struct {
    int a, b, c;
    int bitfield : 4;
};

int main()
{
    char  line[100];
    int   total;
    int   item;

    for (total = 0; total != 0; total += item) {
        printf("Enter # to add \n");
        printf("  or 0 to stop:");

        fgets(line, 100);
        sscanf(line, "%d", item);

        printf("Total: %dn", total);
    }
    printf("Final total %dn", total);
    return (0);
}