#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <assert.h>

class Matrix {

private:
    unsigned _lines;
    unsigned _columns;
    float **_matrix;
    void allocate();
    void deallocate();
    void initialize();

public:
    Matrix(unsigned lines=0, unsigned columns=0);
    Matrix(const Matrix &M);
    ~Matrix();

    // Basics
    void setSize(unsigned lines, unsigned columns);
    unsigned lines() const { return _lines; }
    unsigned columns() const { return _columns; }
    float get(unsigned i, unsigned j) const;
    void set(unsigned i, unsigned j, float value);

    // Matrix functions
    Matrix transposed() const;

    // Matrix generation
    static Matrix identity(unsigned size);
    static Matrix diag(unsigned size, float diagValue);

    // Auxiliary functions
    void copyFrom(const Matrix &M);
    void print();

    // Operators
    void operator=(const Matrix &M);
    Matrix operator+(const Matrix &M) const;
    Matrix operator+(float k) const;
    Matrix operator-(const Matrix &M) const;
    Matrix operator-(float k) const;
    Matrix operator*(const Matrix &M) const;
    Matrix operator*(float k) const;
    void operator+=(const Matrix &M);
    void operator+=(float k);
    void operator-=(const Matrix &M);
    void operator-=(float k);
    void operator*=(float k);

};
#endif // MATRIX_H
