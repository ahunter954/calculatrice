#include <stdio.h>
#include <stdlib.h>
#include "math.h"

float addition(float a, float b)
{
    return a + b; 
}

float soustraction(float a, float b)
{
    return a - b; 
}

float multiplication(float a, float b)
{
    return a * b; 
}

float division(float a, float b)
{
    if (b==0)
    {
        printf("Erreur: division par 0\n");
        exit(1);
    }
    
    return a / b; 
}