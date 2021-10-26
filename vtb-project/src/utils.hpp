#pragma once 

#include <Eigen/Dense>

#include <math.h>
#include <algorithm>
#include <random>

using namespace Eigen;

namespace Utils {
    // Declare a class template
    template <bool is_integral, typename T> struct uniform_distribution_selector;
    // Specialize for true
    template <typename T> struct uniform_distribution_selector<true, T>
    {
    using type = typename std::uniform_int_distribution<T>;
    };
    // Specialize for false
    template <typename T> struct uniform_distribution_selector<false, T>
    {
    using type = typename std::uniform_real_distribution<T>;
    };

    //random uniform float or int distribution with range
    template<typename T>
    inline T random(T range_from = 0, T range_to = 1) {
        using uniform_distribution_type = typename uniform_distribution_selector<std::is_integral<T>::value, T>::type;
        std::random_device                  rand_dev;
        std::mt19937                        generator(rand_dev());
        uniform_distribution_type    distr(range_from, range_to);
        return distr(generator);
    }

    //random uniform float or int distribution with range. Supporting iteration
    template<class Iter, typename T>
    inline void random(Iter start, Iter end, T range_from = 0, T range_to = 1) {
        using uniform_distribution_type = typename uniform_distribution_selector<std::is_integral<T>::value, T>::type;
        std::random_device                  rand_dev;
        std::mt19937                        generator(rand_dev());
        uniform_distribution_type    distr(range_from, range_to);
        std::generate(start, end, [&] () { return distr(generator); });
    }

    //get matrix with random values
    template<typename T>
    inline MatrixX<T> getRandomMatrix(int nrow, int ncol, T start, T end) {
        srand((unsigned int) time(0));
        MatrixXd m = (MatrixXd::Random(nrow,ncol)+MatrixXd::Ones(nrow,ncol))*(end-start)/2 
            + MatrixXd::Constant(nrow, ncol, start);  
        return m.cast<T>();
    }

    //sorting row vector
    template<typename T>
    inline void rowVectorSort(RowVectorX<T>& vec, std::function<bool(T,T)> comp) {
        std::sort(vec.begin(), vec.end(), comp);
    }

    //right division(each element of martix divide is devider to element) value to row vector
    template<class T>
    inline RowVectorX<T> rdivide(T left_value, RowVectorX<T> right_matrix) {
        RowVectorX<T> left_matrix = RowVectorX<T>::Constant(right_matrix.cols(), left_value);
        return left_matrix.cwiseQuotient(right_matrix);
    }

    //right division(each element of martix divide is devider to element) value to column vector
    template<class T>
    inline VectorX<T> rdivide(T left_value, VectorX<T> right_matrix) {
        VectorX<T> left_matrix = VectorX<T>::Constant(right_matrix.rows(), left_value);
        return left_matrix.cwiseQuotient(right_matrix);
    }

    //right division(each element of martix divide is devider to element) value to matrix
    template<class T>
    inline MatrixX<T> rdivide(T left_value, MatrixX<T> right_matrix) {
        MatrixX<T> left_matrix = MatrixX<T>::Constant(right_matrix.rows(), right_matrix.cols(), left_value);
        return left_matrix.cwiseQuotient(right_matrix);
    }

    //row vector ceil
    template<class T>
    inline RowVectorX<T> ceil(RowVectorX<T> matrix) {
        return ceil(matrix.array());
    }

    //column vector ceil
    template<class T>
    inline VectorX<T> ceil(VectorX<T> matrix) {
        return ceil(matrix.array());
    }

    //matrix ceiling
    template<class T>
    inline MatrixX<T> ceil(MatrixX<T> matrix) {
        return ceil(matrix.array());
    }

    //row vector floor
    template<class T>
    inline RowVectorX<T> floor(RowVectorX<T> matrix) {
        return floor(matrix.array());
    }

    //column vector floor
    template<class T>
    inline VectorX<T> floor(VectorX<T> matrix) {
        return floor(matrix.array());
    }

    //matrix flooring
    template<class T>
    inline MatrixX<T> floor(MatrixX<T> matrix) {
        return floor(matrix.array());
    }

    //row vector round
    template<class T>
    inline RowVectorX<T> round(RowVectorX<T> matrix) {
        return round(matrix.array());
    }

    //column vector round
    template<class T>
    inline VectorX<T> round(VectorX<T> matrix) {
        return round(matrix.array());
    }

    //matrix rounding
    template<class T>
    inline MatrixX<T> round(MatrixX<T> matrix) {
        return round(matrix.array());
    }

    //find indexes in column vector with op condition
    template<class T, class UnaryOperator>
    std::vector<int> find(VectorX<T> matrix, UnaryOperator op) {
        std::vector<int> idxs;
        for(int i=0; i<matrix.size(); ++i) {
            T element = matrix(i);
            if(op(element)) {
                idxs.push_back(i);
            }
        }
        return idxs;
    }

    template<class T, class UnaryOperator>
    std::vector<int> find(RowVectorX<T> matrix, UnaryOperator op) {
        std::vector<int> idxs;
        for(int i=0; i<matrix.size(); ++i) {
            T element = matrix(i);
            if(op(element)) {
                idxs.push_back(i);
            }
        }
        return idxs;
    }

    template <typename T, class UnaryOperator>
    int findFirst(VectorX<T> vec, UnaryOperator op) {
        int index = -1;
        for(int i = 0; i < vec.size(); ++i) {
            T element = vec(i);
            if(op(element)) {
                index = i;
                break;
            }
        }
        return index; 
    }

    template <typename T, class UnaryOperator>
    int findFirst(RowVectorX<T> vec, UnaryOperator op) {
        int index = -1;
        for(int i = 0; i < vec.size(); ++i) {
            T element = vec(i);
            if(op(element)) {
                index = i;
                break;
            }
        }
        return index; 
    }

    template<class T = int>
    RowVectorX<T> matlab_colon(const T low, const T high, const T step = 1) {
        const int size = ((high-low)/step)+1;
        return RowVectorX<T>::LinSpaced(size, low, low+step*(size-1));//low:step:hi
    }

    // auxiliary procedures
    template<typename T>
    inline void remove_row(Eigen::MatrixX<T> &matrix, size_t row_to_remove) {
        size_t rows = matrix.rows() - 1;
        size_t cols = matrix.cols();

        if (row_to_remove < rows)
        {
            matrix.block(row_to_remove, 0, rows - row_to_remove, cols) = matrix.block(row_to_remove + 1, 0, rows - row_to_remove, cols);
        }

        matrix.conservativeResize(rows, cols);
    }

    template<typename T>
    inline void remove_rows(Eigen::MatrixX<T> &matrix, std::vector<int> indexes, int additional_offset = 0) {
        int offset = 0;
        for(auto i : indexes) {
            Utils::remove_row(matrix, i+additional_offset-offset);
            offset++;
        }
    }

    template<typename T>
    inline void remove_column(Eigen::MatrixX<T> &matrix, size_t col_to_remove) {
        size_t rows = matrix.rows();
        size_t cols = matrix.cols() - 1;

        if (col_to_remove < cols)
        {
            matrix.block(0, col_to_remove, rows, cols - col_to_remove) = matrix.block(0, col_to_remove + 1, rows, cols - col_to_remove);
        }

        matrix.conservativeResize(rows, cols);
    }

    template<typename T>
    inline void remove_columns(Eigen::MatrixX<T> &matrix, std::vector<int> indexes, int additional_offset = 0) {
        int offset = 0;
        for(auto i : indexes) {
            Utils::remove_column<T>(matrix, i+additional_offset-offset);
            offset++;
        }
    }

    template<typename T>
    inline void remove_element(Eigen::VectorX<T> &vec, size_t elem_to_remove) {
        size_t new_size = vec.size() - 1;

        if (elem_to_remove < new_size)
        {
            vec.segment(elem_to_remove, new_size - elem_to_remove) = vec.segment(elem_to_remove + 1, new_size - elem_to_remove);
        }

        vec.conservativeResize(new_size);
    }

        template<typename T>
    inline void remove_element(Eigen::RowVectorX<T> &vec, size_t elem_to_remove) {
        size_t new_size = vec.size() - 1;

        if (elem_to_remove < new_size)
        {
            vec.segment(elem_to_remove, new_size - elem_to_remove) = vec.segment(elem_to_remove + 1, new_size - elem_to_remove);
        }

        vec.conservativeResize(new_size);
    }

    template<typename T>
    inline void remove_elements_in_vector(Eigen::VectorX<T> &vec, std::vector<int> indexes) {
        int offset = 0;
        for(auto i : indexes) {
            Utils::remove_element<T>(vec, i-offset);
            offset++;
        }
    }

    // returns the float vector of size `size` filled with values `val`
    inline Eigen::VectorXf full_of(size_t size, float val) {
        Eigen::VectorXf res;
        res.resize(size);

        for (size_t i = 0; i < size; ++i)
        {
            res[i] = val;
        }
        return res;
    }

    // check if matrix contains no negative values
    inline bool all_non_negative(Eigen::MatrixXf const &v) {
        for (float x : v.reshaped())
        {
            if (x < 0)
            {
                return false;
            }
        }
        return true;
    }

    // find position of minimal element in float vector
    inline size_t argmin(Eigen::VectorXf const &v) {
        float min = v[0];
        size_t arg = 0;

        for (long int i = 1; i < v.size(); ++i)
        {
            if (v[i] < min)
            {
                min = v[i];
                arg = i;
            }
        }
        return arg;
    }

    template<typename T>
    std::vector<T> intersection(std::vector<T> v1, std::vector<T> v2, std::vector<int>* ind1 = nullptr, std::vector<int>* ind2 = nullptr){
        std::vector<T> v3;
        
        std::vector<T> tmp1 = v1;
        std::vector<T> tmp2 = v2;
        
        std::sort(v1.begin(), v1.end());
        std::sort(v2.begin(), v2.end());

        std::set_intersection(v1.begin(),v1.end(),
                            v2.begin(),v2.end(),
                            back_inserter(v3));
        if(ind1 != nullptr) {
            for(auto el : v3) {
                auto it1 = std::find(tmp1.begin(), tmp1.end(), el);;
                ind1->push_back(std::distance(tmp1.begin(), it1));
            }
        }
        if(ind2 != nullptr) {
            for(auto el : v3) {
                auto it2 = std::find(tmp2.begin(), tmp2.end(), el);
                ind2->push_back(std::distance(tmp2.begin(), it2));
            }
        }
        return v3;
    }

    template<typename T> 
    std::vector<T> get_std_vector(VectorX<T>& matrix) {
        std::vector<T> vec(matrix.size());
        VectorX<T>::Map(&vec[0], matrix.size()) = matrix;
        return vec;
    }

    template<typename T> 
    std::vector<T> get_std_vector(RowVectorX<T>& matrix) {
        std::vector<T> vec(matrix.size());
        RowVectorX<T>::Map(&vec[0], matrix.size()) = matrix;
        return vec;
    }
}