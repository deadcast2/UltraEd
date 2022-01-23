#include <nusys.h>
#include <malloc.h>
#include "vector.h"

#define INITIAL_CAPACITY 8
#define min(x,y) (((x)<(y))?(x):(y))

struct _Vector
{
    ValueType *array;
    int size;
    int capacity;
};

Vector CVector_Create() 
{
    Vector v = (Vector)malloc(sizeof(struct _Vector)); 
    
    if (v == NULL) return NULL;
    
    v->size = 0; 
    v->capacity = INITIAL_CAPACITY; 
    v->array = (ValueType *)malloc(sizeof(ValueType) * v->capacity); 
    
    if (v->array == NULL)
    {
        CVector_Destroy(v);
        return NULL;
    }
    
    return v;
}

void CVector_Destroy(Vector v) 
{
    if (v == NULL) return;

    free(v->array); 
    free(v);
}

void CVector_DoubleCapacity(Vector v) 
{
    if (v == NULL) return;

    const int newCapacity = 2 * v->capacity; 
    
    ValueType *newArray = (ValueType *)malloc(sizeof(ValueType) * newCapacity); 
    
    if (newArray == NULL) return;
    
    for (int i = 0; i < v->size; i++)
    {
        newArray[i] = v->array[i];
    }
    
    free(v->array); 
    v->array = newArray; 
    v->capacity = newCapacity;
}

void CVector_HalfCapacity(Vector v) 
{
    if (v == NULL) return;

    if (v->capacity <= INITIAL_CAPACITY) return;
    
    const int newCapacity = v->capacity / 2; 
    
    ValueType *newArray = (ValueType *)malloc(sizeof(ValueType) * newCapacity); 
    
    if (newArray == NULL) return;
    
    for (int i = 0; i < min(v->size, newCapacity); i++)
    {
        newArray[i] = v->array[i];
    }
    
    free(v->array); 
    v->array = newArray; 
    v->capacity = newCapacity; 
    v->size = min(v->size, newCapacity);
}

void CVector_Add(Vector v, ValueType value)
{
    if (v == NULL) return;

    if (v->size >= v->capacity)
    {
        CVector_DoubleCapacity(v);
    }
    
    v->array[v->size++] = value;
}

ValueType CVector_Get(Vector v, int i) 
{
    if (v == NULL) return NULL;

    if (i < 0 || i >= v->size) return NULL;
    
    return v->array[i];
}

void CVector_Put(Vector v, int i, ValueType value) 
{
    if (v == NULL) return;

    if (i < 0 || i >= v->size) return;
    
    v->array[i] = value;
}

void CVector_AddAt(Vector v, int i, ValueType value) 
{
    if (v == NULL) return;

    if (i < 0 || i >= v->size) return;
    
    if (v->size >= v->capacity)
    {
        CVector_DoubleCapacity(v);
    }
    
    for (int j = v->size; j > i; j--)
    {
        v->array[j] = v->array[j - 1];
    }
    
    v->array[i] = value; 
    v->size++;
}

ValueType CVector_RemoveAt(Vector v, int i) 
{
    if (v == NULL) return NULL;

    if (i < 0 || i >= v->size) return NULL;
    
    ValueType ret = v->array[i]; 
    
    for (int j = i + 1; j < v->size; j++)
    {
        v->array[j - 1] = v->array[j];
    }
    
    v->size--; 
    
    if (4 * v->size < v->capacity)
    {
        CVector_HalfCapacity(v);
    }
    
    return ret;
}

int CVector_IsEmpty(Vector v) 
{ 
    if (v == NULL) return 1;

    return v->size == 0;
}

int CVector_Size(Vector v) 
{
    if (v == NULL) return 0;

    return v->size;
}

void CVector_Clear(Vector v) 
{
    if (v == NULL) return;

    v->size = 0; 
    
    while (v->capacity > INITIAL_CAPACITY)
    {
        CVector_HalfCapacity(v);
    }
}
