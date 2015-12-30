#include <stdio.h>
void main()
{
    int i, j;
    int a[6];
    int tmp = 0;
    printf("Please input 6 numbers:\n");
    for(i = 0; i < 6; i++)
    {
	scanf("%d", &a[i]);
    }

    //every time pick out a smallest number
    for(i = 0; i < 6; i++ )
	for(j = 0; j < 6-i-1; j++)
	{
	    if(a[j] < a[j+1])
	    {
 		tmp = a[j];
		a[j] = a[j+1];
		a[j+1] = tmp;
	    }
	}

    for(i = 0; i < 6; i++)
    {
	    printf("%d\n", a[i]);
    }
}
