/*
 * Copyright © 2022 Christos Pavlatos, George Rassias, Christos Andrikos,
 *                  Evangelos Makris, Aggelos Kolaitis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool gSymmetricBulges;
int gMinStemsAfterBulge;
int gMaxBulgeSize;
bool gCountStemsFromBulges;

#define MIN(a, b) ((a) < (b) ? (a) : (b))

// IS_PAIR checks for AU, GC, GU, UA, CG, UG pairs
#define IS_PAIR(a, b) (IS_PAIR_ONEWAY(a, b) || IS_PAIR_ONEWAY(b, a))

// IS_PAIR_ONEWAY checks for AU, GC, GU pairs
#define IS_PAIR_ONEWAY(a, b)                                                   \
  (((a) == 'a' && (b) == 'u') || ((a) == 'g' && ((b) == 'c' || (b) == 'u')))


void find_bulge(char *sequence, int I, int J, int i, int j, bool *o_has_bulge,
                int *o_bulge_size_left, int *o_bulge_size_right, int *o_stems) {

  *o_has_bulge = false;
  *o_stems = 0;

  for (int leftBulgeSize = 1; leftBulgeSize < MIN(J - I - 1, gMaxBulgeSize + 1);
       leftBulgeSize++) {
    for (int rightBulgeSize = 1;
         rightBulgeSize < MIN(j - i - 1, gMaxBulgeSize + 1); rightBulgeSize++) {

      if (gSymmetricBulges && leftBulgeSize != rightBulgeSize) {
        continue;
      }
      int stems = 0;

      // align stems after bulges
      for (int a = J - leftBulgeSize, b = i + rightBulgeSize;
           a >= I && b <= j && IS_PAIR(sequence[a], sequence[b]); a--, b++) {
        stems++;
      }

      // found better alignment. criteria is loop size
      if (stems >= gMinStemsAfterBulge && stems > *o_stems) {
        *o_has_bulge = true;
        *o_bulge_size_left = leftBulgeSize;
        *o_bulge_size_right = rightBulgeSize;
        *o_stems = stems;
      }
    }
  }
}

void pairalign(char *sequence, int i, int j, int left_left_loop_size, int left_mid_loop_size, int left_right_loop_size, int right_left_loop_size, int right_mid_loop_size, int dd_size,
               void (*cb)(char *, int, int, int, int)) {

  int L = i;
  int M1 = i + left_left_loop_size + 1;
  int M2 = M1 + left_mid_loop_size + 1;
  int l = M2 + left_right_loop_size + 1;
  int R = l + dd_size + 1;
  int m1 = R + right_left_loop_size + 1;
  int m2 = m1 + right_mid_loop_size + 1;
  int r = i + j - 1;
  

  char *dot_bracket = strdup(sequence);
  int left_loop_stems = 0, middle_1_loop_stems = 0, middle_2_loop_stems = 0, right_loop_stems = 0;

  // initialize dot bracket
  int len = strlen(sequence);
  memset(dot_bracket, '.', len);
  dot_bracket[L] = '(';
  dot_bracket[l] = ')';
  dot_bracket[M1] = '[';
  dot_bracket[m1] = ']';
  dot_bracket[M2] = '<';
  dot_bracket[m2] = '>';
  dot_bracket[R] = '{';
  dot_bracket[r] = '}';

  left_loop_stems = 0;
  for (int a = L - 1, b = l - 1; a >= 0 && b <= M2 - 1; a--, b--) {// alan changed m to m1
    if (!IS_PAIR(sequence[a], sequence[b]) || dot_bracket[b] != '.') {
      break;
    }
    dot_bracket[a] = '(';
    dot_bracket[b] = ')';
    left_loop_stems++;
  }

  middle_1_loop_stems = 0;
  for (int a = M1 - 1, b = m1 + 1; a >= L + 1 && b <= m2 - 1; a--, b++) {
    if (!IS_PAIR(sequence[a], sequence[b])) {
      break;
    }
    dot_bracket[a] = '[';
    dot_bracket[b] = ']';
    middle_1_loop_stems++;
  }
  
  middle_2_loop_stems = 0;
  for (int a = M2 - 1, b = m2 + 1; a >= M1 + 1 && b <= r - 1; a--, b++) {
    if (!IS_PAIR(sequence[a], sequence[b])) {
      break;
    }
    dot_bracket[a] = '<';
    dot_bracket[b] = '>';
    middle_2_loop_stems++;
  }

  right_loop_stems = 0;
  for (int a = R + 1, b = r + 1; a <= m1 + 1 && b <= len - 1; a++, b++) {
    if (!IS_PAIR(sequence[a], sequence[b]) || dot_bracket[a] != '.') {
      break;
    }
    dot_bracket[a] = '{';
    dot_bracket[b] = '}';
    right_loop_stems++;
  }

  cb(dot_bracket, left_loop_stems, middle_1_loop_stems, middle_2_loop_stems, right_loop_stems);

  bool rBulge, lBulge, m1Bulge, m2Bulge;
  int rBulgeSizeRight, rBulgeSizeLeft, rStems;
  int m1BulgeSizeRight, m1BulgeSizeLeft, m1Stems;
  int m2BulgeSizeRight, m2BulgeSizeLeft, m2Stems;
  int lBulgeSizeRight, lBulgeSizeLeft, lStems;

  find_bulge(sequence, 0, L - left_loop_stems - 1, l + left_loop_stems + 1,
             R - 1, &lBulge, &lBulgeSizeLeft, &lBulgeSizeRight, &lStems);

  find_bulge(sequence, L + 1, M1 - middle_1_loop_stems - 1,
             m1 + middle_1_loop_stems + 1, m2 - 1, &m1Bulge, &m1BulgeSizeLeft,
             &m1BulgeSizeRight, &m1Stems);

  find_bulge(sequence, M1 + 1, M2 - middle_2_loop_stems - 1,
             m2 + middle_2_loop_stems + 1, r - 1, &m2Bulge, &m2BulgeSizeLeft,
             &m2BulgeSizeRight, &m2Stems);

  find_bulge(sequence, l + 1 , R - right_loop_stems - 1,
             r + right_loop_stems + 1, len - 1, &rBulge, &rBulgeSizeLeft,
            &rBulgeSizeRight,&rStems);

  if (lBulge) 
  {
    memset(&dot_bracket[L - left_loop_stems - lBulgeSizeLeft - lStems], '(',
           lStems);
    memset(&dot_bracket[l + left_loop_stems + 1 + lBulgeSizeRight], ')',
           lStems);

    cb(dot_bracket, left_loop_stems + (gCountStemsFromBulges ? lStems : 0), middle_1_loop_stems, middle_2_loop_stems, right_loop_stems);

    memset(&dot_bracket[L - left_loop_stems - lBulgeSizeLeft - lStems], '.',
           lStems);
    memset(&dot_bracket[l + left_loop_stems + 1 + lBulgeSizeRight], '.',
           lStems);
  }

  if (m1Bulge) 
  {
    memset(&dot_bracket[M1 - middle_1_loop_stems - m1BulgeSizeLeft - m1Stems], '[',
           lStems);
    memset(&dot_bracket[m1 + middle_1_loop_stems + 1 + m1BulgeSizeRight], ']',
           lStems);

    cb(dot_bracket, left_loop_stems, middle_1_loop_stems + (gCountStemsFromBulges ? m1Stems : 0), middle_2_loop_stems, right_loop_stems);

    memset(&dot_bracket[M1 - middle_1_loop_stems - m1BulgeSizeLeft - m1Stems], '.',
           lStems);
    memset(&dot_bracket[m1 + middle_1_loop_stems + 1 +m1BulgeSizeRight], '.',
           lStems);     
  }
  
  if (m2Bulge) 
  {
    memset(&dot_bracket[M2 - middle_2_loop_stems - m2BulgeSizeLeft - m2Stems], '<',
           lStems);
    memset(&dot_bracket[m2 + middle_2_loop_stems + 1 + m2BulgeSizeRight], '>',
           lStems);

    cb(dot_bracket, left_loop_stems, middle_1_loop_stems, middle_2_loop_stems + (gCountStemsFromBulges ? m2Stems : 0), right_loop_stems);

    memset(&dot_bracket[M2 - middle_2_loop_stems - m2BulgeSizeLeft - m2Stems], '.',
           lStems);
    memset(&dot_bracket[m2 + middle_2_loop_stems + 1 +m2BulgeSizeRight], '.',
           lStems);     
  }

  if (rBulge) 
  {
    memset(&dot_bracket[R - right_loop_stems - rBulgeSizeLeft - rStems], '{',
           rStems);
    memset(&dot_bracket[r + right_loop_stems + 1 + rBulgeSizeRight], '}',
           rStems);

    cb(dot_bracket, left_loop_stems, middle_1_loop_stems, middle_2_loop_stems, right_loop_stems + (gCountStemsFromBulges ? rStems : 0)); //alan changed to middle_1_loop_stems? too few arguments, added both middles
  }

  if (lBulge && rBulge) 
  {
    memset(&dot_bracket[L - left_loop_stems - lBulgeSizeLeft - lStems], '(',
           lStems);
    memset(&dot_bracket[l + left_loop_stems + 1 + lBulgeSizeRight], ')',
           lStems);

    cb(dot_bracket, left_loop_stems + (gCountStemsFromBulges ? lStems : 0), middle_1_loop_stems, middle_2_loop_stems, right_loop_stems + (gCountStemsFromBulges ? rStems : 0));

    memset(&dot_bracket[R - right_loop_stems - rBulgeSizeLeft - rStems], '.',
           rStems);
    memset(&dot_bracket[r + right_loop_stems + 1 + rBulgeSizeRight], '.',
           rStems);

  }

  if (lBulge && m1Bulge)
  {
    memset(&dot_bracket[M1 - middle_1_loop_stems - m1BulgeSizeLeft - m1Stems], '[',
           lStems);
    memset(&dot_bracket[m1 + middle_1_loop_stems + 1 + m1BulgeSizeRight], ']',
           lStems);

    cb(dot_bracket, left_loop_stems + (gCountStemsFromBulges ? lStems : 0), middle_1_loop_stems + (gCountStemsFromBulges ? m1Stems : 0), middle_2_loop_stems, right_loop_stems);

    memset(&dot_bracket[L - left_loop_stems - lBulgeSizeLeft - lStems], '.',
           lStems);
    memset(&dot_bracket[l + left_loop_stems + 1 + lBulgeSizeRight], '.',
           lStems); 
  }

  if (rBulge && m1Bulge) 
  {
    memset(&dot_bracket[R - right_loop_stems - rBulgeSizeLeft - rStems], '{',
           rStems);
    memset(&dot_bracket[r + right_loop_stems + 1 + rBulgeSizeRight], '}',
           rStems);

    cb(dot_bracket, left_loop_stems, middle_1_loop_stems + (gCountStemsFromBulges ? m1Stems : 0), middle_2_loop_stems, right_loop_stems + (gCountStemsFromBulges ? rStems : 0));
  }

  if (lBulge && m2Bulge)
  {
    memset(&dot_bracket[M2 - middle_2_loop_stems - m2BulgeSizeLeft - m2Stems], '<',
           lStems);
    memset(&dot_bracket[m2 + middle_2_loop_stems + 1 + m2BulgeSizeRight], '>',
           lStems);

    cb(dot_bracket, left_loop_stems + (gCountStemsFromBulges ? lStems : 0), middle_1_loop_stems, middle_2_loop_stems + (gCountStemsFromBulges ? m2Stems : 0), right_loop_stems);

    memset(&dot_bracket[L - left_loop_stems - lBulgeSizeLeft - lStems], '.',
           lStems);
    memset(&dot_bracket[l + left_loop_stems + 1 + lBulgeSizeRight], '.',
           lStems); 
  }

  if (rBulge && m2Bulge) 
  {
    memset(&dot_bracket[R - right_loop_stems - rBulgeSizeLeft - rStems], '{',
           rStems);
    memset(&dot_bracket[r + right_loop_stems + 1 + rBulgeSizeRight], '}',
           rStems);

    cb(dot_bracket, left_loop_stems, middle_1_loop_stems, middle_2_loop_stems + (gCountStemsFromBulges ? m2Stems : 0), right_loop_stems + (gCountStemsFromBulges ? rStems : 0));
  }

  if (lBulge && m1Bulge && rBulge) 
  {
    memset(&dot_bracket[L - left_loop_stems - lBulgeSizeLeft - lStems], '(',
           lStems);
    memset(&dot_bracket[l + left_loop_stems + 1 + lBulgeSizeRight], ')',
           lStems); 
    
    cb(dot_bracket, left_loop_stems + (gCountStemsFromBulges ? lStems : 0), middle_1_loop_stems + (gCountStemsFromBulges ? m1Stems : 0), middle_2_loop_stems, right_loop_stems + (gCountStemsFromBulges ? rStems : 0));
  }
  
  if (lBulge && m2Bulge && rBulge) 
  {
    memset(&dot_bracket[L - left_loop_stems - lBulgeSizeLeft - lStems], '(',
           lStems);
    memset(&dot_bracket[l + left_loop_stems + 1 + lBulgeSizeRight], ')',
           lStems); 
    
    cb(dot_bracket, left_loop_stems + (gCountStemsFromBulges ? lStems : 0), middle_1_loop_stems, middle_2_loop_stems + (gCountStemsFromBulges ? m2Stems : 0), right_loop_stems + (gCountStemsFromBulges ? rStems : 0));
  }
  
  if (lBulge && m1Bulge && m2Bulge && rBulge)
  {
	//what should i add here?
	//as a placeholder I'm leaving the following code
	  
	memset(&dot_bracket[L - left_loop_stems - lBulgeSizeLeft - lStems], '(',
           lStems);
	memset(&dot_bracket[l + left_loop_stems + 1 + lBulgeSizeRight], ')',
           lStems); 
    
	cb(dot_bracket, left_loop_stems + (gCountStemsFromBulges ? lStems : 0), middle_1_loop_stems + (gCountStemsFromBulges ? m1Stems : 0), middle_2_loop_stems + (gCountStemsFromBulges ? m2Stems : 0),
	   right_loop_stems + (gCountStemsFromBulges ? rStems : 0));
  }
  
  free(dot_bracket);
}

void initialize(int max_bulge_size, int min_stems_after_bulge,
                bool symmetric_bulges,bool count_stems_from_bulges) {
  gSymmetricBulges = symmetric_bulges;
  gMaxBulgeSize = max_bulge_size;
  gMinStemsAfterBulge = min_stems_after_bulge;
  gCountStemsFromBulges = count_stems_from_bulges;
}