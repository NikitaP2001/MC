typedef unsigned long u32;

void foo()
{	
	typedef int u32;
	typedef unsigned char u8;
	
	u32 number;
	u8 c;
	
	{
		u8 c1;
	}
}

void bar()
{	
	u8 c2;
}

int main() {
    char str[100] = "Hello, World!";
    int numbers[10];
	
    for (int i = 0; i < 10; i++) {
        numbers[i] = i * i;
    }

    for (int i = 0; i < 10; i++) {
        printf("Square of %d is %d\n", i, numbers[i]);
    }

    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    printf("Length of the string: %d\n", len);

    int a = 5, b = 10;
    int *ptrA = &a, *ptrB = &b;
    printf("Before swap: a = %d, b = %d\n", a, b);
    int temp = *ptrA;
    *ptrA = *ptrB;
    *ptrB = temp;
    printf("After swap: a = %d, b = %d\n", a, b);

    int matrix[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    /* print matrix content */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }

    char input[100];
    printf("Enter a string: ");
    fgets(input, 100, stdin);

    int i = 0;
    while (input[i] != '\0') {
        if (input[i] >= 'a' && input[i] <= 'z') {
            input[i] -= 32;
        }
        i++;
    }
    printf("Uppercase string: %s\n", input);

    int prime = 1;
    for (int i = 2; i <= a / 2; i++) {
        if (a % i == 0) {
            prime = 0;
            break;
        }
    }
    if (prime) {
        printf("%d is a prime number\n", a);
    } else {
        printf("%d is not a prime number\n", a);
    }

    struct Point {
        int x;
        int y;
    };

    struct Point p1 = {3, 4};
    printf("Point p1: (%d, %d)\n", p1.x, p1.y);

    struct Point p2;
    p2.x = 5;
    p2.y = 6;
    printf("Point p2: (%d, %d)\n", p2.x, p2.y);

    return 0;
}