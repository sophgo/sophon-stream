//
// Created by samuel on 19-3-13.
//
#include <iostream>
#include <algorithm>
#include "HungarianState.h"
#include <math.h>

#define EPSILON 0.000001
#define MAX_VALUE 100000000
#define MAX_FLOAT_VALUE 100000000

bool dequals(float a, float b) {
    //一定要#include <math.h>  labs 针对long整数   fabs针对浮点数，abs针对整数
    return fabs(a - b) < EPSILON;
}

template <typename T>
int np_argmax1d(std::vector<T> &C, int length) {
    // fixed int --> T
    T tmp_max = C[0];
    int arg_max = 0;

    for (int i = 0; i < length; i++) {
        if (C[i] > tmp_max) {
            tmp_max = C[i];
            arg_max = i;
        }
    }
    return arg_max;
}

template <typename T>
int np_argmax2d(std::vector<std::vector<T>> &C, int rows, int cols) {
    T tmp_max = C[0][0];
    int arg_max = 0;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (C[i][j] > tmp_max) {
                tmp_max = C[i][j];
                arg_max = i * cols + j;
            }
        }
    }
    return arg_max;
}


HungarianState::HungarianState(const std::vector<std::vector<float>> &cost_matrix) {
    // cost_matrix[0].size() 列数
    // cost_matrix.size()    行数
    std::size_t rows = cost_matrix.size();
    std::size_t cols = cost_matrix[0].size();
    transposed = (cols < rows);


    if (transposed) {
        C.resize(cols);
        for (int i = 0; i < cols; i++) {
            C[i].resize(rows);
        }
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                C[j][i] = cost_matrix[i][j];
            }
        }
    } else {
        C.assign(cost_matrix.begin(), cost_matrix.end());
    }

    rows = C.size();
    cols = C[0].size();

    row_uncovered.resize(rows, true);
    col_uncovered.resize(cols, true);

    path.resize(rows + cols);
    for (int i = 0; i < rows + cols ; i++) {
        path[i].resize(2, 0);
    }

    marked.resize(rows);
    for (int i = 0; i < rows; i++) {
        marked[i].resize(cols, 0);
    }


}

HungarianState::~HungarianState() {}

void HungarianState::_clear_covers() {
    // Clear all covered matrix cells
    for (int i = 0; i < row_uncovered.size(); i++) {
        row_uncovered[i] = true;
    }
    for (int i = 0; i < col_uncovered.size(); i++) {
        col_uncovered[i] = true;
    }
}

// int HungarianState::_find_prime_in_row(int row) {
//     /*
//     Find the first prime element in the specified row. Returns
//     the column index, or -1 if no starred element was found.
//     */
//     int col = -1;
//     // 2019.3.14 当vector传入的模板类型为bool时,
//     std::vector<bool> compares(marked[0].size(), false);

//     for (int i = 0; i < marked[0].size(); i++) {
//         if (marked[row][i] == 2) {
//             compares[i] = true;
//             if (col < 0) {
//                 col = i;
//             }
//         }
//     }

//     if (marked[row][col] != 2) {
//         col = -1;
//     }
//     return col;

// }

// 如果有bug, 高岱恒eat shit.
void *step1(HungarianState &state) {
    /*
     * 第一次执行step1的时候(当然,不同于step3到step6, step1在每次更新的时候只会执行一次),
     * 因为默认设置的col_uncovered和row_uncovered都为true, 所以step1的功能是:
     *
     * ① 将state.C的每一行都减去这一行的最小值, 以确保新的state.C的每一行
     *   都有1个或者多个为0的元素.
     * ② 并将state.C中等于0的元素的索引位置于state.marked标记为1. (初始化的时候state.marked的所有元素都为0).
     *
     * */
    // 1) 减去每行的最小值.
    int rows = state.row_uncovered.size();
    int cols = state.col_uncovered.size();

    for (int i = 0; i < rows; i++) {
        float min = *std::min_element(state.C[i].begin(), state.C[i].end());
        for (int j = 0; j < cols; j++) {
            state.C[i][j] -= min;
        }
    }


    // 2) 注意, 对1行中有多个0的state.C, 需要返回所有0的索引位置.
    //          float的值判断需要额外注意.
    //          C++ 没有and, 只能用&&.
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            // fixme :修正EPSILON的值. (已改正) 难道dequals还有问题?
            if (dequals(state.C[i][j], 0)) {
                if (state.col_uncovered[j] && state.row_uncovered[i]) {
                    state.marked[i][j] = 1;
                    state.col_uncovered[j] = false;
                    state.row_uncovered[i] = false;
                }
            }
        }
    }
    state._clear_covers();
    return (void *)step3;

//    for i, j in zip(*np.where(state.C == 0)):
//    if state.col_uncovered[j] and state.row_uncovered[i]:
//    state.marked[i, j] = 1
//    state.col_uncovered[j] = False
//    state.row_uncovered[i] = False


}

// 如果有bug, 高岱恒eat shit.
void *step3(HungarianState &state) {
    /*Cover each column containing a starred zero. If n columns are covered,
      the starred zeros describe a complete set of unique assignments.
      In this case, Go to DONE, otherwise, Go to Step 4.
      如果说step3能够覆盖n个columns(columns的总数为m), 那么说明n个detections和n个
      trackers可以1-1匹配上. 这种情况下, 结束优化, 否则的话, 进入step4继续执行linear
      assignment算法.

      所以step3的功能是:
     *
     * ① 创建一个名为marked的矩阵(bool型, shape与state.marked一样. 还是由二维vector组成.)
     *
     * ② 并根据marked的情况, 对marked的某列中有True的情况, 将未覆盖的列(col_uncovered)的对应列设置为False(双重否定表肯定).
     * */
    std::vector<std::vector<bool>> marked;
    int rows = state.marked.size();
    int cols = state.marked[0].size();
    int tmp = 0;


    marked.resize(rows);
    for (int i = 0; i < rows; i++) {
        marked[i].resize(cols, false);
    }

    // 一般来说, 判断一个变量的值和一个常量是否相等,
    // 用constant == variable的形式, 因为可能会误写
    // marked[i][j] = 1; 这样会永远返回True, 对于复杂工程
    // debug的时候比较费劲...
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (state.marked[i][j] == 1) {
                marked[i][j] = true;
                tmp += 1;
            }
        }
    }

    // 对state.col_uncovered[np.any(marked, axis=0)] = False
    // 的C++版本实现.
    // np.any(xx, axis=0), 对xx的每一列, 只要有为True的值, 最后就会返回True
    // >>> marked
    //array([[ True, False, False],
    //       [False, False, False]], dtype=bool)
    //>>> np.any(marked, axis=0)
    //array([ True, False, False], dtype=bool)
    // fixme: 忘记实现state.col_uncovered部分, 只实现了np.any(...)部分. (已修改.)
    for (int j = 0; j < cols; j++) {
        for (int i = 0; i < rows; i++) {
            if (marked[i][j] == true) {
                state.col_uncovered[j] = false;
            }
        }
    }

//   fixme: 正常的代码会进入30次step3, 而C++版本代码只会进入2次.
    if (tmp < state.C.size()) {
        return (void *)step4;
    } else {
        return nullptr;
    }

}

// 如果有bug, 高岱恒eat shit.
void *step4(HungarianState &state) {
    /*
    Find a noncovered zero and prime it. If there is no starred zero
    in the row containing this primed zero, Go to Step 5. Otherwise,
    cover this row and uncover the column containing the starred
    zero. Continue in this manner until there are no uncovered zeros
    left. Save the smallest uncovered value and Go to Step 6.
     *
     * Step4和Step5是重点. 2019.3.14先把计算逻辑写完, 明天再梳理它为什么要这么做.
     * */
    // We convert to int as numpy operations are faster on int.
    std::vector<std::vector<int>> C;
    unsigned int rows = state.row_uncovered.size();
    unsigned int cols = state.col_uncovered.size();
    C.resize(rows);
    for (int i = 0; i < rows; i++) {
        C[i].resize(cols, 0);
    }


    //    C = (state.C == 0).astype(np.int)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            // fixme: 可能这里有问题? 2019.3.15
            if (dequals(state.C[i][j], 0)) {
                C[i][j] = 1;
            }
        }
    }

    //    covered_C = C * state.row_uncovered[:, np.newaxis]
    //    保留C中row_uncovered为True的行的值, False对应的行值都为0.
    std::vector<std::vector<int>> covered_C;
    covered_C.assign(C.begin(), C.end());

    for (int i = 0; i < rows; i++) {
        if (state.row_uncovered[i] != true) {
            for (int j = 0; j < cols; j++) {
                covered_C[i][j] = 0;
            }
        }
    }
    //    covered_C *= state.col_uncovered.astype(dtype=np.int, copy=False)
    //    对covered_C, 保留col_uncovered中为True的列, 把col_uncovered为False的列置为0.
    std::vector<int> temp_cols(cols, 0);
    for (int j = 0; j < cols; j++) {
        if (state.col_uncovered[j]) {
            temp_cols[j] = 1;
        }
    }

    for (int j = 0; j < cols; j++) {
        if (temp_cols[j] != 1) {
            for (int i = 0; i < rows; i++) {
                covered_C[i][j] = 0;
            }
        }
    }

    while (1) {
        //1. Find an uncovered zero
        //        np.argmax(...)的用法.
        //        >>> a
        //        array([[0, 1, 2],
        //        [3, 4, 5]])
        //        >>> np.argmax(a)
        //        5
        //        >>> a = np.array([1, 2, 2])
        //        >>> np.argmax(a)
        //        1 返回第一个最大值的索引位置!
        // row, col = np.unravel_index(np.argmax(covered_C), (n, m))
        // 把np.argmax封装一下.
        int arg_max = 0;
        arg_max = np_argmax2d(covered_C, rows, cols);
        // np.unravel_index是把1维的索引映射回二维的位置.
        // fixme 写错了坐标.(修正)
        int row_coord = arg_max / cols;
        int col_coord = arg_max - (row_coord * cols);

        if (covered_C[row_coord][col_coord] == 0) {
            return (void *)step6;
        } else {
            state.marked[row_coord][col_coord] = 2;
            // Find the first starred element in the row.
            int star_col = 0;
            // fixme: 错误的改变了state.marked!!! 这里只是判断, 不改变state.marked本身.
            std::vector<int> tmp_restore(cols, 0);
            for (int j = 0; j < cols; j++) {
                tmp_restore[j] = (int)(state.marked[row_coord][j] == 1);
            }

            star_col = np_argmax1d(tmp_restore, cols);
            if (state.marked[row_coord][star_col] != 1) {
                state.Z0_r = row_coord;
                state.Z0_c = col_coord;

                return (void *)step5;
            } else {
                col_coord = star_col;
                state.row_uncovered[row_coord] = false;
                state.col_uncovered[col_coord] = true;

                //                covered_C[:, col] = C[:, col] * (
                //                        state.row_uncovered.astype(dtype=np.int, copy=False))
                // covered_C中第col列的值都*state.row_uncovered的值, 若对应的位置为False, 则covered_C中
                // 第col列对应的某行的值为0; 若对应位置为True, 不变.
                // fixed: 大bug!!!
                for (int i = 0; i < rows; i++) {
                    if (state.row_uncovered[i] != true) {
                        covered_C[i][col_coord] = 0;
                    } else {
                        covered_C[i][col_coord] = C[i][col_coord];
                    }
                }
                //                covered_C[row] = 0
                for (int j = 0; j < cols; j++) {
                    covered_C[row_coord][j] = 0;
                }

            }

        }


    }
}

// 如果有bug, 邢子龙eat shit.
void *step5(HungarianState &state) {
    /*
    Construct a series of alternating primed and starred zeros as follows.

    Let Z0 represent the uncovered primed zero found in Step 4.
    Let Z1 denote the starred zero in the column of Z0 (if any).
    Let Z2 denote the primed zero in the row of Z1 (there will always be one).
    Continue until the series terminates at a primed zero that has no starred
    zero in its column. Unstar each starred zero of the series, star each
    primed zero of the series, erase all primes and uncover every line in the
    matrix. Return to Step 3
    */

    unsigned int row_length = state.marked.size();
    unsigned int col_length = state.marked[0].size();

    int count = 0;
    //    path = state.path
    //    注意, path= state.path是浅拷贝.也就是说, 修改path也就是修改state.path本身.
    //    path[count, 0] = state.Z0_r
    //    path[count, 1] = state.Z0_c
    state.path[count][0] = state.Z0_r;
    state.path[count][1] = state.Z0_c;

    // fixme: while里面可能有问题? 2019.3.15
    //    while True:
    //# Find the first starred element in the col defined by
    //# the path.
    //        row = np.argmax(state.marked[:, path[count, 1]] == 1)
    //    if not state.marked[row, path[count, 1]] == 1:
    //# Could not find one
    //    break
    //    else:
    //    count += 1
    //    path[count, 0] = row
    //    path[count, 1] = path[count - 1, 1]
    //
    //# Find the first prime element in the row defined by the
    //# first path step
    //    col = np.argmax(state.marked[path[count, 0]] == 2)
    //    if state.marked[row, col] != 2:
    //    col = -1
    //    count += 1
    //    path[count, 0] = path[count - 1, 0]
    //    path[count, 1] = col

    while (1) {
        // Find the first starred element in the col defined by the path.
        //        row = np.argmax(state.marked[:, path[count, 1]] == 1)
        //    state.marked[:, x] == 1表示判断marked矩阵的第x列的所有值跟1的差别. 返回bool的数组.
        int row = 0;
        std::vector<int> tmp_row_count(row_length, 1);


        for (int i = 0; i < row_length; i++) {
            // fixme: 龙哥修改, 贼NB.
            unsigned int colIndex = 0;
            (state.path[count][1] == -1) ? (colIndex = col_length - 1) : (colIndex = state.path[count][1]);
            tmp_row_count[i] = state.marked[i][colIndex];

            if (tmp_row_count[i] == 1) {
                tmp_row_count[i] = 1;
            } else {
                tmp_row_count[i] = 0;
            }
        }
        row = np_argmax1d(tmp_row_count, row_length);

        //modify at 15:40
        // fixme: 计算逻辑没问题, 问题出在vector不会将-1视为有效的索引. 而Python的numpy
        //        会将-1视为是提取最后一个元素的索引. 故出现错误!
        unsigned int colIndex = 0;
        (state.path[count][1] == -1) ? (colIndex = col_length - 1) : (colIndex = state.path[count][1]);

        if (state.marked[row][colIndex] != 1) {
            // Could not find one.
            break;
        } else {
            count += 1;
            state.path[count][0] = row;
            state.path[count][1] = state.path[count - 1][1];
        }


        // Find the first prime element in the row defined by the
        // first path step
        //        col = np.argmax(state.marked[path[count, 0]] == 2)
        int col = 0;
        std::vector<int> tmp_col_count(col_length, 0);
        tmp_col_count.assign(state.marked[state.path[count][0]].begin(), state.marked[state.path[count][0]].end());


        for (int i = 0; i < col_length; i++) {
            if (tmp_col_count[i] == 2) {
                tmp_col_count[i] = 1;
            } else {
                tmp_col_count[i] = 0;
            }
        }
        col = np_argmax1d(tmp_col_count, col_length);


        if (state.marked[row][col] != 2) {
            col = -1;
        }
        count += 1;
        state.path[count][0] = state.path[count - 1][0];
        state.path[count][1] = col;
    }

    // Convert paths
    //    for i in range(count + 1):
    //    if state.marked[path[i, 0], path[i, 1]] == 1:
    //    state.marked[path[i, 0], path[i, 1]] = 0
    //    else:
    //    state.marked[path[i, 0], path[i, 1]] = 1
    // fixme: .
    for (int i = 0; i < count + 1; i++) {
        unsigned int colIndex = 0;
        (state.path[i][1] == -1) ? (colIndex = col_length - 1) : (colIndex = state.path[i][1]);

        if (state.marked[state.path[i][0]][colIndex] == 1) {
            state.marked[state.path[i][0]][colIndex] = 0;
        } else {
            state.marked[state.path[i][0]][colIndex] = 1;
        }
    }


    // Erase all prime markings
    state._clear_covers();
    //    state.marked[state.marked == 2] = 0
    // 2019.3.14 星期四
    for (int i = 0; i < row_length; i++) {
        for (int j = 0; j < col_length; j++) {
            if (state.marked[i][j] == 2) {
                state.marked[i][j] = 0;
            }
        }
    }

    return (void *)step3;

}

// 如果有bug, 高岱恒eat shit.
void *step6(HungarianState &state) {
    /*
    Add the value found in Step 4 to every element of each covered row,
    and subtract it from every element of each uncovered column.
    Return to Step 4 without altering any stars, primes, or covered lines.
     *
     * */
//    if np.any(state.row_uncovered) and np.any(state.col_uncovered):
    bool row_t = false;
    bool col_t = false;

    for (auto i : state.row_uncovered) {
        if (i == true) {
            row_t = true;
        }
    }

    for (auto i : state.col_uncovered) {
        if (i == true) {
            col_t = true;
        }
    }


    if (row_t && col_t) {
        //    minval = np.min(state.C[state.row_uncovered], axis=0)
        //    返回state.row_uncovered为True的state.C中每列最小的值.
        //    notice: 经过step1后, state.C的值都变成大于等于0的了.
        //            对于我们这个任务, state.C的值都小于1. 所以minval中的默认值很重要.
        unsigned int rows = state.row_uncovered.size();
        unsigned int cols = state.col_uncovered.size();
        // fixme: 最初minval的size定位rows了, 应该为cols.
        std::vector<float> minval(cols, MAX_FLOAT_VALUE);
        float minvals = MAX_FLOAT_VALUE;

        // fixed: 2019.3.15 应该先遍历行, 再遍历列.
        for (int i = 0; i < rows; i++) {
            if (state.row_uncovered[i]) {
                for (int j = 0; j < cols ; j++) {
                    minval[j] = std::min(minval[j], state.C[i][j]);
                }
            }
        }

        //    minvals = np.min(minval[state.col_uncovered])
        //    实现这个逻辑, 同样地, 相对简单. minvals是一个float型的值.
        for (int i = 0; i < cols; i++) {
            if (state.col_uncovered[i]) {
                minvals = std::min(minvals, minval[i]);
            }
        }


        //    state.C[np.logical_not(state.row_uncovered)] += minvals
        //    np.logical_not (逻辑非)
        //    对满足row_uncovered == False的行添加统一的最小值minvals.
        for (int i = 0; i < rows; i++) {
            if (state.row_uncovered[i] != true) {
                for (int j = 0; j < cols; j++) {
                    state.C[i][j] += minvals;
                }
            }
        }
        //    state.C[:, state.col_uncovered] -= minvals
        //    对满足col_uncovered == True的列, 统一减去最小值minvals.
        for (int j = 0; j < cols; j++) {
            if (state.col_uncovered[j]) {
                for (int i = 0; i < rows; i++) {
                    state.C[i][j] -= minvals;
                }
            }
        }
    }

    return (void *)step4;
}


std::vector<std::vector<int>> linear_assignment(std::vector<std::vector<float>> &cost_matrix) {
    /*
    The Hungarian algorithm.

    Calculate the Munkres solution to the classical assignment problem and
    return the indices for the lowest-cost pairings.

    Parameters
    ----------
    cost_matrix : 2D matrix

    The cost matrix. Does not have to be square.

    Returns
    -------
    indices : 2D array of indices

    The pairs of (row, col) indices in the original array giving
    the original ordering.
    */
    HungarianState state = HungarianState(cost_matrix);

    const unsigned int rows = state.C.size();
    const unsigned int cols = state.C[0].size();

    // fixme: (void*) 传递函数指针的逻辑?
    STEP_FUNC step = step1;
    // No need to bother with assignments if one of the dimensions
    // of the cost matrix is zero-length.
    if (cost_matrix.size() == 0 || cost_matrix[0].size() == 0) {
        step = nullptr;
    }

    while (step) {
        step = (STEP_FUNC)step(state);
    }

    // Look for the starred columns
    // 返回state.marked == 1位置处的所有索引.
    //    results = np.array(np.where(state.marked == 1)).T
    // 保存顺序先行后列.
    // results的形式:
    // array([[0, 0],
    //        [0, 2],
    //        [1, 1]])
    std::vector<std::vector<int>> results;
    unsigned int idx = 0;
    unsigned int count_idx = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (state.marked[i][j] == 1) {
                count_idx ++;
            }
        }
    }
    //fixme: 限定result的输出范围. 2019.3.15
    results.resize(count_idx);
    for (int i = 0; i < count_idx; i++) {
        results[i].resize(2);
    }


    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (state.marked[i][j] == 1) {
                results[idx] = {i, j};
                idx ++;
            }
        }
    }



    // 交换列(一共只有2列)
    if (state.transposed) {
        int tmp = 0;
        for (int i = 0; i < idx; i++) {
            tmp = results[i][1];
            results[i][1] = results[i][0];
            results[i][0] = tmp;
        }
    }

    std::sort(results.begin(), results.end());

    return results;
}

