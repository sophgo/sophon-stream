#include <iostream>
#include <algorithm>
#include "HungarianState.h"
#include <cmath>

#define EPSILON (1e-6)
#define MAX_FLOAT_VALUE 100000000

bool equals(float a, float b)
{
    return fabs(a-b) < EPSILON;
}

template<typename T>
int argmax_1d(std::vector<T> & data, int len)
{
    T cur_max = data[0];
    int max_id = 0;
    for(int i = 0;i<len;++i)
    {
        if(data[i] > cur_max)
        {
            cur_max = data[i];
            max_id = i;
        }
    }
    return max_id;
}

template<typename T>
int argmax_2d(std::vector<std::vector<T>> & data, int rows, int cols)
{
    T cur_max = data[0][0];
    int max_id = 0;
    for(int i = 0;i< rows;++i)
    {
        for(int j = 0;j<cols;++j)
        {
            if(data[i][j] > cur_max)
            {
                cur_max = data[i][j];
                max_id = i * cols + j;
            }
        }
    }
    return max_id;
}

HungarianState::HungarianState(const std::vector<std::vector<float>> & cost)
{
    std::size_t rows = cost.size();
    std::size_t cols = cost[0].size();
    transposed = (cols < rows);

    if(transposed)
    {
        C.resize(cols, std::vector<float>(rows));
        for(int i = 0;i<rows;++i)
            for(int j = 0;j<cols;++j)
                C[j][i] = cost[i][j];
    }
    else
        C.assign(cost.begin(), cost.end());

    rows = C.size();
    cols = C[0].size();

    row_uncovered.resize(rows, true);
    col_uncovered.resize(cols, true);

    path.resize(rows + cols, std::vector<int>(2, 0));
    marked.resize(rows, std::vector<int>(cols, 0));
}

HungarianState::~HungarianState() {}

void HungarianState::clear_covers() {
    for (int i = 0; i < row_uncovered.size(); i++) {
        row_uncovered[i] = true;
    }
    for (int i = 0; i < col_uncovered.size(); i++) {
        col_uncovered[i] = true;
    }
}

// int HungarianState::find_prime_in_row(int row)
// {
//     int col = -1;
//     std::vector<bool> compares(marked[0].size(), false);
//     for(int i = 0;i < marked[0].size();++i)
//     {
//         if(marked[row][i] == 2)
//         {
//             compares[i] = true;
//             if(col < 0)
//                 col = i;
//         }
//     }
//     if(marked[row][col] != 2)
//         col = -1;
//     return col;
// }

void * step1(HungarianState & state)
{
    int rows = state.row_uncovered.size();
    int cols = state.col_uncovered.size();

    for(int i = 0;i < rows;++i)
    {
        float min = *min_element(state.C[i].begin(), state.C[i].end());
        for(int j = 0;j<cols;++j)
        {
            state.C[i][j] -= min;
        }
    }
    for(int i = 0;i<rows;++i)
    {
        for(int j = 0;j<cols;++j)
        {
            if(equals(state.C[i][j], 0) && state.col_uncovered[j] && state.row_uncovered[i])
            {
                state.marked[i][j] = 1;
                state.col_uncovered[j] = false;
                state.row_uncovered[i] = false;                
            }
        }
    }
    state.clear_covers();
    return (void*) step2;
}

void * step2(HungarianState & state)
{
    int rows = state.marked.size();
    int cols = state.marked[0].size();
    std::vector<std::vector<bool>> marked(rows, std::vector<bool>(cols, false));
    int temp = 0;
    for(int i = 0;i < rows;++i)
    {
        for(int j = 0;j<cols;++j)
        {
            if(state.marked[i][j] == 1)
            {
                marked[i][j] == true;
                temp += 1;
            }
        }
    }
    for(int j = 0;j<cols;++j)
    {
        for(int i = 0;i < rows;++i)
        {
            if(marked[i][j])
                state.col_uncovered[j] = false;
        }
    }
    if(temp < rows)
        return (void *)step3;
    return nullptr;
}

void * step3(HungarianState & state)
{
    int rows = state.row_uncovered.size();
    int cols = state.col_uncovered.size();
    std::vector<std::vector<int>> C(rows, std::vector<int>(cols, 0));

    for(int i = 0;i<rows;++i)
    {
        for(int j = 0;j<cols;++j)
        {
            if(equals(state.C[i][j], 0))
                C[i][j] = 1;
        }
    }
    std::vector<std::vector<int>> covered_C;
    covered_C.assign(C.begin(), C.end());

    for(int i = 0;i<rows;++i)
    {
        if(!state.row_uncovered[i])
        {
            for(int j = 0;j<cols;++j)
                covered_C[i][j] = 0;
        }
    }
    for(int j = 0;j<cols;++j)
    {
        if(!state.col_uncovered[j])
        {
            for(int i = 0;i<rows;++i)
                covered_C[i][j] = 0;
        }
    }
    while(1)
    {
        int arg_max = arg_max = argmax_2d(covered_C, rows, cols);
        int row_coord = arg_max / cols;
        int col_coord = arg_max % cols;
        if(covered_C[row_coord][col_coord] == 0)
        {
            return (void*)step5;
        }
        else
        {
            state.marked[row_coord][col_coord] = 2;
            int star_col = 0;
            std::vector<int> temp_restore(cols, 0);
            for(int j = 0;j < cols; ++j)
                temp_restore[j] = (int)(state.marked[row_coord][j] == 1);
            star_col = argmax_1d(temp_restore, cols);
            if(state.marked[row_coord][star_col] != 1)
            {
                state.Z0_r = row_coord;
                state.Z0_c = col_coord;
                return (void*)step4;
            }
            else
            {
                col_coord = star_col;
                state.row_uncovered[row_coord] = false;
                state.col_uncovered[col_coord] = true;
                for (int i = 0; i < rows; i++) 
                {
                    if (!state.row_uncovered[i]) 
                        covered_C[i][col_coord] = 0;
                    else
                        covered_C[i][col_coord] = C[i][col_coord];
                }
                for (int j = 0; j < cols; j++)
                    covered_C[row_coord][j] = 0;
            }
        }
    }
}

void * step4(HungarianState & state)
{
    int row_length = state.marked.size();
    int col_length = state.marked[0].size();
    int count = 0;
    state.path[count][0] = state.Z0_r;
    state.path[count][1] = state.Z0_c;

    while(1)
    {
        int row = 0;
        std::vector<int> tmp_row_count(row_length, 1);
        for (int i = 0; i < row_length; i++) 
        {
            unsigned int colIndex = 0;
            (state.path[count][1] == -1) ? (colIndex = col_length - 1) : (colIndex = state.path[count][1]);
            tmp_row_count[i] = state.marked[i][colIndex];
            if (tmp_row_count[i] == 1)
                tmp_row_count[i] = 1;
            else
                tmp_row_count[i] = 0;
        }
        row = argmax_1d(tmp_row_count, row_length);
        unsigned int colIndex = 0;
        (state.path[count][1] == -1) ? (colIndex = col_length - 1) : (colIndex = state.path[count][1]);
        if (state.marked[row][colIndex] != 1) 
        {
            break;
        }
        else
        {
            count += 1;
            state.path[count][0] = row;
            state.path[count][1] = state.path[count - 1][1];
        }
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
        col = argmax_1d(tmp_col_count, col_length);
        if (state.marked[row][col] != 2) {
            col = -1;
        }
        count += 1;
        state.path[count][0] = state.path[count - 1][0];
        state.path[count][1] = col;
    }
    for (int i = 0; i < count + 1; i++) {
        unsigned int colIndex = 0;
        (state.path[i][1] == -1) ? (colIndex = col_length - 1) : (colIndex = state.path[i][1]);

        if (state.marked[state.path[i][0]][colIndex] == 1) {
            state.marked[state.path[i][0]][colIndex] = 0;
        } else {
            state.marked[state.path[i][0]][colIndex] = 1;
        }
    }
    state.clear_covers();
    for (int i = 0; i < row_length; i++) {
        for (int j = 0; j < col_length; j++) {
            if (state.marked[i][j] == 2) {
                state.marked[i][j] = 0;
            }
        }
    }
    return (void *)step2;
}

void * step5(HungarianState & state)
{
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
        unsigned int rows = state.row_uncovered.size();
        unsigned int cols = state.col_uncovered.size();
        std::vector<float> minval(cols, MAX_FLOAT_VALUE);
        float minvals = MAX_FLOAT_VALUE;

        for (int i = 0; i < rows; i++) {
            if (state.row_uncovered[i]) {
                for (int j = 0; j < cols ; j++) {
                    minval[j] = std::min(minval[j], state.C[i][j]);
                }
            }
        }
        for (int i = 0; i < cols; i++) {
            if (state.col_uncovered[i]) {
                minvals = std::min(minvals, minval[i]);
            }
        }
        for (int i = 0; i < rows; i++) {
            if (state.row_uncovered[i] != true) {
                for (int j = 0; j < cols; j++) {
                    state.C[i][j] += minvals;
                }
            }
        }
        for (int j = 0; j < cols; j++) {
            if (state.col_uncovered[j]) {
                for (int i = 0; i < rows; i++) {
                    state.C[i][j] -= minvals;
                }
            }
        }
    }
    return (void *)step3;
}

std::vector<std::vector<int>> linear_assignment(std::vector<std::vector<float>> & cost)
{
    HungarianState state = HungarianState(cost);

    const unsigned int rows = state.C.size();
    const unsigned int cols = state.C[0].size();

    HUNGARIAN_FUNC step = step1;
    if (cost.size() == 0 || cost[0].size() == 0) {
        step = nullptr;
    }
    while (step) {
        step = (HUNGARIAN_FUNC)step(state);
    }
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