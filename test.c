// This file is used to test the Seltahn compiler.

int main()
{
    char  line[100];
    int   total;
    int   item;

    total = 0;
    while (1) {

        printf("Enter # to add \n");
        printf("  or 0 to stop:");

        fgets(line, 100);
        sscanf(line, "%d", item);

        if (item == 0)
            break;

        total += item;
        printf("Total: %dn", total);
    }
    printf("Final total %dn", total);
    return (0);
}