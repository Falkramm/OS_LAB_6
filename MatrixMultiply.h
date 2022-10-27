//
// Created by user on 12/10/2022.
//

#ifndef LAB4_MATRIXMULTIPLY_H
#define LAB4_MATRIXMULTIPLY_H

#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <thread>
#include "BufferedChannel.h"

class MatrixMultiply {
private:
    template<class ValueType=int>
    using Matrix = std::vector<std::vector<ValueType> >;
    Matrix<int> A, B;
    Matrix<Matrix<int> > splitA, splitB;
    int N = 1;
    int splitSize = 0;
public:
    MatrixMultiply(int n) {
        N = n;//TODO
        initMatrix(A);
        initMatrix(B);
        //printMatrix(A);
        //printMatrix(B);
        Matrix<int> C = multiplyAB(A, B);
        //printMatrix(C);
        for (int i = 1; i <= N; ++i)
            printTimeData(test(i));
    }

    void printTimeData(clock_t mainTime) const {
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "Size policy           " << N << "     Split policy        " << splitSize << '\n';
        std::cout << "Main time in ms       " << mainTime << '\n';
    }

    clock_t test(int splitsize) {
        clock_t mainTime = -clock();
        splitSize = splitsize;
        splitA = splitMatrix(A);
        splitB = splitMatrix(B);
        Matrix<Matrix<int> > splitC = multiplySplitAB(splitA, splitB);
        //printSplitMatrix(splitC);
        mainTime += clock();
        return mainTime;
    }

    static void printMatrix(Matrix<int> &marix) {
        std::cout << "----------------------------------\n";
        for (auto &i: marix) {
            for (int j: i)
                std::cout << j << ' ';
            std::cout << '\n';
        }
        std::cout << "----------------------------------\n";
    }

    static void printSplitMatrix(Matrix<Matrix<int> > &splitMatrix) {
        std::cout << "----------------------------------\n";
        for (auto &i: splitMatrix) {

            for (auto &j: i) {
                std::cout << "----------------------------------\n";
                for (auto &x: j) {
                    for (int y: x)
                        std::cout << y << ' ';
                    std::cout << '\n';
                }
                std::cout << "----------------------------------\n";
            }

        }
    }

    Matrix<Matrix<int> > splitMatrix(Matrix<int> &splitingMatrix) const {
        Matrix<Matrix<int> > answer;
        answer.resize((int(splitingMatrix.size()) + splitSize - 1) / splitSize,
                      std::vector<Matrix<int> >((splitingMatrix.front().size() + splitSize - 1) / splitSize));
        for (size_t i = 0; i < splitingMatrix.size(); ++i)
            for (size_t j = 0; j < splitingMatrix[i].size(); ++j) {
                if (j % splitSize == 0)
                    answer[i / splitSize][j / splitSize].emplace_back();
                answer[i / splitSize][j / splitSize][i % splitSize].push_back(splitingMatrix[i][j]);
            }
        return answer;

    }

    void initMatrix(Matrix<int> &initialMatrix) const {
        initialMatrix.resize(N, std::vector<int>(N));
        for (size_t i = 0; i < N; ++i)
            for (size_t j = 0; j < N; ++j)
                initialMatrix[i][j] = std::rand() % 10;
    }

    static Matrix<int> multiplyAB(Matrix<int> A, Matrix<int> B) {
        Matrix<int> C;
        if (A.empty() || B.empty()) {
            if ((A.size() | B.size()) == 0)
                return C;
            throw std::invalid_argument("Slavsia Zubovich (Gori v Adu)");
        }
        if (A.front().size() != B.size())
            throw std::invalid_argument("Shto vy mne dali, liudi?");
        C.resize(A.size(), std::vector<int>(B.front().size()));
        for (size_t i = 0; i < A.size(); ++i)
            for (size_t j = 0; j < B.front().size(); ++j)
                for (size_t k = 0; k < B.size(); ++k)
                    C[i][j] += A[i][k] * B[k][j];
        return C;
    }

    Matrix<Matrix<int> > multiplySplitAB(Matrix<Matrix<int> > A, Matrix<Matrix<int> > B) {
        Matrix<Matrix<int> > C;
        if (A.empty() || B.empty()) {
            if ((A.size() | B.size()) == 0)
                return C;
            throw std::invalid_argument("Slavsia Zubovich (Gori v Adu)");
        }
        if (A.front().size() != B.size())
            throw std::invalid_argument("Shto vy mne dali, liudi?");
        C.resize(A.size(), std::vector<Matrix<int> >(B.front().size()));
        for (size_t i = 0; i < A.size(); ++i)
            for (size_t j = 0; j < B.front().size(); ++j) {
                std::vector<std::thread> thArr(B.size());
                BufferedChannel<std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t> > > bufferedChannel(
                        B.size());
                for (size_t k = 0; k < B.size(); ++k) {
                    bufferedChannel.Send({{i, k},
                                          {k, j}});

                }
                for (size_t k = 0; k < B.size(); ++k) {
                    Matrix<int> block;
                    std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t> > ABIndex = bufferedChannel.Recv().first;
                    thArr[k] = std::thread([this](Matrix<int> X, Matrix<int> Y, Matrix<int> &Z) {
                                               Z = MatrixMultiply::multiplyAB(X, Y);
                                           }, A[ABIndex.first.first][ABIndex.first.second], B[ABIndex.second.first][ABIndex.second.second],
                                           std::ref(block));
                    thArr[k].join();
                    if (C[ABIndex.first.first][ABIndex.second.second].empty())
                        C[ABIndex.first.first][ABIndex.second.second] = block;
                    else
                        C[ABIndex.first.first][ABIndex.second.second] = sumAB(block,
                                                                              C[ABIndex.first.first][ABIndex.second.second]);
                }
            }
        return C;
    }

    static Matrix<int> sumAB(Matrix<int> A, Matrix<int> B) {

        if (A.size() != B.size() || A.front().size() != B.front().size()) {

            throw std::invalid_argument(std::to_string(A.size()) + std::to_string(B.size()));
        }
        Matrix<int> C(A.size(), std::vector<int>(A.front().size()));
        for (size_t i = 0; i < C.size(); ++i)
            for (size_t j = 0; j < C[i].size(); ++j)
                C[i][j] = A[i][j] + B[i][j];
        return C;

    }
};


#endif //LAB4_MATRIXMULTIPLY_H
