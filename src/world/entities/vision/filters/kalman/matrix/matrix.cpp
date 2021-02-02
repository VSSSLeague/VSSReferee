#include "matrix.h"

Matrix::Matrix(unsigned lines, unsigned columns) {
    _lines = lines;
    _columns = columns;

    // Create matrix
    _matrix = NULL;
    allocate();
    initialize();
}

Matrix::Matrix(const Matrix &M) {
    _lines = M.lines();
    _columns = M.columns();

    // Create matrix by copy
    _matrix = NULL;
    allocate();
    copyFrom(M);
}

Matrix::~Matrix() {
    deallocate();
}

void Matrix::allocate() {
    if(_lines!=0 && _columns!=0) {
        _matrix = new float*[_lines];
        for(unsigned i=0; i<_lines; i++)
            _matrix[i] = new float[_columns];
    }
}

void Matrix::deallocate() {
    if(_matrix!=NULL) {
        for(unsigned i=0; i<_lines; i++)
            delete[] _matrix[i];
        delete[] _matrix;
        _matrix = NULL;
    }
}

void Matrix::initialize() {
    for(unsigned i=0; i<_lines; i++)
        for(unsigned j=0; j<_columns; j++)
            _matrix[i][j] = 0;
}

void Matrix::setSize(unsigned lines, unsigned columns) {
    deallocate();
    _lines = lines;
    _columns = columns;
    allocate();
    initialize();
}

float Matrix::get(unsigned i, unsigned j) const {
    assert(i<_lines && j<_columns);
    return _matrix[i][j];
}

void Matrix::set(unsigned i, unsigned j, float value) {
    assert(i<_lines && j<_columns);
    _matrix[i][j] = value;
}

void Matrix::copyFrom(const Matrix &M) {
    assert(_lines==M.lines() && _columns==M.columns());
    for(unsigned i=0; i<_lines; i++)
        for(unsigned j=0; j<_columns; j++)
            _matrix[i][j] = M.get(i, j);
}

void Matrix::print() {
    for(unsigned i=0; i<_lines; i++) {
        for(unsigned j=0; j<_columns; j++)
            std::cout << _matrix[i][j] << " ";
        std::cout << "\n";
    }
}

Matrix Matrix::identity(unsigned size) {
    return diag(size, 1);
}

Matrix Matrix::diag(unsigned size, float diagValue) {
    Matrix M(size, size);
    for(unsigned i=0; i<size; i++)
        M.set(i, i, diagValue);

    return M;
}

Matrix Matrix::transposed() const {
    Matrix temp(_columns, _lines);
    for(unsigned i=0; i<_lines; i++)
        for(unsigned j=0; j<_columns; j++)
            temp.set(j, i, _matrix[i][j]);

    return temp;
}

void Matrix::operator=(const Matrix &M) {
    deallocate();
    _lines = M._lines;
    _columns = M._columns;
    allocate();
    copyFrom(M);
}

Matrix Matrix::operator+(const Matrix &M) const {
    assert(_lines==M.lines() && _columns==M.columns());
    Matrix temp(_lines, _columns);
    for(unsigned i=0; i<_lines; i++)
        for(unsigned j=0; j<_columns; j++)
            temp.set(i, j, _matrix[i][j]+M.get(i,j));

    return temp;
}

Matrix Matrix::operator-(const Matrix &M) const {
    assert(_lines==M.lines() && _columns==M.columns());
    Matrix temp(_lines, _columns);
    for(unsigned i=0; i<_lines; i++)
        for(unsigned j=0; j<_columns; j++)
            temp.set(i, j, _matrix[i][j]-M.get(i,j));

    return temp;
}

Matrix Matrix::operator+(float k) const {
    Matrix temp(_lines, _columns);
    for(unsigned i=0; i<_lines; i++)
        for(unsigned j=0; j<_columns; j++)
            temp.set(i, j, _matrix[i][j]+k);

    return temp;
}

Matrix Matrix::operator-(float k) const {
    return (*this)+(-k);
}

Matrix Matrix::operator*(const Matrix &M) const {
    assert(_columns==M.lines());
    Matrix temp(_lines, M.columns());

    for(unsigned i=0; i<_lines; i++) {
        for(unsigned j=0; j<M.columns();j++) {
            temp.set(i, j, 0);
            for(unsigned k=0;k<M.lines();k++)
                temp.set(i, j, temp.get(i,j)+_matrix[i][k]*M.get(k,j));
        }
    }

    return temp;
}

Matrix Matrix::operator*(float k) const {
    Matrix temp(_lines, _columns);
    for(unsigned i=0; i<_lines; i++)
        for(unsigned j=0; j<_columns; j++)
            temp.set(i, j, _matrix[i][j]*k);

    return temp;
}

void Matrix::operator+=(const Matrix &M) {
    for(unsigned i=0; i<_lines; i++)
        for(unsigned j=0; j<_columns; j++)
            _matrix[i][j] += M.get(i,j);
}

void Matrix::operator+=(float k) {
    for(unsigned i=0; i<_lines; i++)
        for(unsigned j=0; j<_columns; j++)
            _matrix[i][j] += k;
}

void Matrix::operator-=(const Matrix &M) {
    for(unsigned i=0; i<_lines; i++)
        for(unsigned j=0; j<_columns; j++)
            _matrix[i][j] -= M.get(i,j);
}

void Matrix::operator-=(float k) {
    for(unsigned i=0; i<_lines; i++)
        for(unsigned j=0; j<_columns; j++)
            _matrix[i][j] -= k;
}

void Matrix::operator*=(float k) {
    for(unsigned i=0; i<_lines; i++)
        for(unsigned j=0; j<_columns; j++)
            _matrix[i][j] *= k;
}
