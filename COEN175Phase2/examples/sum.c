/* sum.c */

int printf();

int main(void)
{
    int i, n, the_sum;

    i = 0;
    the_sum = 0;
    n = 10;

    while (i <= n) {
	the_sum = the_sum + i;
	i = i + 1;
    }

    printf("%d %d\n", the_sum, n * (n + 1) / 2);
}
