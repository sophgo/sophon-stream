#pragma once

#ifndef LINEAR_ASSIGNMENT_HUNGARIANSTATE_H
#define LINEAR_ASSIGNMENT_HUNGARIANSTATE_H

#include <functional>
#include <vector>

class HungarianState
// https://brilliant.org/wiki/hungarian-matching/
{
 public:
  int Z0_c = 0;
  int Z0_r = 0;
  std::vector<std::vector<float>> C;
  bool transposed;
  std::vector<bool> row_uncovered;
  std::vector<bool> col_uncovered;
  std::vector<std::vector<int>> path;
  std::vector<std::vector<int>> marked;

  HungarianState(const std::vector<std::vector<float>>& cost_matrix);
  ~HungarianState();

  void _clear_covers();
  // int find_prime_in_row(int x);
};

// using HUNGARIAN_FUNC = std::function<void*(HungarianState &)>;

typedef void* (*STEP_FUNC)(HungarianState& state);

void* step1(HungarianState& s);
void* step6(HungarianState& s);
void* step3(HungarianState& s);
void* step4(HungarianState& s);
void* step5(HungarianState& s);

std::vector<std::vector<int>> linear_assignment(
    std::vector<std::vector<float>>& outputs);

#endif