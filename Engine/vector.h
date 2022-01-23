/*
Dynamic Array in C
by Dimitrios Michail
https://d-michail.github.io/assets/teaching/data-structures/013_VectorImplementation.en.pdf
*/

#ifndef _VECTOR_H
#define _VECTOR_H

typedef struct _Actor Actor;
typedef struct _Vector *Vector;
typedef Actor *ValueType;

Vector CVector_Create();
void CVector_Destroy(Vector);
void CVector_DoubleCapacity(Vector v);
void CVector_HalfCapacity(Vector v);
ValueType CVector_Get(Vector, int);
void CVector_Put(Vector, int, ValueType);
void CVector_Add(Vector, ValueType);
void CVector_AddAt(Vector, int, ValueType);
ValueType CVector_RemoveAt(Vector, int);
int CVector_IsEmpty(Vector);
int CVector_Size(Vector);
void CVector_Clear(Vector);

#endif
