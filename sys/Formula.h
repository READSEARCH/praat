#ifndef _Formula_h_
#define _Formula_h_
/* Formula.h
 *
 * Copyright (C) 1990-2011,2013,2014,2015 Paul Boersma
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this work. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Data.h"

#define kFormula_EXPRESSION_TYPE_NUMERIC  0
#define kFormula_EXPRESSION_TYPE_STRING  1
#define kFormula_EXPRESSION_TYPE_NUMERIC_ARRAY  2
#define kFormula_EXPRESSION_TYPE_STRING_ARRAY  3
#define kFormula_EXPRESSION_TYPE_UNKNOWN  4

struct Formula_NumericArray {
	long numberOfRows, numberOfColumns;
	double **data;
};

Thing_declare (InterpreterVariable);

typedef struct structStackel {
	#define Stackel_NUMBER  0
	#define Stackel_STRING  1
	#define Stackel_NUMERIC_ARRAY  2
	#define Stackel_STRING_ARRAY  3
	#define Stackel_VARIABLE  -1
	int which;   /* 0 or negative = no clean-up required, positive = requires clean-up */
	union {
		double number;
		char32 *string;
		struct Formula_NumericArray numericArray;
		InterpreterVariable variable;
	};
} *Stackel;
const char32 *Stackel_whichText (Stackel me);

struct Formula_Result {
	int expressionType;
	union {
		double numericResult;
		char32 *stringResult;
		struct Formula_NumericArray numericArrayResult;
	} result;
};

Thing_declare (Interpreter);

void Formula_compile (Interpreter interpreter, Daata data, const char32 *expression, int expressionType, bool optimize);

void Formula_run (long row, long col, struct Formula_Result *result);

/* End of file Formula.h */
#endif
